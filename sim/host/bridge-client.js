/**
 * WebSocket client for local hardware bridge server.
 * Phase 4: implements shared hardware transport interface with session states.
 */
import {
  TRANSPORT_BRIDGE,
  SESSION_DISCONNECTED,
  SESSION_CONNECTING,
  SESSION_ACTIVE,
  SESSION_DEGRADED,
  SESSION_RECONNECTING,
  claimPortExclusive,
  releasePortExclusive,
  currentPortHolder,
  portBusyMessage,
  isWebSerialAvailable,
} from './hardware-transport.js';

export class BridgeClient {
  /**
   * @param {string} url
   */
  constructor(url) {
    this.transportKind = TRANSPORT_BRIDGE;
    this.url = url;
    /** @type {WebSocket | null} */
    this.ws = null;
    this.bridgeReachable = false;
    this.deviceAttached = false;
    /** @type {import('./hardware-transport.js').HardwareSessionState} */
    this.sessionState = SESSION_DISCONNECTED;
    this.firmwareModuleKind = null;
    this.devicePath = null;
    this.lastError = null;
    this.reconnectAttempt = 0;
    this.fatalError = false;
    this.onStatus = () => {};
    this.onError = (message) => {
      this.lastError = message;
    };
    this.onLinkLoss = () => {};
    /** @type {Map<number, { resolve: (buf: ArrayBuffer) => void, reject: (err: Error) => void, timer: ReturnType<typeof setTimeout> }>} */
    this._pendingBySequence = new Map();
    /** @type {((status: object) => void) | null} */
    this._sessionWaiter = null;
    this.maxInFlight = 2;
    this._hadActiveSession = false;
  }

  get sessionActive() {
    return this.sessionState === SESSION_ACTIVE;
  }

  _setState(state) {
    this.sessionState = state;
    this.onStatus(this.statusSnapshot());
  }

  statusSnapshot() {
    return {
      transportKind: TRANSPORT_BRIDGE,
      transportAvailable: true,
      bridgeReachable: this.bridgeReachable,
      deviceAttached: this.deviceAttached,
      sessionActive: this.sessionActive,
      sessionState: this.sessionState,
      firmwareModuleKind: this.firmwareModuleKind,
      devicePath: this.devicePath,
      lastError: this.lastError,
      reconnectAttempt: this.reconnectAttempt,
      webSerialAvailable: isWebSerialAvailable(),
    };
  }

  connect() {
    if (currentPortHolder() === 'webserial') {
      return Promise.reject(new Error(portBusyMessage(TRANSPORT_BRIDGE)));
    }
    return new Promise((resolve, reject) => {
      if (this.ws && this.ws.readyState === WebSocket.OPEN) {
        resolve();
        return;
      }
      const ws = new WebSocket(this.url);
      ws.binaryType = 'arraybuffer';
      this.ws = ws;

      ws.onopen = () => {
        this.bridgeReachable = true;
        ws.send(JSON.stringify({ type: 'ping' }));
        this.onStatus(this.statusSnapshot());
        resolve();
      };

      ws.onerror = () => {
        this.bridgeReachable = false;
        reject(new Error('Bridge connection failed'));
      };

      ws.onclose = () => {
        const wasActive = this._hadActiveSession;
        this.bridgeReachable = false;
        this.deviceAttached = false;
        this._hadActiveSession = false;
        this._rejectAllPending(new Error('Bridge connection closed'));
        if (wasActive && !this.fatalError) {
          this.sessionState = SESSION_DEGRADED;
          this.onStatus(this.statusSnapshot());
          this.onLinkLoss();
        } else if (!this.fatalError) {
          this._setState(SESSION_DISCONNECTED);
        }
      };

      ws.onmessage = (event) => {
        if (event.data instanceof ArrayBuffer) {
          this._resolveBinaryResponse(event.data);
          return;
        }
        const text = typeof event.data === 'string' ? event.data : String(event.data);
        try {
          const msg = JSON.parse(text);
          this._handleControl(msg);
        } catch (err) {
          this.onError(err.message);
        }
      };
    });
  }

  _rejectAllPending(err) {
    for (const [, pending] of this._pendingBySequence) {
      clearTimeout(pending.timer);
      pending.reject(err);
    }
    this._pendingBySequence.clear();
  }

  _resolveBinaryResponse(data) {
    const buf = new Uint8Array(data);
    if (buf.length < 8) {
      return;
    }
    const inner = buf.subarray(4);
    const view = new DataView(inner.buffer, inner.byteOffset, inner.byteLength);
    if (inner.byteLength < 10) {
      return;
    }
    const sequence = view.getUint32(6, true);
    const pending = this._pendingBySequence.get(sequence);
    if (!pending) {
      const fifoKey = this._pendingBySequence.keys().next().value;
      if (fifoKey !== undefined) {
        const fifoPending = this._pendingBySequence.get(fifoKey);
        if (fifoPending) {
          clearTimeout(fifoPending.timer);
          this._pendingBySequence.delete(fifoKey);
          fifoPending.resolve(data);
        }
      }
      return;
    }
    clearTimeout(pending.timer);
    this._pendingBySequence.delete(sequence);
    pending.resolve(data);
  }

