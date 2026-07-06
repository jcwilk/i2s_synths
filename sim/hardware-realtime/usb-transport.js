import {
  BRIDGE_ACK_SIZE,
  BRIDGE_EXCHANGE_RESPONSE_SIZE,
  BRIDGE_STATUS_OK,
  BRIDGE_MAGIC,
  BRIDGE_WIRE_PREFIX_SIZE,
  buildEnterUsbFrame,
  buildExitUsbFrame,
  buildExchangeRequest,
  parseAck,
  parseExchangeResponse,
  statusIsOk,
  BRIDGE_CMD_EXCHANGE_USB,
  BRIDGE_CMD_LOOPBACK_USB,
  BRIDGE_CMD_EXCHANGE_USB_PACK4,
  buildPack4ExchangeRequest,
  parsePack4ExchangeResponse,
  BRIDGE_CMD_EXCHANGE_USB_22K_MONO,
  buildMono22kExchangeRequest,
  parseMono22kExchangeResponse,
} from './frame-protocol.js';

const MAGIC_BYTES = Buffer.from([
  BRIDGE_MAGIC & 0xff,
  (BRIDGE_MAGIC >> 8) & 0xff,
  (BRIDGE_MAGIC >> 16) & 0xff,
  (BRIDGE_MAGIC >> 24) & 0xff,
]);

const MAX_WIRE_INNER_BYTES = 33000;

function attachBridgeReader(port) {
  if (port._bridgeReader) {
    return port._bridgeReader;
  }

  const reader = {
    buffer: Buffer.alloc(0),
    waiters: [],
    onData(chunk) {
      reader.buffer = Buffer.concat([reader.buffer, chunk]);
      reader.flushWaiters();
    },
    onError(error) {
      const waiters = reader.waiters;
      reader.waiters = [];
      for (const waiter of waiters) {
        waiter.reject(error);
      }
    },
    tryTake() {
      while (reader.buffer.length >= BRIDGE_WIRE_PREFIX_SIZE) {
        const wireLen = reader.buffer.readUInt32LE(0);
        if (wireLen === 0 || wireLen > MAX_WIRE_INNER_BYTES) {
          reader.buffer = reader.buffer.subarray(1);
          continue;
        }
        const total = BRIDGE_WIRE_PREFIX_SIZE + wireLen;
        if (reader.buffer.length < total) {
          return null;
        }
        const inner = reader.buffer.subarray(BRIDGE_WIRE_PREFIX_SIZE, total);
        reader.buffer = reader.buffer.subarray(total);
        return inner;
      }
      return null;
    },
    flushWaiters() {
      if (reader.waiters.length === 0) {
        return;
      }
      let inner = reader.tryTake();
      while (inner && reader.waiters.length > 0) {
        const waiter = reader.waiters.shift();
        clearTimeout(waiter.timer);
        waiter.resolve(inner);
        inner = reader.tryTake();
      }
    },
    reset() {
      reader.buffer = Buffer.alloc(0);
      while (port.readableLength > 0) {
        port.read(port.readableLength);
      }
    },
    readFrame(timeoutMs = 15000) {
      const existing = reader.tryTake();
      if (existing) {
        return Promise.resolve(existing);
      }
      return new Promise((resolve, reject) => {
        const waiter = {
          resolve,
          reject,
          timer: setTimeout(() => {
            const index = reader.waiters.indexOf(waiter);
            if (index >= 0) {
              reader.waiters.splice(index, 1);
            }
            const buffered = reader.buffer.length;
            reader.reset();
            reject(new Error(`audio sync timeout (${buffered} bytes buffered)`));
          }, timeoutMs),
        };
        reader.waiters.push(waiter);
      });
    },
  };

  port.on('data', reader.onData);
  port.on('error', reader.onError);
  port._bridgeReader = reader;
  if (port.readableLength > 0) {
    reader.onData(port.read(port.readableLength));
  }
  return reader;
}

export async function openAudioPort(portPath, baudRate = 115200) {
  const { SerialPort } = await import('serialport');
  const port = new SerialPort({
    path: portPath,
    baudRate,
    autoOpen: false,
    highWaterMark: 65536,
  });
  await new Promise((resolve, reject) => {
    port.open((error) => (error ? reject(error) : resolve()));
  });
  attachBridgeReader(port);
  await drainIncoming(port, 500);
  return port;
}

