/**
 * WebSocket client for local hardware bridge server.
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
    /** @type {Promise<ArrayBuffer> | null} */
    this._pendingBinary = null;
    /** @type {((buf: ArrayBuffer) => void) | null} */
    this._binaryResolver = null;
    /** @type {((status: object) => void) | null} */
    this._sessionWaiter = null;
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
        this.onStatus(this.statusSnapshot());
      };

      ws.onmessage = (event) => {
        if (event.data instanceof ArrayBuffer) {
          if (this._binaryResolver) {
            const resolve = this._binaryResolver;
            this._binaryResolver = null;
            this._pendingBinary = null;
            resolve(event.data);
          }
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
  }

  disconnect() {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
    this.bridgeReachable = false;
    this.deviceAttached = false;
    this.sessionActive = false;
  }

  /**
   * @param {Uint8Array} requestBytes length-prefixed wire frame
   * @returns {Promise<ArrayBuffer>}
   */
  relayExchange(requestBytes) {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN || !this.sessionActive) {
      return Promise.reject(new Error('No active bridge session'));
    }
    if (this._pendingBinary) {
      return Promise.reject(new Error('Exchange already in flight'));
    }
    this._pendingBinary = new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {
        this._binaryResolver = null;
        this._pendingBinary = null;
        reject(new Error('Exchange relay timeout'));
      }, 15000);
      this._binaryResolver = (buf) => {
        clearTimeout(timeout);
        resolve(buf);
      };
    });
    this.ws.send(requestBytes);
    return this._pendingBinary;
  }
}
