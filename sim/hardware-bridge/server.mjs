#!/usr/bin/env node
/**
 * Local bridge server: WebSocket ↔ USB CDC relay for PWA hardware slot.
 * Default: ws://localhost:8765
 */
import { createServer } from 'node:http';
import { WebSocketServer } from 'ws';
import {
  firmwareKindToName,
  BRIDGE_WIRE_PREFIX_SIZE,
} from './frame-protocol.js';
import {
  openAudioPort,
  closeAudioPort,
  enterUsbNeighborMode,
  exitUsbNeighborMode,
  queryModuleKind,
  relayExchange,
  listSerialPorts,
  isLikelyEsp32,
} from './usb-relay.js';

const DEFAULT_PORT = 8765;
const args = process.argv.slice(2);
const verbose = args.includes('--verbose');
const portArg = args.find((a) => a.startsWith('--port='));
const portPathArg = args.find((a) => a.startsWith('--device='));
const PORT = portArg ? Number(portArg.split('=')[1]) : DEFAULT_PORT;
const DEVICE_PATH = portPathArg ? portPathArg.split('=')[1] : process.env.FIRMWARE_PORT ?? null;

/** @type {import('ws').WebSocket | null} */
let activeClient = null;
/** @type {import('serialport').SerialPort | null} */
let usbPort = null;
let sessionActive = false;
let devicePath = DEVICE_PATH;

function log(...parts) {
  if (verbose) {
    console.error('[bridge]', ...parts);
  }
}

function statusPayload() {
  return {
    type: 'status',
    bridgeReachable: true,
    deviceAttached: !!(usbPort && usbPort.isOpen),
    devicePath: devicePath ?? null,
    sessionActive,
    moduleKind: sessionActive ? lastModuleKind : null,
  };
}

let lastModuleKind = null;

async function resolveDevicePath() {
  if (devicePath) {
    return devicePath;
  }
  const ports = await listSerialPorts();
  const candidates = ports.filter(isLikelyEsp32);
  if (candidates.length === 1) {
    devicePath = candidates[0].path;
    return devicePath;
  }
  if (candidates.length > 1) {
    throw new Error(`Multiple serial ports found: ${candidates.map((p) => p.path).join(', ')} — set FIRMWARE_PORT or --device=`);
  }
  throw new Error('No USB device found — connect ESP32 and set FIRMWARE_PORT');
}

async function attachUsb() {
  if (usbPort && usbPort.isOpen) {
    return;
  }
  const path = await resolveDevicePath();
  usbPort = await openAudioPort(path);
  devicePath = path;
  log('USB attached', path);
}

async function detachUsb() {
  if (usbPort) {
    await closeAudioPort(usbPort);
    usbPort = null;
  }
}

async function startSession(ws, mode = 'pwa') {
  if (sessionActive) {
    throw new Error('Session already active');
  }
  await attachUsb();
  const enterMode = mode === 'pwa' ? 1 : 0;
  await enterUsbNeighborMode(usbPort, enterMode);
  let kindId = null;
  let lastModuleKind = null;
  try {
    kindId = await queryModuleKind(usbPort);
    lastModuleKind = firmwareKindToName(kindId);
  } catch (err) {
    log('module query failed', err.message);
  }
  sessionActive = true;
  ws.send(JSON.stringify({
    ...statusPayload(),
    type: 'session',
    action: 'started',
    firmwareModuleKind: lastModuleKind,
    firmwareModuleKindId: kindId,
  }));
  broadcastStatus(ws);
}

async function stopSession(ws) {
  if (!sessionActive) {
    return;
  }
  try {
    if (usbPort && usbPort.isOpen) {
      await exitUsbNeighborMode(usbPort);
    }
  } catch (err) {
    log('exit error', err.message);
  }
  sessionActive = false;
  lastModuleKind = null;
  ws.send(JSON.stringify({ type: 'session', action: 'stopped' }));
  broadcastStatus(ws);
}

function broadcastStatus(ws) {
  ws.send(JSON.stringify(statusPayload()));
}

async function handleControlMessage(ws, msg) {
  if (msg.type === 'ping') {
    ws.send(JSON.stringify({ type: 'pong' }));
    broadcastStatus(ws);
    return;
  }

  if (msg.type === 'session' && msg.action === 'start') {
    if (activeClient !== ws) {
      ws.send(JSON.stringify({ type: 'error', message: 'Not the active client' }));
      return;
    }
    try {
      await startSession(ws, msg.mode ?? 'pwa');
    } catch (err) {
      ws.send(JSON.stringify({ type: 'error', message: err.message }));
      broadcastStatus(ws);
    }
    return;
  }

  if (msg.type === 'session' && msg.action === 'stop') {
    await stopSession(ws);
    return;
  }

  if (msg.type === 'enumerate') {
    const ports = await listSerialPorts();
    ws.send(JSON.stringify({ type: 'enumerate', ports }));
    return;
  }

  ws.send(JSON.stringify({ type: 'error', message: `Unknown control message: ${msg.type}` }));
}

const httpServer = createServer((_req, res) => {
  res.writeHead(200, { 'Content-Type': 'text/plain' });
  res.end('hardware-bridge WebSocket server — connect via ws://\n');
});

const wss = new WebSocketServer({ server: httpServer });

wss.on('connection', (ws) => {
  if (activeClient) {
    ws.send(JSON.stringify({ type: 'error', message: 'Bridge busy — single client only' }));
    ws.close(1013, 'single client');
    return;
  }

  activeClient = ws;
  log('client connected');
  broadcastStatus(ws);

  ws.on('message', async (data, isBinary) => {
    try {
      if (isBinary) {
        if (!sessionActive || !usbPort) {
          ws.send(JSON.stringify({ type: 'error', message: 'No active session for binary relay' }));
          return;
        }
        const request = Buffer.from(data);
        if (request.length < BRIDGE_WIRE_PREFIX_SIZE) {
          ws.send(JSON.stringify({ type: 'error', message: 'Binary frame too short' }));
          return;
        }
        const response = await relayExchange(usbPort, request);
        const wire = Buffer.alloc(BRIDGE_WIRE_PREFIX_SIZE + response.length);
        wire.writeUInt32LE(response.length, 0);
        response.copy(wire, BRIDGE_WIRE_PREFIX_SIZE);
        ws.send(wire, { binary: true });
        return;
      }

      const msg = JSON.parse(data.toString());
      await handleControlMessage(ws, msg);
    } catch (err) {
      ws.send(JSON.stringify({ type: 'error', message: err.message }));
      log('message error', err.message);
    }
  });

  ws.on('close', async () => {
    log('client disconnected');
    if (activeClient === ws) {
      activeClient = null;
    }
    if (sessionActive) {
      await stopSession(ws);
    }
  });
});

httpServer.listen(PORT, async () => {
  console.log(`hardware-bridge listening on ws://localhost:${PORT}`);
  if (DEVICE_PATH) {
    try {
      await attachUsb();
      console.log(`USB device pre-attached: ${devicePath}`);
    } catch (err) {
      console.warn(`USB pre-attach failed: ${err.message}`);
    }
  } else {
    console.log('USB device will attach on first session (set FIRMWARE_PORT or --device= to pre-bind)');
  }
});

process.on('SIGINT', async () => {
  if (sessionActive && usbPort) {
    await exitUsbNeighborMode(usbPort).catch(() => {});
  }
  await detachUsb();
  process.exit(0);
});
