/** Shared host↔device binary frame layout for hardware module bridge (mono 22.05 kHz). */

export const SAMPLE_RATE = 22050;
export const BUFFER_LEN = 128;
export const BRIDGE_MAGIC = 0x31424d48; // 'HMB1' little-endian
export const BRIDGE_HEADER_SIZE = 6;

export const BRIDGE_CMD_ENTER = 0x01;
export const BRIDGE_CMD_EXIT = 0x02;
export const BRIDGE_CMD_EXCHANGE = 0x03;
export const BRIDGE_CMD_LOOPBACK = 0x04;

export const BRIDGE_STATUS_OK = 0x0001;
export const BRIDGE_STATUS_ERR_BAD_MAGIC = 0x0002;
export const BRIDGE_STATUS_ERR_BAD_CMD = 0x0003;
export const BRIDGE_STATUS_ERR_NOT_ACTIVE = 0x0004;
export const BRIDGE_STATUS_ERR_SHORT_READ = 0x0005;

export const BRIDGE_AUDIO_BYTES = BUFFER_LEN * 2;
export const BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE = 4 + BRIDGE_AUDIO_BYTES * 2 + 8;
export const BRIDGE_EXCHANGE_RESPONSE_PAYLOAD_SIZE = 4 + BRIDGE_AUDIO_BYTES * 2 + 2;
export const BRIDGE_EXCHANGE_REQUEST_SIZE = BRIDGE_HEADER_SIZE + BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE;
export const BRIDGE_EXCHANGE_RESPONSE_SIZE = BRIDGE_HEADER_SIZE + BRIDGE_EXCHANGE_RESPONSE_PAYLOAD_SIZE;
export const BRIDGE_ACK_SIZE = BRIDGE_HEADER_SIZE + 6;

export function writeHeader(view, offset, command) {
  view.setUint32(offset, BRIDGE_MAGIC, true);
  view.setUint8(offset + 4, command);
  view.setUint8(offset + 5, 0);
  return offset + BRIDGE_HEADER_SIZE;
}

export function buildExchangeRequest({
  command = BRIDGE_CMD_EXCHANGE,
  sequence,
  downstreamIn,
  upstreamIn,
  primary = 0,
  secondary = 0,
}) {
  const buffer = new ArrayBuffer(BRIDGE_EXCHANGE_REQUEST_SIZE);
  const view = new DataView(buffer);
  const int16 = new Int16Array(buffer);
  let offset = writeHeader(view, 0, command);
  view.setUint32(offset, sequence >>> 0, true);
  offset += 4;
  int16.set(downstreamIn, offset / 2);
  offset += BRIDGE_AUDIO_BYTES;
  int16.set(upstreamIn, offset / 2);
  offset += BRIDGE_AUDIO_BYTES;
  view.setFloat32(offset, primary, true);
  offset += 4;
  view.setFloat32(offset, secondary, true);
  return Buffer.from(buffer);
}

export function parseExchangeResponse(buffer) {
  const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
  const magic = view.getUint32(0, true);
  if (magic !== BRIDGE_MAGIC) {
    throw new Error(`bad response magic: 0x${magic.toString(16)}`);
  }
  const command = view.getUint8(4);
  let offset = BRIDGE_HEADER_SIZE;
  const sequence = view.getUint32(offset, true);
  offset += 4;
  const int16 = new Int16Array(buffer.buffer, buffer.byteOffset + offset, BUFFER_LEN * 2);
  const downstreamOut = Int16Array.from(int16.subarray(0, BUFFER_LEN));
  const upstreamOut = Int16Array.from(int16.subarray(BUFFER_LEN, BUFFER_LEN * 2));
  offset += BRIDGE_AUDIO_BYTES * 2;
  const status = view.getUint16(offset, true);
  return { command, sequence, downstreamOut, upstreamOut, status };
}

export function parseAck(buffer) {
  const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
  const magic = view.getUint32(0, true);
  if (magic !== BRIDGE_MAGIC) {
    throw new Error(`bad ack magic: 0x${magic.toString(16)}`);
  }
  const command = view.getUint8(4);
  const sequence = view.getUint32(BRIDGE_HEADER_SIZE, true);
  const status = view.getUint16(BRIDGE_HEADER_SIZE + 4, true);
  return { command, sequence, status };
}

export function buildEnterFrame() {
  const buffer = new ArrayBuffer(BRIDGE_HEADER_SIZE);
  const view = new DataView(buffer);
  writeHeader(view, 0, BRIDGE_CMD_ENTER);
  return Buffer.from(buffer);
}

export function buildExitFrame() {
  const buffer = new ArrayBuffer(BRIDGE_HEADER_SIZE);
  const view = new DataView(buffer);
  writeHeader(view, 0, BRIDGE_CMD_EXIT);
  return Buffer.from(buffer);
}
