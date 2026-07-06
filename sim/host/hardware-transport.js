/**
 * Shared hardware transport contract and session state constants (Phase 4).
 */

export const TRANSPORT_BRIDGE = 'bridge';
export const TRANSPORT_WEB_SERIAL = 'webserial';

/** @typedef {'disconnected'|'connecting'|'active'|'degraded'|'reconnecting'} HardwareSessionState */

export const SESSION_DISCONNECTED = 'disconnected';
export const SESSION_CONNECTING = 'connecting';
export const SESSION_ACTIVE = 'active';
export const SESSION_DEGRADED = 'degraded';
export const SESSION_RECONNECTING = 'reconnecting';

export const AUTO_RETRY_MAX = 3;
export const AUTO_RETRY_BACKOFF_MS = 2000;

/** @type {'bridge'|'webserial'|null} */
let portHolder = null;

/**
 * @returns {boolean}
 */
export function isWebSerialAvailable() {
  return typeof navigator !== 'undefined' && 'serial' in navigator;
}

/**
 * @param {'bridge'|'webserial'} holder
 * @returns {boolean}
 */
export function claimPortExclusive(holder) {
  if (portHolder !== null && portHolder !== holder) {
    return false;
  }
  portHolder = holder;
  return true;
}

/**
 * @param {'bridge'|'webserial'} holder
 */
export function releasePortExclusive(holder) {
  if (portHolder === holder) {
    portHolder = null;
  }
}

/**
 * @returns {'bridge'|'webserial'|null}
 */
export function currentPortHolder() {
  return portHolder;
}

/**
 * @param {'bridge'|'webserial'} attempted
 * @returns {string}
 */
export function portBusyMessage(attempted) {
  const other = portHolder === TRANSPORT_BRIDGE ? 'bridge server' : 'Web Serial';
  if (attempted === TRANSPORT_WEB_SERIAL) {
    return `Port busy: ${other} session holds the device. Disconnect the other session before connecting via Web Serial.`;
  }
  return `Port busy: ${other} session holds the device. Disconnect Web Serial in the browser before using the bridge.`;
}

/**
 * @typedef {object} TransportStatusSnapshot
 * @property {'bridge'|'webserial'} transportKind
 * @property {boolean} transportAvailable
 * @property {boolean} bridgeReachable
 * @property {boolean} deviceAttached
 * @property {boolean} sessionActive
 * @property {HardwareSessionState} sessionState
 * @property {string|null} firmwareModuleKind
 * @property {string|null} devicePath
 * @property {string|null} lastError
 * @property {number} reconnectAttempt
 */

/**
 * @typedef {object} HardwareTransport
 * @property {'bridge'|'webserial'} transportKind
 * @property {HardwareSessionState} sessionState
 * @property {boolean} sessionActive
 * @property {() => TransportStatusSnapshot} statusSnapshot
 * @property {() => Promise<void>} connect
 * @property {() => Promise<void>} disconnect
 * @property {(mode?: string) => Promise<TransportStatusSnapshot>} startSession
 * @property {() => Promise<void>} stopSession
 * @property {() => Promise<TransportStatusSnapshot>} reconnect
 * @property {(requestBytes: Uint8Array, sequence: number) => Promise<ArrayBuffer>} relayExchange
 * @property {() => void} onLinkLoss
 * @property {() => void} onStatus
 * @property {(message: string) => void} onError
 */
