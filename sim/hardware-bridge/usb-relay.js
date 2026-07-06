import {
  BRIDGE_STATUS_OK,
  buildEnterUsbFrame,
  buildExitUsbFrame,
  buildQueryModuleFrame,
  parseAck,
  parseExchangeResponse,
  statusIsOk,
  BRIDGE_ENTER_MODE_PWA_ADC,
  BRIDGE_WIRE_PREFIX_SIZE,
} from './frame-protocol.js';

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

async function readLengthPrefixedFrame(port, timeoutMs = 15000) {
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

export async function enterUsbNeighborMode(port, mode = BRIDGE_ENTER_MODE_PWA_ADC) {
  port._bridgeReader?.reset();
  return sendAckCommand(port, buildEnterUsbFrame(mode));
}

export async function exitUsbNeighborMode(port) {
  const ack = await sendAckCommand(port, buildExitUsbFrame());
  port._bridgeReader?.reset();
  return ack;
}

export async function queryModuleKind(port) {
  const ack = await sendAckCommand(port, buildQueryModuleFrame());
  return ack.sequence;
}

export async function relayExchange(port, requestBytes) {
  await writePort(port, requestBytes);
  const inner = await readLengthPrefixedFrame(port);
  const response = parseExchangeResponse(inner);
  if (!statusIsOk(response.status)) {
    throw new Error(`exchange status 0x${response.status.toString(16)} at sequence ${response.sequence}`);
  }
  return inner;
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

export async function listSerialPorts() {
  const { SerialPort } = await import('serialport');
  return SerialPort.list();
}

export function isLikelyEsp32(portInfo) {
  const text = `${portInfo.manufacturer ?? ''} ${portInfo.vendorId ?? ''} ${portInfo.productId ?? ''}`.toLowerCase();
  return text.includes('esp') || text.includes('303a') || portInfo.path?.startsWith('/dev/ttyACM');
}
