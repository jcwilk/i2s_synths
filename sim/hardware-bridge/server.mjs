#!/usr/bin/env node
/**
 * Local bridge server: WebSocket ↔ USB CDC relay for PWA hardware slot.
 * Phase 3: pipelined USB relay (depth 2), relay timing metrics.
 * Default: ws://localhost:8765
 */
import { createServer } from 'node:http';
import { performance } from 'node:perf_hooks';
import { WebSocketServer } from 'ws';
import {
  firmwareKindToName,
  BRIDGE_WIRE_PREFIX_SIZE,
  PERIOD_MS,
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
const USB_PIPELINE_DEPTH = 2;
const RELAY_P99_BUDGET_MS = 2;
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
let lastModuleKind = null;

/** @type {{ ws: import('ws').WebSocket, request: Buffer }[]} */
const relayQueue = [];
let usbInFlight = 0;
const relayTimingsMs = [];

function log(...parts) {
  if (verbose) {
    console.error('[bridge]', ...parts);
  }
}

function recordRelayMs(ms) {
  relayTimingsMs.push(ms);
  while (relayTimingsMs.length > 500) {
    relayTimingsMs.shift();
  }
}

function relayP99Ms() {
  if (relayTimingsMs.length === 0) {
    return 0;
  }
  const sorted = [...relayTimingsMs].sort((a, b) => a - b);
  return sorted[Math.floor(sorted.length * 0.99)] ?? 0;
}

function statusPayload() {
  return {
    type: 'status',
    bridgeReachable: true,
    deviceAttached: !!(usbPort && usbPort.isOpen),
    devicePath: devicePath ?? null,
    sessionActive,
    moduleKind: sessionActive ? lastModuleKind : null,
    relayP99Ms: relayP99Ms(),
    relayBudgetMs: RELAY_P99_BUDGET_MS,
    usbPipelineDepth: USB_PIPELINE_DEPTH,
    bufferPeriodMs: PERIOD_MS,
  };
}

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
  relayQueue.length = 0;
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

async function drainRelayQueue() {
  if (!usbPort || !sessionActive) {
    return;
  }
  while (relayQueue.length > 0 && usbInFlight < USB_PIPELINE_DEPTH) {
    const job = relayQueue.shift();
    usbInFlight += 1;
    const t0 = performance.now();
    relayExchange(usbPort, job.request)
      .then((response) => {
        const relayMs = performance.now() - t0;
        recordRelayMs(relayMs);
        const wire = Buffer.alloc(BRIDGE_WIRE_PREFIX_SIZE + response.length);
        wire.writeUInt32LE(response.length, 0);
        response.copy(wire, BRIDGE_WIRE_PREFIX_SIZE);
        if (job.ws.readyState === job.ws.OPEN) {
          job.ws.send(wire, { binary: true });
        }
      })
      .catch((err) => {
        if (job.ws.readyState === job.ws.OPEN) {
          job.ws.send(JSON.stringify({ type: 'error', message: err.message }));
        }
        log('relay error', err.message);
      })
      .finally(() => {
        usbInFlight -= 1;
        drainRelayQueue();
      });
  }
}

function enqueueRelay(ws, request) {
  relayQueue.push({ ws, request });
  while (relayQueue.length > USB_PIPELINE_DEPTH * 2) {
    relayQueue.shift();
    log('relay queue drop (overrun)');
  }
  drainRelayQueue();
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
        enqueueRelay(ws, request);
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
    relayQueue.length = 0;
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
