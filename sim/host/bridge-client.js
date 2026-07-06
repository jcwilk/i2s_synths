/**
 * WebSocket client for local hardware bridge server.
 * Phase 3: depth-2 pipelined binary exchanges matched by sequence.
 */
export class BridgeClient {
  /**
   * @param {string} url
   */
  constructor(url) {
    this.url = url;
    /** @type {WebSocket | null} */
    this.ws = null;
    this.bridgeReachable = false;
    this.deviceAttached = false;
    this.sessionActive = false;
    this.firmwareModuleKind = null;
    this.devicePath = null;
    this.onStatus = () => {};
    this.onError = () => {};
    /** @type {Map<number, { resolve: (buf: ArrayBuffer) => void, reject: (err: Error) => void, timer: ReturnType<typeof setTimeout> }>} */
    this._pendingBySequence = new Map();
    /** @type {((status: object) => void) | null} */
    this._sessionWaiter = null;
    this.maxInFlight = 2;
  }

  connect() {
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
        resolve();
      };

      ws.onerror = () => {
        this.bridgeReachable = false;
        reject(new Error('Bridge connection failed'));
      };

      ws.onclose = () => {
        this.bridgeReachable = false;
        this.deviceAttached = false;
        this.sessionActive = false;
        this._rejectAllPending(new Error('Bridge connection closed'));
        this.onStatus(this.statusSnapshot());
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
      if (msg.sessionActive !== undefined) {
        this.sessionActive = msg.sessionActive;
      }
      if (msg.firmwareModuleKind !== undefined) {
        this.firmwareModuleKind = msg.firmwareModuleKind;
      }
      if (msg.action === 'started' && msg.firmwareModuleKind) {
        this.firmwareModuleKind = msg.firmwareModuleKind;
        this.sessionActive = true;
      }
      if (msg.action === 'stopped') {
        this.sessionActive = false;
        this.firmwareModuleKind = null;
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
    }
  }

  statusSnapshot() {
    return {
      bridgeReachable: this.bridgeReachable,
      deviceAttached: this.deviceAttached,
      sessionActive: this.sessionActive,
      firmwareModuleKind: this.firmwareModuleKind,
      devicePath: this.devicePath,
    };
  }

  async startSession(mode = 'pwa') {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      await this.connect();
    }
    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {
        this._sessionWaiter = null;
        reject(new Error('Session start timeout'));
      }, 10000);
      this._sessionWaiter = (status) => {
        clearTimeout(timeout);
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
    this.sessionActive = false;
    this._rejectAllPending(new Error('Session stopped'));
  }

  disconnect() {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
    this.bridgeReachable = false;
    this.deviceAttached = false;
    this.sessionActive = false;
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
