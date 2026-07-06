/** Phase 1 USB realtime frame layout (length-prefixed on audio CDC). */

export const BUFFER_LEN = 512;
export const BRIDGE_MAGIC = 0x31424d48;
export const BRIDGE_HEADER_SIZE = 6;
export const BRIDGE_WIRE_PREFIX_SIZE = 4;

export const BRIDGE_CMD_ENTER_USB = 0x11;
export const BRIDGE_CMD_EXIT_USB = 0x12;
export const BRIDGE_CMD_EXCHANGE_USB = 0x13;
export const BRIDGE_CMD_LOOPBACK_USB = 0x14;
export const BRIDGE_CMD_EXCHANGE_USB_PACK4 = 0x15;
export const BRIDGE_USB_PACK_PERIODS = 4;
export const BRIDGE_CMD_EXCHANGE_USB_22K_MONO = 0x16;
export const SPIKE_SAMPLE_RATE = 22050;
export const SPIKE_MONO_BUFFER_LEN = 128;
export const SPIKE_MONO_AUDIO_BYTES = SPIKE_MONO_BUFFER_LEN * 2;

export const BRIDGE_STATUS_OK = 0x0001;
export const BRIDGE_STATUS_ERR_BAD_MAGIC = 0x0002;
export const BRIDGE_STATUS_ERR_BAD_CMD = 0x0003;
export const BRIDGE_STATUS_ERR_NOT_ACTIVE = 0x0004;
export const BRIDGE_STATUS_ERR_SHORT_READ = 0x0005;
export const BRIDGE_STATUS_ERR_BAD_LENGTH = 0x0006;
export const BRIDGE_STATUS_OVERRUN = 0x0010;
export const BRIDGE_STATUS_UNDERRUN = 0x0020;
export const BRIDGE_STATUS_SEQ_GAP = 0x0040;

export const BRIDGE_AUDIO_BYTES = BUFFER_LEN * 2;
export const BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE = 4 + BRIDGE_AUDIO_BYTES * 2 + 8;
export const BRIDGE_EXCHANGE_RESPONSE_PAYLOAD_SIZE = 4 + BRIDGE_AUDIO_BYTES * 2 + 2 + 4;
export const BRIDGE_EXCHANGE_REQUEST_SIZE = BRIDGE_HEADER_SIZE + BRIDGE_EXCHANGE_REQUEST_PAYLOAD_SIZE;
export const BRIDGE_EXCHANGE_RESPONSE_SIZE = BRIDGE_HEADER_SIZE + BRIDGE_EXCHANGE_RESPONSE_PAYLOAD_SIZE;
export const BRIDGE_PACK4_AUDIO_BYTES = BRIDGE_AUDIO_BYTES * BRIDGE_USB_PACK_PERIODS;
export const BRIDGE_PACK4_REQUEST_PAYLOAD_SIZE = 4 + BRIDGE_PACK4_AUDIO_BYTES * 2 + 8;
export const BRIDGE_PACK4_RESPONSE_PAYLOAD_SIZE = 4 + BRIDGE_PACK4_AUDIO_BYTES * 2 + 2 + 4;
export const BRIDGE_PACK4_REQUEST_SIZE = BRIDGE_HEADER_SIZE + BRIDGE_PACK4_REQUEST_PAYLOAD_SIZE;
export const BRIDGE_PACK4_RESPONSE_SIZE = BRIDGE_HEADER_SIZE + BRIDGE_PACK4_RESPONSE_PAYLOAD_SIZE;
export const SPIKE_MONO_REQUEST_PAYLOAD_SIZE = 4 + SPIKE_MONO_AUDIO_BYTES * 2 + 8;
export const SPIKE_MONO_RESPONSE_PAYLOAD_SIZE = 4 + SPIKE_MONO_AUDIO_BYTES * 2 + 2 + 4;
export const SPIKE_MONO_REQUEST_SIZE = BRIDGE_HEADER_SIZE + SPIKE_MONO_REQUEST_PAYLOAD_SIZE;
export const SPIKE_MONO_RESPONSE_SIZE = BRIDGE_HEADER_SIZE + SPIKE_MONO_RESPONSE_PAYLOAD_SIZE;
export const BRIDGE_ACK_SIZE = BRIDGE_HEADER_SIZE + 6;

export const PERIOD_MS = (BUFFER_LEN / 2 / 44100) * 1000;
export const SPIKE_MONO_PERIOD_MS = (SPIKE_MONO_BUFFER_LEN / SPIKE_SAMPLE_RATE) * 1000;
export const PACK4_AUDIO_MS = PERIOD_MS * BRIDGE_USB_PACK_PERIODS;
export const RESPONSE_DEADLINE_MS = PERIOD_MS * 3;
export const ACCEPTANCE_DURATION_S = 60;
export const MIN_EXCHANGES = 10000;
export const LATENCY_P50_MAX_MS = 12;
export const LATENCY_P99_MAX_MS = 20;

