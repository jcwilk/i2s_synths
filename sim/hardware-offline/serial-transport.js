import {
  BRIDGE_ACK_SIZE,
  BRIDGE_EXCHANGE_RESPONSE_SIZE,
  BRIDGE_STATUS_OK,
  buildEnterFrame,
  buildExitFrame,
  buildExchangeRequest,
  parseAck,
  parseExchangeResponse,
  BRIDGE_CMD_EXCHANGE,
  BRIDGE_CMD_LOOPBACK,
} from './frame-protocol.js';

export async function openSerialPort(portPath, baudRate = 115200) {
  const { SerialPort } = await import('serialport');
  const port = new SerialPort({ path: portPath, baudRate, autoOpen: false });
  await new Promise((resolve, reject) => {
    port.open((error) => (error ? reject(error) : resolve()));
  });
  await delay(1500);
  return port;
}

function delay(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

function readExact(port, length, timeoutMs = 10000) {
  return new Promise((resolve, reject) => {
    const chunks = [];
    let received = 0;
    const deadline = Date.now() + timeoutMs;

    const onData = (chunk) => {
      chunks.push(chunk);
      received += chunk.length;
      if (received >= length) {
        cleanup();
        resolve(Buffer.concat(chunks).subarray(0, length));
      } else if (Date.now() > deadline) {
        cleanup();
        reject(new Error(`serial read timeout (${received}/${length} bytes)`));
      }
    };

    const onError = (error) => {
      cleanup();
      reject(error);
    };

    const timer = setInterval(() => {
      if (Date.now() > deadline) {
        cleanup();
        reject(new Error(`serial read timeout (${received}/${length} bytes)`));
      }
    }, 50);

    function cleanup() {
      clearInterval(timer);
      port.off('data', onData);
      port.off('error', onError);
    }

    port.on('data', onData);
    port.on('error', onError);

    if (port.readableLength >= length) {
      const chunk = port.read(length);
      cleanup();
      resolve(chunk);
    }
  });
}

function writePort(port, buffer) {
  return new Promise((resolve, reject) => {
    port.write(buffer, (error) => (error ? reject(error) : resolve()));
  });
}

async function sendAckCommand(port, frame) {
  await writePort(port, frame);
  const ack = await readExact(port, BRIDGE_ACK_SIZE);
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
  const responseBuffer = await readExact(port, BRIDGE_EXCHANGE_RESPONSE_SIZE);
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
