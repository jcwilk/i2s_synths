import {
  BRIDGE_ACK_SIZE,
  BRIDGE_EXCHANGE_RESPONSE_SIZE,
  BRIDGE_STATUS_OK,
  BRIDGE_MAGIC,
  buildEnterFrame,
  buildExitFrame,
  buildExchangeRequest,
  parseAck,
  parseExchangeResponse,
  BRIDGE_CMD_EXCHANGE,
  BRIDGE_CMD_LOOPBACK,
} from './frame-protocol.js';

const MAGIC_BYTES = Buffer.from([
  BRIDGE_MAGIC & 0xff,
  (BRIDGE_MAGIC >> 8) & 0xff,
  (BRIDGE_MAGIC >> 16) & 0xff,
  (BRIDGE_MAGIC >> 24) & 0xff,
]);

async function drainIncoming(port, quietMs = 250, maxMs = 8000) {
  const started = Date.now();
  let lastData = started;
  let sawSetupComplete = false;
  return new Promise((resolve) => {
    const onData = (chunk) => {
      lastData = Date.now();
      if (chunk.includes(Buffer.from('Setup complete.'))) {
        sawSetupComplete = true;
      }
    };
    port.on('data', onData);
    const timer = setInterval(() => {
      const now = Date.now();
      const bootReady = sawSetupComplete && now - lastData >= quietMs;
      const timedOut = now - started >= maxMs;
      if (bootReady || timedOut) {
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

export async function openSerialPort(portPath, baudRate = 115200) {
  const { SerialPort } = await import('serialport');
  const port = new SerialPort({ path: portPath, baudRate, autoOpen: false });
  await new Promise((resolve, reject) => {
    port.open((error) => (error ? reject(error) : resolve()));
  });
  await drainIncoming(port);
  return port;
}

function delay(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

function readSyncedFrame(port, frameLength, timeoutMs = 15000) {
  return new Promise((resolve, reject) => {
    let buffer = Buffer.alloc(0);
    const deadline = Date.now() + timeoutMs;

    const tryExtract = () => {
      const idx = buffer.indexOf(MAGIC_BYTES);
      if (idx < 0) {
        if (buffer.length > MAGIC_BYTES.length - 1) {
          buffer = buffer.subarray(buffer.length - (MAGIC_BYTES.length - 1));
        }
        return false;
      }
      if (idx > 0) {
        buffer = buffer.subarray(idx);
      }
      if (buffer.length >= frameLength) {
        cleanup();
        resolve(buffer.subarray(0, frameLength));
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
        reject(new Error(`serial sync timeout (${buffer.length} bytes buffered)`));
      }
    }, 50);

    function cleanup() {
      clearInterval(timer);
      port.off('data', onData);
      port.off('error', onError);
    }

    port.on('data', onData);
    port.on('error', onError);

    if (port.readableLength > 0) {
      buffer = Buffer.concat([buffer, port.read(port.readableLength)]);
      if (tryExtract()) {
        return;
      }
    }
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

async function sendAckCommand(port, frame) {
  await writePort(port, frame);
  const ack = await readSyncedFrame(port, BRIDGE_ACK_SIZE);
  const parsed = parseAck(ack);
  if (parsed.status !== BRIDGE_STATUS_OK) {
    throw new Error(`device returned status 0x${parsed.status.toString(16)} for command 0x${parsed.command.toString(16)}`);
  }
  return parsed;
}

export async function enterOfflineMode(port) {
  return sendAckCommand(port, buildEnterFrame());
}

export async function exitOfflineMode(port) {
  return sendAckCommand(port, buildExitFrame());
}

export async function exchangePeriod(port, periodSpec, sequence, command = BRIDGE_CMD_EXCHANGE) {
  const frame = buildExchangeRequest({
    command,
    sequence,
    downstreamIn: periodSpec.downstreamIn,
    upstreamIn: periodSpec.upstreamIn,
    primary: periodSpec.primary ?? 0,
    secondary: periodSpec.secondary ?? 0,
  });
  await writePort(port, frame);
  const responseBuffer = await readSyncedFrame(port, BRIDGE_EXCHANGE_RESPONSE_SIZE);
  const response = parseExchangeResponse(responseBuffer);
  if (response.status !== BRIDGE_STATUS_OK) {
    throw new Error(`exchange status 0x${response.status.toString(16)} at sequence ${sequence}`);
  }
  return response;
}

export async function runLoopbackSelfTest(port, periodSpec, sequence = 0) {
  const response = await exchangePeriod(port, periodSpec, sequence, BRIDGE_CMD_LOOPBACK);
  return response;
}

export function closeSerialPort(port) {
  return new Promise((resolve) => {
    if (!port || !port.isOpen) {
      resolve();
      return;
    }
    port.close(() => resolve());
  });
}
