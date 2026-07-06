/** Browser-side frame protocol (mirrors sim/hardware-bridge/frame-protocol.js). */

export {
  SAMPLE_RATE,
  BUFFER_LEN,
  PERIOD_MS,
} from '../shared/audio-constants.js';

export const BRIDGE_MAGIC = 0x31424d48;
export const BRIDGE_HEADER_SIZE = 6;
export const BRIDGE_WIRE_PREFIX_SIZE = 4;

export const BRIDGE_CMD_ENTER_USB = 0x11;
export const BRIDGE_CMD_EXIT_USB = 0x12;
export const BRIDGE_CMD_EXCHANGE_USB = 0x13;
export const BRIDGE_CMD_LOOPBACK_USB = 0x14;
export const BRIDGE_CMD_QUERY_MODULE = 0x15;

export const BRIDGE_ENTER_MODE_INJECTED = 0;
export const BRIDGE_ENTER_MODE_PWA_ADC = 1;

export const BRIDGE_STATUS_OK = 0x0001;
export const BRIDGE_STATUS_SEQ_GAP = 0x0040;

export const FIRMWARE_MODULE_KINDS = {
  0: 'passthrough',
  1: 'delay',
  2: 'merger',
  3: 'debug_tone',
  4: 'cutoff',
};

export const MODULE_KIND_TO_FIRMWARE = Object.fromEntries(
  Object.entries(FIRMWARE_MODULE_KINDS).map(([k, v]) => [v, Number(k)]),
);

export const BRIDGE_AUDIO_BYTES = BUFFER_LEN * 2;
export const BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE = 4 + BRIDGE_AUDIO_BYTES * 2 + 8;
export const BRIDGE_EXCHANGE_RESPONSE_TELEMETRY_BYTES = 12;
export const BRIDGE_EXCHANGE_RESPONSE_PAYLOAD_SIZE =
  4 + BRIDGE_AUDIO_BYTES * 2 + 2 + 4 + BRIDGE_EXCHANGE_RESPONSE_TELEMETRY_BYTES;
export const BRIDGE_EXCHANGE_REQUEST_SIZE = BRIDGE_HEADER_SIZE + BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE;
export const BRIDGE_EXCHANGE_RESPONSE_SIZE = BRIDGE_HEADER_SIZE + BRIDGE_EXCHANGE_RESPONSE_PAYLOAD_SIZE;
export const BRIDGE_ACK_SIZE = BRIDGE_HEADER_SIZE + 6;

export function writeHeader(view, offset, command, mode = 0) {
  view.setUint32(offset, BRIDGE_MAGIC, true);
  view.setUint8(offset + 4, command);
  view.setUint8(offset + 5, mode);
  return offset + BRIDGE_HEADER_SIZE;
}

export function wrapLengthPrefixed(innerBuffer) {
  const out = new Uint8Array(BRIDGE_WIRE_PREFIX_SIZE + innerBuffer.length);
  const view = new DataView(out.buffer);
  view.setUint32(0, innerBuffer.length, true);
  out.set(innerBuffer, BRIDGE_WIRE_PREFIX_SIZE);
  return out;
}

export function buildEnterUsbFrame(mode = BRIDGE_ENTER_MODE_PWA_ADC) {
  const buffer = new ArrayBuffer(BRIDGE_HEADER_SIZE);
  const view = new DataView(buffer);
  writeHeader(view, 0, BRIDGE_CMD_ENTER_USB, mode);
  return wrapLengthPrefixed(new Uint8Array(buffer));
}

export function buildExitUsbFrame() {
  const buffer = new ArrayBuffer(BRIDGE_HEADER_SIZE);
  const view = new DataView(buffer);
  writeHeader(view, 0, BRIDGE_CMD_EXIT_USB);
  return wrapLengthPrefixed(new Uint8Array(buffer));
}

export function buildQueryModuleFrame() {
  const buffer = new ArrayBuffer(BRIDGE_HEADER_SIZE);
  const view = new DataView(buffer);
  writeHeader(view, 0, BRIDGE_CMD_QUERY_MODULE);
  return wrapLengthPrefixed(new Uint8Array(buffer));
}

export function buildExchangeRequest({ sequence, downstreamIn, upstreamIn }) {
  const buffer = new ArrayBuffer(BRIDGE_EXCHANGE_REQUEST_SIZE);
  const view = new DataView(buffer);
  const int16 = new Int16Array(buffer);
  let offset = writeHeader(view, 0, BRIDGE_CMD_EXCHANGE_USB);
  view.setUint32(offset, sequence >>> 0, true);
  offset += 4;
  int16.set(downstreamIn, offset / 2);
  offset += BRIDGE_AUDIO_BYTES;
  int16.set(upstreamIn, offset / 2);
  offset += BRIDGE_AUDIO_BYTES;
  view.setFloat32(offset, 0.5, true);
  offset += 4;
  view.setFloat32(offset, 0.5, true);
  return wrapLengthPrefixed(new Uint8Array(buffer));
}

export function parseInnerFrame(buffer) {
  const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
  const magic = view.getUint32(0, true);
  if (magic !== BRIDGE_MAGIC) {
    throw new Error(`bad magic: 0x${magic.toString(16)}`);
  }
  const command = view.getUint8(4);
  const mode = view.getUint8(5);
  return { view, command, mode };
}

export function parseExchangeResponse(buffer) {
  const { view, command } = parseInnerFrame(buffer);
  let offset = BRIDGE_HEADER_SIZE;
  const sequence = view.getUint32(offset, true);
  offset += 4;
  const int16 = new Int16Array(buffer.buffer, buffer.byteOffset + offset, BUFFER_LEN * 2);
  const downstreamOut = Int16Array.from(int16.subarray(0, BUFFER_LEN));
  const upstreamOut = Int16Array.from(int16.subarray(BUFFER_LEN, BUFFER_LEN * 2));
  offset += BRIDGE_AUDIO_BYTES * 2;
  const status = view.getUint16(offset, true);
  offset += 2;
  let timestampUs = 0;
  let primaryTelemetry;
  let secondaryTelemetry;
  if (offset + 4 <= buffer.byteLength) {
    timestampUs = view.getUint32(offset, true);
    offset += 4;
  }
  if (offset + 8 <= buffer.byteLength) {
    primaryTelemetry = view.getFloat32(offset, true);
    offset += 4;
    secondaryTelemetry = view.getFloat32(offset, true);
    offset += 4;
  }
  let processingUs;
  if (offset + 4 <= buffer.byteLength) {
    processingUs = view.getUint32(offset, true);
  }
  return {
    command,
    sequence,
    downstreamOut,
    upstreamOut,
    status,
    timestampUs,
    primaryTelemetry,
    secondaryTelemetry,
    processingUs,
  };
}

export function parseAck(inner) {
  const { view, command } = parseInnerFrame(inner);
  const sequence = view.getUint32(BRIDGE_HEADER_SIZE, true);
  const status = view.getUint16(BRIDGE_HEADER_SIZE + 4, true);
  return { command, sequence, status };
}

export function firmwareKindToName(kindId) {
  return FIRMWARE_MODULE_KINDS[kindId] ?? null;
}

export function statusIsOk(status) {
  return (status & BRIDGE_STATUS_OK) !== 0
    && (status & BRIDGE_STATUS_SEQ_GAP) === 0;
}