  _handleControl(msg) {
    if (msg.type === 'status' || msg.type === 'session') {
      if (msg.bridgeReachable !== undefined) {
        this.bridgeReachable = msg.bridgeReachable;
      }
      if (msg.deviceAttached !== undefined) {
        this.deviceAttached = msg.deviceAttached;
      }
      if (msg.devicePath !== undefined) {
        this.devicePath = msg.devicePath;
      }
      if (msg.sessionActive !== undefined && msg.type === 'status') {
        if (!msg.sessionActive && this.sessionState === SESSION_ACTIVE) {
          this._hadActiveSession = false;
          this.sessionState = SESSION_DEGRADED;
          this.onLinkLoss();
        }
      }
      if (msg.firmwareModuleKind !== undefined) {
        this.firmwareModuleKind = msg.firmwareModuleKind;
      }
      if (msg.action === 'started' && msg.firmwareModuleKind) {
        this.firmwareModuleKind = msg.firmwareModuleKind;
        this._setState(SESSION_ACTIVE);
        this._hadActiveSession = true;
        claimPortExclusive(TRANSPORT_BRIDGE);
      }
      if (msg.action === 'stopped') {
        this.firmwareModuleKind = null;
        this._hadActiveSession = false;
        if (!this.fatalError && this.sessionState !== SESSION_DEGRADED) {
          this._setState(SESSION_DISCONNECTED);
        }
        releasePortExclusive(TRANSPORT_BRIDGE);
      }
      const snapshot = this.statusSnapshot();
      this.onStatus(snapshot);
      if (this._sessionWaiter && snapshot.sessionActive) {
        const waiter = this._sessionWaiter;
        this._sessionWaiter = null;
        waiter(snapshot);
      }
      return;
    }
    if (msg.type === 'error') {
      this.onError(msg.message);
      if (msg.message?.includes('port') || msg.message?.includes('busy')) {
        this.lastError = msg.message;
      }
    }
  }

  async startSession(mode = 'pwa') {
    if (currentPortHolder() === 'webserial') {
      throw new Error(portBusyMessage(TRANSPORT_BRIDGE));
    }
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      await this.connect();
    }
    this._setState(SESSION_CONNECTING);
    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {
        this._sessionWaiter = null;
        this._setState(SESSION_DISCONNECTED);
        reject(new Error('Session start timeout'));
      }, 10000);
      this._sessionWaiter = (status) => {
        clearTimeout(timeout);
        this.fatalError = false;
        this.reconnectAttempt = 0;
        resolve(status);
      };
      this.ws.send(JSON.stringify({ type: 'session', action: 'start', mode }));
    });
  }

  async stopSession() {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      return;
    }
    this.ws.send(JSON.stringify({ type: 'session', action: 'stop' }));
    this._hadActiveSession = false;
    this.firmwareModuleKind = null;
    releasePortExclusive(TRANSPORT_BRIDGE);
    if (this.sessionState !== SESSION_DEGRADED && this.sessionState !== SESSION_RECONNECTING) {
      this._setState(SESSION_DISCONNECTED);
    }
    this._rejectAllPending(new Error('Session stopped'));
  }

  async reconnect() {
    if (this.fatalError) {
      throw new Error(this.lastError ?? 'Fatal error — disconnect and reconnect manually');
    }
    this._setState(SESSION_RECONNECTING);
    this.reconnectAttempt += 1;
    this.onStatus(this.statusSnapshot());
    try {
      if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
        await this.connect();
      }
      return await this.startSession('pwa');
    } catch (err) {
      this._setState(SESSION_DEGRADED);
      this.lastError = err.message;
      this.onError(err.message);
      throw err;
    }
  }

  markFatal(message) {
    this.fatalError = true;
    this.lastError = message;
    this._hadActiveSession = false;
    this._setState(SESSION_DISCONNECTED);
    releasePortExclusive(TRANSPORT_BRIDGE);
    this._rejectAllPending(new Error(message));
  }

  disconnect() {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
    this.bridgeReachable = false;
    this.deviceAttached = false;
    this._hadActiveSession = false;
    releasePortExclusive(TRANSPORT_BRIDGE);
    this._setState(SESSION_DISCONNECTED);
    this.fatalError = false;
    this.reconnectAttempt = 0;
    this.lastError = null;
    this._rejectAllPending(new Error('Disconnected'));
  }

  /**
   * @param {Uint8Array} requestBytes length-prefixed wire frame
   * @param {number} sequence exchange sequence for response matching
   * @returns {Promise<ArrayBuffer>}
   */
  relayExchange(requestBytes, sequence) {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN || !this.sessionActive) {
      return Promise.reject(new Error('No active bridge session'));
    }
    if (this._pendingBySequence.size >= this.maxInFlight) {
      return Promise.reject(new Error('Exchange pipeline full'));
    }
    return new Promise((resolve, reject) => {
      const timer = setTimeout(() => {
        this._pendingBySequence.delete(sequence);
        reject(new Error('Exchange relay timeout'));
      }, 15000);
      this._pendingBySequence.set(sequence, { resolve, reject, timer });
      this.ws.send(requestBytes);
    });
  }
}
