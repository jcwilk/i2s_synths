#!/usr/bin/env node
/**
 * Phase 2 bridge integration smoke test (headless).
 */
import { WebSocket } from 'ws';
import {
  BUFFER_LEN,
  BRIDGE_MAGIC,
  BRIDGE_WIRE_PREFIX_SIZE,
  PERIOD_MS,
  buildExchangeRequest,
  parseExchangeResponse,
} from './frame-protocol.js';

const url = process.env.BRIDGE_URL ?? 'ws://localhost:8765';
const durationS = Number(process.env.ACCEPTANCE_DURATION_S ?? 30);

function waitForJson(ws, predicate, timeoutMs = 10000) {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error('JSON wait timeout')), timeoutMs);
    const handler = (data) => {
      const text = typeof data === 'string' ? data : data.toString();
      try {
        const msg = JSON.parse(text);
        if (predicate(msg)) {
          clearTimeout(timer);
          ws.off('message', handler);
          resolve(msg);
        }
      } catch (_) {}
    };
    ws.on('message', handler);
  });
}

function relayOne(ws, requestBytes) {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error('Exchange timeout')), 15000);
    const handler = (data) => {
      const buf = Buffer.isBuffer(data) ? data : Buffer.from(data);
      if (buf.length < BRIDGE_WIRE_PREFIX_SIZE + 6) {
        return;
      }
      const wireLen = buf.readUInt32LE(0);
      const inner = buf.subarray(BRIDGE_WIRE_PREFIX_SIZE, BRIDGE_WIRE_PREFIX_SIZE + wireLen);
      if (inner.readUInt32LE(0) !== BRIDGE_MAGIC) {
        return;
      }
      clearTimeout(timer);
      ws.off('message', handler);
      resolve(parseExchangeResponse(inner));
    };
    ws.on('message', handler);
    ws.send(requestBytes);
  });
}

const downstreamIn = new Int16Array(BUFFER_LEN);
const upstreamIn = new Int16Array(BUFFER_LEN);
for (let i = 0; i < BUFFER_LEN; i++) {
  downstreamIn[i] = Math.sin(i / 8) * 8000;
  upstreamIn[i] = 0;
}

const ws = new WebSocket(url);
ws.binaryType = 'arraybuffer';

const started = Date.now();
let exchanges = 0;
let underruns = 0;
let firmwareKind = null;

ws.on('open', async () => {
  try {
    ws.send(JSON.stringify({ type: 'ping' }));
    await waitForJson(ws, (m) => m.type === 'status');
    ws.send(JSON.stringify({ type: 'session', action: 'start', mode: 'pwa' }));
    const sessionMsg = await waitForJson(ws, (m) => m.action === 'started');
    firmwareKind = sessionMsg.firmwareModuleKind;
    console.log(`SESSION_STARTED kind=${firmwareKind}`);

    const deadline = started + durationS * 1000;
    let sequence = 0;
    while (Date.now() < deadline) {
      const req = buildExchangeRequest({ sequence: sequence++, downstreamIn, upstreamIn });
      const parsed = await relayOne(ws, req);
      exchanges++;
      if ((parsed.status & 0x0001) === 0) {
        underruns++;
      }
      const elapsed = Date.now() - started;
      const waitMs = Math.max(0, PERIOD_MS - 1);
      if (elapsed + waitMs < deadline) {
        await new Promise((r) => setTimeout(r, waitMs));
      }
    }

    ws.send(JSON.stringify({ type: 'session', action: 'stop' }));
    ws.close();
  } catch (err) {
    console.error('FAIL', err.message);
    ws.close();
    process.exit(1);
  }
});

ws.on('close', () => {
  const elapsed = ((Date.now() - started) / 1000).toFixed(1);
  const pass = exchanges >= Math.floor(durationS * 100) && underruns === 0 && firmwareKind === 'delay';
  console.log(JSON.stringify({
    platform: process.platform,
    bridgeUrl: url,
    chainLayout: 'headless relay (gateway, WASM, HW delay, WASM — PWA manual for full chain)',
    firmwareModuleKind: firmwareKind,
    durationS: elapsed,
    exchanges,
    underruns,
    pass,
  }, null, 2));
  console.log(pass ? 'OVERALL: PASS' : 'OVERALL: FAIL');
  process.exit(pass ? 0 : 1);
});

ws.on('error', (err) => {
  console.error('WS error', err.message);
  process.exit(1);
});
