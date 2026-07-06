/**
 * Hardware session coordinator: reconnect state machine and bounded auto-retry (Phase 4).
 */

import {
  SESSION_ACTIVE,
  SESSION_DEGRADED,
  SESSION_RECONNECTING,
  SESSION_DISCONNECTED,
  AUTO_RETRY_MAX,
  AUTO_RETRY_BACKOFF_MS,
} from './hardware-transport.js';

export class HardwareSessionManager {
  /**
   * @param {import('./hardware-transport.js').HardwareTransport} transport
   */
  constructor(transport) {
    this.transport = transport;
    this.autoRetryEnabled = true;
    this.autoRetryCount = 0;
    /** @type {ReturnType<typeof setTimeout> | null} */
    this.autoRetryTimer = null;
    this.onStateChange = () => {};
    this.onFatal = () => {};
    transport.onLinkLoss = () => this._handleLinkLoss();
  }

  get sessionState() {
    return this.transport.sessionState;
  }

  get sessionActive() {
    return this.transport.sessionActive;
  }

  cancelAutoRetry() {
    if (this.autoRetryTimer !== null) {
      clearTimeout(this.autoRetryTimer);
      this.autoRetryTimer = null;
    }
    this.autoRetryCount = 0;
  }

  _handleLinkLoss() {
    if (this.transport.sessionState === SESSION_ACTIVE) {
      this.transport.sessionState = SESSION_DEGRADED;
      this.transport.onStatus(this.transport.statusSnapshot());
      this.onStateChange(SESSION_DEGRADED);
      if (this.autoRetryEnabled && !this.transport.fatalError) {
        this._scheduleAutoRetry();
      }
    }
  }

  _scheduleAutoRetry() {
    if (this.autoRetryCount >= AUTO_RETRY_MAX) {
      return;
    }
    this.cancelAutoRetry();
    this.autoRetryTimer = setTimeout(() => {
      this.autoRetryTimer = null;
      if (this.transport.sessionState !== SESSION_DEGRADED) {
        return;
      }
      this.operatorReconnect().catch(() => {});
    }, AUTO_RETRY_BACKOFF_MS);
  }

  /**
   * @param {string} expectedModuleKind
   * @returns {Promise<import('./hardware-transport.js').TransportStatusSnapshot>}
   */
  async startSession(expectedModuleKind) {
    this.cancelAutoRetry();
    const status = await this.transport.startSession('pwa');
    if (status.firmwareModuleKind && status.firmwareModuleKind !== expectedModuleKind) {
      await this.transport.stopSession();
      const msg = `Module kind mismatch: slot is ${expectedModuleKind}, device is ${status.firmwareModuleKind}`;
      this.transport.markFatal(msg);
      this.onFatal(msg);
      throw new Error(msg);
    }
    this.onStateChange(SESSION_ACTIVE);
    return status;
  }

  /**
   * @param {string} expectedModuleKind
   */
  async operatorReconnect(expectedModuleKind) {
    if (this.transport.fatalError) {
      throw new Error(this.transport.lastError ?? 'Fatal error');
    }
    if (
      this.transport.sessionState !== SESSION_DEGRADED
      && this.transport.sessionState !== SESSION_RECONNECTING
    ) {
      return this.transport.statusSnapshot();
    }
    this.autoRetryCount += 1;
    try {
      const status = await this.transport.reconnect();
      if (status.firmwareModuleKind && status.firmwareModuleKind !== expectedModuleKind) {
        const msg = `Module kind mismatch: slot is ${expectedModuleKind}, device is ${status.firmwareModuleKind}`;
        this.transport.markFatal(msg);
        this.onFatal(msg);
        throw new Error(msg);
      }
      this.autoRetryCount = 0;
      this.onStateChange(SESSION_ACTIVE);
      return status;
    } catch (err) {
      if (this.autoRetryCount < AUTO_RETRY_MAX && this.autoRetryEnabled) {
        this._scheduleAutoRetry();
      }
      throw err;
    }
  }

  async stopSession() {
    this.cancelAutoRetry();
    await this.transport.stopSession();
    this.onStateChange(this.transport.sessionState);
  }

  async disconnect() {
    this.cancelAutoRetry();
    await this.transport.disconnect();
    this.onStateChange(SESSION_DISCONNECTED);
  }
}
