/**
 * Web Serial direct hardware transport (Phase 4).
 */

import {
  BRIDGE_STATUS_OK,
  BRIDGE_ENTER_MODE_PWA_ADC,
  buildEnterUsbFrame,
  buildExitUsbFrame,
  buildQueryModuleFrame,
  parseAck,
  parseExchangeResponse,
  firmwareKindToName,
  statusIsOk,
} from './frame-protocol.js';
import { createWebSerialFrameReader } from './web-serial-frame-reader.js';
import {
  TRANSPORT_WEB_SERIAL,
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

export class WebSerialTransport {
  constructor() {
    this.transportKind = TRANSPORT_WEB_SERIAL;
    /** @type {import('./hardware-transport.js').HardwareSessionState} */
    this.sessionState = SESSION_DISCONNECTED;
    /** @type {SerialPort | null} */
    this.port = null;
    /** @type {WritableStreamDefaultWriter<Uint8Array> | null} */
    this.writer = null;
    /** @type {ReturnType<typeof createWebSerialFrameReader> | null} */
    this.frameReader = null;
    this.firmwareModuleKind = null;
    this.devicePath = null;
    this.lastError = null;
    this.reconnectAttempt = 0;
    this.fatalError = false;
    this.onLinkLoss = () => {};
    this.onStatus = () => {};
    this.onError = (message) => {
      this.lastError = message;
    };
    /** @type {Map<number, { resolve: (buf: ArrayBuffer) => void, reject: (err: Error) => void, timer: ReturnType<typeof setTimeout> }>} */
    this._pendingBySequence = new Map();
    this.maxInFlight = 2;
  }

  get sessionActive() {
    return this.sessionState === SESSION_ACTIVE;
  }

  statusSnapshot() {
    return {
      transportKind: TRANSPORT_WEB_SERIAL,
      transportAvailable: isWebSerialAvailable(),
      bridgeReachable: false,
      deviceAttached: this.port !== null,
      sessionActive: this.sessionActive,
      sessionState: this.sessionState,
      firmwareModuleKind: this.firmwareModuleKind,
      devicePath: this.devicePath,
      lastError: this.lastError,
      reconnectAttempt: this.reconnectAttempt,
    };
  }

  _emitStatus() {
    this.onStatus(this.statusSnapshot());
  }

  _setState(state) {
    this.sessionState = state;
    this._emitStatus();
  }

  async connect() {
    if (!isWebSerialAvailable()) {
      throw new Error('Web Serial is not available in this browser');
    }
    if (currentPortHolder() === 'bridge') {
      throw new Error(portBusyMessage(TRANSPORT_WEB_SERIAL));
    }
    if (!claimPortExclusive(TRANSPORT_WEB_SERIAL)) {
      throw new Error(portBusyMessage(TRANSPORT_WEB_SERIAL));
    }
    this._setState(SESSION_CONNECTING);
    try {
      this.port = await navigator.serial.requestPort();
      await this.port.open({ baudRate: 115200 });
      this.devicePath = 'web-serial';
      this.writer = this.port.writable.getWriter();
      this.frameReader = createWebSerialFrameReader();
      await this.frameReader.attach(this.port.readable);
      await this._drainBoot(2500);
      this._emitStatus();
    } catch (err) {
      releasePortExclusive(TRANSPORT_WEB_SERIAL);
      await this._teardownPort();
      this._setState(SESSION_DISCONNECTED);
      if (err instanceof DOMException && err.name === 'NotFoundError') {
        throw new Error('Web Serial port selection cancelled');
      }
      if (err instanceof DOMException && err.name === 'SecurityError') {
        throw new Error('Web Serial permission denied — allow serial access in browser settings');
      }
      if (err instanceof DOMException && err.name === 'InvalidStateError') {
        throw new Error('Port busy: another application or bridge session holds the device');
      }
      throw err;
    }
  }

  async _drainBoot(ms) {
    await new Promise((r) => setTimeout(r, ms));
    this.frameReader?.reset();
  }

  async _writeFrame(frame) {
    if (!this.writer) {
      throw new Error('Web Serial port not open');
    }
    await this.writer.write(frame);
  }

  async _sendAckCommand(frame) {
    await this._writeFrame(frame);
    const inner = await this.frameReader.readFrame();
    const parsed = parseAck(inner);
    if (parsed.status !== BRIDGE_STATUS_OK) {
      throw new Error(`device status 0x${parsed.status.toString(16)}`);
    }
    return parsed;
  }

  async startSession(mode = 'pwa') {
    if (!this.port || !this.frameReader) {
      throw new Error('Web Serial not connected');
    }
    this._setState(SESSION_CONNECTING);
    const enterMode = mode === 'pwa' ? BRIDGE_ENTER_MODE_PWA_ADC : BRIDGE_ENTER_MODE_PWA_ADC;
    this.frameReader.reset();
    await this._sendAckCommand(buildEnterUsbFrame(enterMode));
    const ack = await this._sendAckCommand(buildQueryModuleFrame());
    this.firmwareModuleKind = firmwareKindToName(ack.sequence);
    this._setState(SESSION_ACTIVE);
    this.fatalError = false;
    this.reconnectAttempt = 0;
    this._watchDisconnect();
    return this.statusSnapshot();
  }

  _watchDisconnect() {
    if (!this.port) {
      return;
    }
    const port = this.port;
    if (port.ondisconnect !== undefined) {
      port.addEventListener('disconnect', () => {
        if (this.port === port && this.sessionState === SESSION_ACTIVE) {
          this._handleUnexpectedLinkLoss();
        }
      });
    }
  }

  _handleUnexpectedLinkLoss() {
    if (this.fatalError) {
      return;
    }
    this._rejectAllPending(new Error('Web Serial link lost'));
    this._setState(SESSION_DEGRADED);
    this.onLinkLoss();
  }

  async stopSession() {
    if (this.port && this.frameReader) {
      try {
        await this._sendAckCommand(buildExitUsbFrame());
      } catch (_) {
        // device may already be gone
      }
    }
    this.firmwareModuleKind = null;
    if (this.sessionState !== SESSION_DEGRADED && this.sessionState !== SESSION_RECONNECTING) {
      this._setState(SESSION_DISCONNECTED);
    }
    this._rejectAllPending(new Error('Session stopped'));
  }

  async disconnect() {
    await this.stopSession();
    await this._teardownPort();
    releasePortExclusive(TRANSPORT_WEB_SERIAL);
    this._setState(SESSION_DISCONNECTED);
    this.lastError = null;
    this.reconnectAttempt = 0;
    this.fatalError = false;
  }

  async _teardownPort() {
    this._rejectAllPending(new Error('Disconnected'));
    if (this.writer) {
      try {
        await this.writer.close();
      } catch (_) {}
      this.writer = null;
    }
    if (this.port) {
      try {
        await this.port.close();
      } catch (_) {}
      this.port = null;
    }
    this.frameReader = null;
    this.devicePath = null;
  }

  async reconnect() {
    if (this.fatalError) {
      throw new Error(this.lastError ?? 'Fatal error — disconnect and reconnect manually');
    }
    this._setState(SESSION_RECONNECTING);
    this.reconnectAttempt += 1;
    this._emitStatus();
    try {
      if (!this.port || !this.port.readable) {
        await this._teardownPort();
        releasePortExclusive(TRANSPORT_WEB_SERIAL);
        if (!claimPortExclusive(TRANSPORT_WEB_SERIAL)) {
          throw new Error(portBusyMessage(TRANSPORT_WEB_SERIAL));
        }
        this.port = await navigator.serial.requestPort();
        await this.port.open({ baudRate: 115200 });
        this.writer = this.port.writable.getWriter();
        this.frameReader = createWebSerialFrameReader();
        await this.frameReader.attach(this.port.readable);
        await this._drainBoot(1500);
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
    this._setState(SESSION_DISCONNECTED);
    this._rejectAllPending(new Error(message));
  }

  _rejectAllPending(err) {
    for (const [, pending] of this._pendingBySequence) {
      clearTimeout(pending.timer);
      pending.reject(err);
    }
    this._pendingBySequence.clear();
  }

  relayExchange(requestBytes, sequence) {
    if (!this.sessionActive || !this.frameReader) {
      return Promise.reject(new Error('No active Web Serial session'));
    }
    if (this._pendingBySequence.size >= this.maxInFlight) {
      return Promise.reject(new Error('Exchange pipeline full'));
    }
    return new Promise((resolve, reject) => {
      const timer = setTimeout(() => {
        this._pendingBySequence.delete(sequence);
        reject(new Error('Exchange relay timeout'));
      }, 15000);
      this._pendingBySequence.set(sequence, {
        resolve: async (data) => {
          try {
            resolve(data);
          } catch (e) {
            reject(e);
          }
        },
        reject,
        timer,
      });
      this._writeFrame(requestBytes).catch((err) => {
        clearTimeout(timer);
        this._pendingBySequence.delete(sequence);
        if (this.sessionState === SESSION_ACTIVE) {
          this._handleUnexpectedLinkLoss();
        }
        reject(err);
      });
      this._readExchangeResponse(sequence).catch((err) => {
        const pending = this._pendingBySequence.get(sequence);
        if (pending) {
          clearTimeout(pending.timer);
          this._pendingBySequence.delete(sequence);
          pending.reject(err);
        }
        if (this.sessionState === SESSION_ACTIVE) {
          this._handleUnexpectedLinkLoss();
        }
      });
    });
  }

  async _readExchangeResponse(sequence) {
    const inner = await this.frameReader.readFrame();
    const response = parseExchangeResponse(inner);
    if (!statusIsOk(response.status)) {
      throw new Error(`exchange status 0x${response.status.toString(16)}`);
    }
    const wire = new Uint8Array(4 + inner.length);
    const view = new DataView(wire.buffer);
    view.setUint32(0, inner.length, true);
    wire.set(inner, 4);
    const pending = this._pendingBySequence.get(sequence);
    if (pending) {
      clearTimeout(pending.timer);
      this._pendingBySequence.delete(sequence);
      pending.resolve(wire.buffer);
      return;
    }
    const fifoKey = this._pendingBySequence.keys().next().value;
    if (fifoKey !== undefined) {
      const fifoPending = this._pendingBySequence.get(fifoKey);
      if (fifoPending) {
        clearTimeout(fifoPending.timer);
        this._pendingBySequence.delete(fifoKey);
        fifoPending.resolve(wire.buffer);
      }
    }
  }
}
