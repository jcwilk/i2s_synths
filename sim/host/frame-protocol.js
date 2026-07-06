/** Browser-side frame protocol (mirrors sim/hardware-bridge/frame-protocol.js). */

export {
  SAMPLE_RATE,
  BUFFER_LEN,
  PERIOD_MS,
} from '../shared/audio-constants.js';

export const BRIDGE_MAGIC = 0x31424d48;
export const BRIDGE_HEADER_SIZE = 6;
export const BRIDGE_WIRE_PREFIX_SIZE = 4;

export const BRIDGE_CMD_EXCHANGE_USB = 0x13;

export const FIRMWARE_MODULE_KINDS = {
  0: 'passthrough',
  1: 'delay',
  2: 'merger',
  3: 'debug_tone',
  4: 'cutoff',
};

export const BRIDGE_AUDIO_BYTES = BUFFER_LEN * 2;
export const BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE = 4 + BRIDGE_AUDIO_BYTES * 2 + 8;
export const BRIDGE_EXCHANGE_REQUEST_SIZE = BRIDGE_HEADER_SIZE + BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE;

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

export function parseExchangeResponse(buffer) {
  const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
  let offset = BRIDGE_HEADER_SIZE;
  const sequence = view.getUint32(offset, true);
  offset += 4;
  const int16 = new Int16Array(buffer.buffer, buffer.byteOffset + offset, BUFFER_LEN * 2);
  const downstreamOut = Int16Array.from(int16.subarray(0, BUFFER_LEN));
  const upstreamOut = Int16Array.from(int16.subarray(BUFFER_LEN, BUFFER_LEN * 2));
  offset += BRIDGE_AUDIO_BYTES * 2;
  const status = view.getUint16(offset, true);
  return { sequence, downstreamOut, upstreamOut, status };
}
