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
} from './frame-protocol.js';

const MAGIC_BYTES = Buffer.from([
  BRIDGE_MAGIC & 0xff,
  (BRIDGE_MAGIC >> 8) & 0xff,
  (BRIDGE_MAGIC >> 16) & 0xff,
  (BRIDGE_MAGIC >> 24) & 0xff,
]);

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
        while (port.readableLength > 0) {
          port.read();
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
  return new Promise((resolve, reject) => {
    let buffer = Buffer.alloc(0);
    const deadline = Date.now() + timeoutMs;

    const tryExtract = () => {
      if (buffer.length < BRIDGE_WIRE_PREFIX_SIZE) {
        return false;
      }
      const wireLen = buffer.readUInt32LE(0);
      const total = BRIDGE_WIRE_PREFIX_SIZE + wireLen;
      if (buffer.length >= total) {
        cleanup();
        resolve(buffer.subarray(BRIDGE_WIRE_PREFIX_SIZE, total));
        return true;
      }
      return false;
    };

    const onData = (chunk) => {
      buffer = Buffer.concat([buffer, chunk]);
      tryExtract();
    };

    const onError = (error) => {
      cleanup();
      reject(error);
    };

    const timer = setInterval(() => {
      if (Date.now() > deadline) {
        cleanup();
        reject(new Error(`audio sync timeout (${buffer.length} bytes buffered)`));
      }
      tryExtract();
    }, 1);

    function cleanup() {
      clearInterval(timer);
      port.off('data', onData);
      port.off('error', onError);
    }

    port.on('data', onData);
    port.on('error', onError);

    if (port.readableLength > 0) {
      buffer = Buffer.concat([buffer, port.read(port.readableLength)]);
      tryExtract();
    }
  });
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
  return sendAckCommand(port, buildEnterUsbFrame());
}

export async function exitUsbNeighborMode(port) {
  return sendAckCommand(port, buildExitUsbFrame());
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