export function writeHeader(view, offset, command) {
  view.setUint32(offset, BRIDGE_MAGIC, true);
  view.setUint8(offset + 4, command);
  view.setUint8(offset + 5, 0);
  return offset + BRIDGE_HEADER_SIZE;
}

export function wrapLengthPrefixed(innerBuffer) {
  const out = Buffer.alloc(BRIDGE_WIRE_PREFIX_SIZE + innerBuffer.length);
  out.writeUInt32LE(innerBuffer.length, 0);
  innerBuffer.copy(out, BRIDGE_WIRE_PREFIX_SIZE);
  return out;
}

export function buildExchangeRequest({
  command = BRIDGE_CMD_EXCHANGE_USB,
  sequence,
  downstreamIn,
  upstreamIn,
  primary = 0.5,
  secondary = 0.5,
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
  return wrapLengthPrefixed(Buffer.from(buffer));
}

export function buildPack4ExchangeRequest({
  command = BRIDGE_CMD_EXCHANGE_USB_PACK4,
  baseSequence,
  downstreamIn,
  upstreamIn,
  primary = 0.5,
  secondary = 0.5,
}) {
  const buffer = new ArrayBuffer(BRIDGE_PACK4_REQUEST_SIZE);
  const view = new DataView(buffer);
  const int16 = new Int16Array(buffer);
  let offset = writeHeader(view, 0, command);
  view.setUint32(offset, baseSequence >>> 0, true);
  offset += 4;
  int16.set(downstreamIn, offset / 2);
  offset += BRIDGE_PACK4_AUDIO_BYTES;
  int16.set(upstreamIn, offset / 2);
  offset += BRIDGE_PACK4_AUDIO_BYTES;
  view.setFloat32(offset, primary, true);
  offset += 4;
  view.setFloat32(offset, secondary, true);
  return wrapLengthPrefixed(Buffer.from(buffer));
}

export function deterministicPack4Pattern(baseSequence) {
  const downstreamIn = new Int16Array(BUFFER_LEN * BRIDGE_USB_PACK_PERIODS);
  const upstreamIn = new Int16Array(BUFFER_LEN * BRIDGE_USB_PACK_PERIODS);
  for (let period = 0; period < BRIDGE_USB_PACK_PERIODS; period++) {
    const { downstreamIn: ds, upstreamIn: us } = deterministicPattern(baseSequence + period);
    downstreamIn.set(ds, period * BUFFER_LEN);
    upstreamIn.set(us, period * BUFFER_LEN);
  }
  return { downstreamIn, upstreamIn };
}

export function parsePack4ExchangeResponse(inner) {
  const buf = Buffer.from(inner);
  const { command } = parseInnerFrame(buf);
  let offset = BRIDGE_HEADER_SIZE;
  const sequence = buf.readUInt32LE(offset);
  offset += 4;
  const downstreamOut = new Int16Array(BUFFER_LEN * BRIDGE_USB_PACK_PERIODS);
  const upstreamOut = new Int16Array(BUFFER_LEN * BRIDGE_USB_PACK_PERIODS);
  for (let i = 0; i < downstreamOut.length; i++) {
    downstreamOut[i] = buf.readInt16LE(offset + i * 2);
  }
  offset += BRIDGE_PACK4_AUDIO_BYTES;
  for (let i = 0; i < upstreamOut.length; i++) {
    upstreamOut[i] = buf.readInt16LE(offset + i * 2);
  }
  offset += BRIDGE_PACK4_AUDIO_BYTES;
  const status = buf.readUInt16LE(offset);
  offset += 2;
  const timestampUs = buf.readUInt32LE(offset);
  return { command, sequence, downstreamOut, upstreamOut, status, timestampUs };
}

export function buildMono22kExchangeRequest({
  command = BRIDGE_CMD_EXCHANGE_USB_22K_MONO,
  sequence,
  downstreamIn,
  upstreamIn,
  primary = 0.5,
  secondary = 0.5,
}) {
  const buffer = new ArrayBuffer(SPIKE_MONO_REQUEST_SIZE);
  const view = new DataView(buffer);
  const int16 = new Int16Array(buffer);
  let offset = writeHeader(view, 0, command);
  view.setUint32(offset, sequence >>> 0, true);
  offset += 4;
  int16.set(downstreamIn, offset / 2);
  offset += SPIKE_MONO_AUDIO_BYTES;
  int16.set(upstreamIn, offset / 2);
  offset += SPIKE_MONO_AUDIO_BYTES;
  view.setFloat32(offset, primary, true);
  offset += 4;
  view.setFloat32(offset, secondary, true);
  return wrapLengthPrefixed(Buffer.from(buffer));
}

export function parseMono22kExchangeResponse(inner) {
  const buf = Buffer.from(inner);
  const { command } = parseInnerFrame(buf);
  let offset = BRIDGE_HEADER_SIZE;
  const sequence = buf.readUInt32LE(offset);
  offset += 4;
  const downstreamOut = new Int16Array(SPIKE_MONO_BUFFER_LEN);
  const upstreamOut = new Int16Array(SPIKE_MONO_BUFFER_LEN);
  for (let i = 0; i < SPIKE_MONO_BUFFER_LEN; i++) {
    downstreamOut[i] = buf.readInt16LE(offset + i * 2);
  }
  offset += SPIKE_MONO_AUDIO_BYTES;
  for (let i = 0; i < SPIKE_MONO_BUFFER_LEN; i++) {
    upstreamOut[i] = buf.readInt16LE(offset + i * 2);
  }
  offset += SPIKE_MONO_AUDIO_BYTES;
  const status = buf.readUInt16LE(offset);
  offset += 2;
  const timestampUs = buf.readUInt32LE(offset);
  return { command, sequence, downstreamOut, upstreamOut, status, timestampUs };
}

export function deterministicMono22kPattern(sequence) {
  const downstreamIn = new Int16Array(SPIKE_MONO_BUFFER_LEN);
  const upstreamIn = new Int16Array(SPIKE_MONO_BUFFER_LEN);
  let lfsr = (sequence + 1) >>> 0;
  for (let i = 0; i < SPIKE_MONO_BUFFER_LEN; i++) {
    lfsr ^= lfsr << 13;
    lfsr ^= lfsr >>> 17;
    lfsr ^= lfsr << 5;
    lfsr >>>= 0;
    const sample = (lfsr & 0x7fff) - (sequence % 97);
    downstreamIn[i] = sample;
    upstreamIn[i] = sample ^ 0x1555;
  }
  return { downstreamIn, upstreamIn };
}

export function buildEnterUsbFrame() {
  const buffer = new ArrayBuffer(BRIDGE_HEADER_SIZE);
  const view = new DataView(buffer);
  writeHeader(view, 0, BRIDGE_CMD_ENTER_USB);
  return wrapLengthPrefixed(Buffer.from(buffer));
}

export function buildExitUsbFrame() {
  const buffer = new ArrayBuffer(BRIDGE_HEADER_SIZE);
  const view = new DataView(buffer);
  writeHeader(view, 0, BRIDGE_CMD_EXIT_USB);
  return wrapLengthPrefixed(Buffer.from(buffer));
}

export function parseInnerFrame(buffer) {
  const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
  const magic = view.getUint32(0, true);
  if (magic !== BRIDGE_MAGIC) {
    throw new Error(`bad magic: 0x${magic.toString(16)}`);
  }
  const command = view.getUint8(4);
  return { view, command };
}

export function parseExchangeResponse(inner) {
  const { view, command } = parseInnerFrame(inner);
  let offset = BRIDGE_HEADER_SIZE;
  const sequence = view.getUint32(offset, true);
  offset += 4;
  const int16 = new Int16Array(inner.buffer, inner.byteOffset + offset, BUFFER_LEN * 2);
  const downstreamOut = Int16Array.from(int16.subarray(0, BUFFER_LEN));
  const upstreamOut = Int16Array.from(int16.subarray(BUFFER_LEN, BUFFER_LEN * 2));
  offset += BRIDGE_AUDIO_BYTES * 2;
  const status = view.getUint16(offset, true);
  offset += 2;
  const timestampUs = view.getUint32(offset, true);
  return { command, sequence, downstreamOut, upstreamOut, status, timestampUs };
}

export function parseAck(inner) {
  const { view, command } = parseInnerFrame(inner);
  const sequence = view.getUint32(BRIDGE_HEADER_SIZE, true);
  const status = view.getUint16(BRIDGE_HEADER_SIZE + 4, true);
  return { command, sequence, status };
}

export function statusIsOk(status) {
  return (status & BRIDGE_STATUS_OK) !== 0
    && (status & BRIDGE_STATUS_OVERRUN) === 0
    && (status & BRIDGE_STATUS_UNDERRUN) === 0
    && (status & BRIDGE_STATUS_SEQ_GAP) === 0;
}

export function deterministicPattern(sequence) {
  const downstreamIn = new Int16Array(BUFFER_LEN);
  const upstreamIn = new Int16Array(BUFFER_LEN);
  let lfsr = (sequence + 1) >>> 0;
  for (let i = 0; i < BUFFER_LEN; i += 2) {
    lfsr ^= lfsr << 13;
    lfsr ^= lfsr >>> 17;
    lfsr ^= lfsr << 5;
    lfsr >>>= 0;
    const sample = (lfsr & 0x7fff) - (sequence % 97);
    downstreamIn[i] = sample;
    downstreamIn[i + 1] = -sample;
    upstreamIn[i] = sample ^ 0x1555;
    upstreamIn[i + 1] = -sample ^ 0x1555;
  }
  return { downstreamIn, upstreamIn };
}

export function arraysEqual(a, b) {
  if (a.length !== b.length) {
    return false;
  }
  for (let i = 0; i < a.length; i++) {
    if (a[i] !== b[i]) {
      return false;
    }
  }
  return true;
}

export function percentile(sorted, p) {
  if (sorted.length === 0) {
    return 0;
  }
  const idx = Math.min(sorted.length - 1, Math.ceil((p / 100) * sorted.length) - 1);
  return sorted[Math.max(0, idx)];
}