async function drainIncoming(port, quietMs = 250, maxMs = 5000) {
  const started = Date.now();
  let lastData = started;
  return new Promise((resolve) => {
    const onData = () => {
      lastData = Date.now();
    };
    port.on('data', onData);
    const timer = setInterval(() => {
      const now = Date.now();
      if (now - lastData >= quietMs || now - started >= maxMs) {
        clearInterval(timer);
        port.off('data', onData);
        if (port._bridgeReader) {
          port._bridgeReader.reset();
        } else {
          while (port.readableLength > 0) {
            port.read();
          }
        }
        resolve();
      }
    }, 50);
  });
}

function writePort(port, buffer) {
  return new Promise((resolve, reject) => {
    port.write(buffer, (error) => {
      if (error) {
        reject(error);
        return;
      }
      port.drain((drainError) => (drainError ? reject(drainError) : resolve()));
    });
  });
}

export function writeFrame(port, buffer) {
  return writePort(port, buffer);
}

export function readResponseFrame(port, timeoutMs = 15000) {
  return readLengthPrefixedFrame(port, timeoutMs);
}

function readLengthPrefixedFrame(port, timeoutMs = 15000) {
  return attachBridgeReader(port).readFrame(timeoutMs);
}

async function sendAckCommand(port, frame) {
  await writePort(port, frame);
  const inner = await readLengthPrefixedFrame(port);
  const parsed = parseAck(inner);
  if (parsed.status !== BRIDGE_STATUS_OK) {
    throw new Error(`device status 0x${parsed.status.toString(16)} for command 0x${parsed.command.toString(16)}`);
  }
  return parsed;
}

export async function enterUsbNeighborMode(port) {
  port._bridgeReader?.reset();
  return sendAckCommand(port, buildEnterUsbFrame());
}

export async function exitUsbNeighborMode(port) {
  const ack = await sendAckCommand(port, buildExitUsbFrame());
  port._bridgeReader?.reset();
  return ack;
}

export async function exchangeUsbPeriod(port, periodSpec, sequence, command = BRIDGE_CMD_EXCHANGE_USB) {
  const frame = buildExchangeRequest({
    command,
    sequence,
    downstreamIn: periodSpec.downstreamIn,
    upstreamIn: periodSpec.upstreamIn,
    primary: periodSpec.primary ?? 0.5,
    secondary: periodSpec.secondary ?? 0.5,
  });
  const sendAt = performance.now();
  await writeFrame(port, frame);
  const inner = await readResponseFrame(port);
  const receiveAt = performance.now();
  const response = parseExchangeResponse(inner);
  if (!statusIsOk(response.status)) {
    throw new Error(`exchange status 0x${response.status.toString(16)} at sequence ${sequence}`);
  }
  return { ...response, roundTripMs: receiveAt - sendAt };
}

export async function exchangeUsbPack4(port, packSpec, baseSequence) {
  const frame = buildPack4ExchangeRequest({
    baseSequence,
    downstreamIn: packSpec.downstreamIn,
    upstreamIn: packSpec.upstreamIn,
    primary: packSpec.primary ?? 0.5,
    secondary: packSpec.secondary ?? 0.5,
  });
  const sendAt = performance.now();
  await writeFrame(port, frame);
  const inner = await readResponseFrame(port, 30000);
  const receiveAt = performance.now();
  const response = parsePack4ExchangeResponse(inner);
  if (!statusIsOk(response.status)) {
    throw new Error(`pack4 status 0x${response.status.toString(16)} at base ${baseSequence}`);
  }
  return { ...response, roundTripMs: receiveAt - sendAt };
}

export async function exchangeUsb22kMono(port, periodSpec, sequence) {
  const frame = buildMono22kExchangeRequest({
    sequence,
    downstreamIn: periodSpec.downstreamIn,
    upstreamIn: periodSpec.upstreamIn,
    primary: periodSpec.primary ?? 0.5,
    secondary: periodSpec.secondary ?? 0.5,
  });
  const sendAt = performance.now();
  await writeFrame(port, frame);
  const inner = await readResponseFrame(port, 15000);
  const receiveAt = performance.now();
  const response = parseMono22kExchangeResponse(inner);
  if (!statusIsOk(response.status)) {
    throw new Error(`22k-mono status 0x${response.status.toString(16)} at sequence ${sequence}`);
  }
  return { ...response, roundTripMs: receiveAt - sendAt };
}

export async function runUsbLoopbackSelfTest(port, periodSpec, sequence = 0) {
  return exchangeUsbPeriod(port, periodSpec, sequence, BRIDGE_CMD_LOOPBACK_USB);
}

export function closeAudioPort(port) {
  return new Promise((resolve) => {
    if (!port || !port.isOpen) {
      resolve();
      return;
    }
    port.close(() => resolve());
  });
}

export { MAGIC_BYTES };
