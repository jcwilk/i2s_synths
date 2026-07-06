#!/usr/bin/env node
/**
 * Phase 3 sustained soak harness — bridge relay path with structured attestation.
 *
 * Usage:
 *   node sim/hardware-bridge/phase3-soak.mjs --scenario S2 [--duration 600] [--bridge ws://localhost:8765]
 *
 * Scenarios (design matrix):
 *   S1  GW + 7 WASM + HW end (passthrough) — bridge relay proxy
 *   S2  GW + WASM + HW delay + WASM
 *   S3  GW + HW merger + 2 WASM
 *   S4  GW + WASM + HW cutoff + WASM merger (loopback)
 *   S5  GW + WASM + HW debug_tone + WASM
 *   S6  GW + max interleave, HW middle (delay)
 *
 * Full mixed-chain PWA audition: configure chain in host UI per scenario, run audio 10+ min,
 * compare drop counters from hardwareAdapter.soakSummary() in browser console.
 */
import os from 'node:os';
import { WebSocket } from 'ws';
import {
  BUFFER_LEN,
  PERIOD_MS,
  buildExchangeRequest,
  parseExchangeResponse,
} from './frame-protocol.js';

const SCENARIOS = {
  S1: { moduleKind: 'passthrough', chainLayout: 'GW + 7 WASM + HW end (passthrough)', mic: true },
  S2: { moduleKind: 'delay', chainLayout: 'GW + WASM + HW delay + WASM', mic: true },
  S3: { moduleKind: 'merger', chainLayout: 'GW + HW merger + 2 WASM', mic: true },
  S4: { moduleKind: 'cutoff', chainLayout: 'GW + WASM + HW cutoff + WASM merger (loopback)', mic: true },
  S5: { moduleKind: 'debug_tone', chainLayout: 'GW + WASM + HW debug_tone + WASM', mic: false },
  S6: { moduleKind: 'delay', chainLayout: 'GW + max WASM/HW interleave, HW middle (delay)', mic: true },
};

function parseArgs(argv) {
  const scenarioIdx = argv.indexOf('--scenario');
  const durationIdx = argv.indexOf('--duration');
  const bridgeIdx = argv.indexOf('--bridge');
  return {
    scenario: scenarioIdx >= 0 ? argv[scenarioIdx + 1] : 'S2',
    durationS: durationIdx >= 0 ? Number(argv[durationIdx + 1]) : 600,
    bridgeUrl: bridgeIdx >= 0 ? argv[bridgeIdx + 1] : process.env.BRIDGE_URL ?? 'ws://localhost:8765',
  };
}

function waitForJson(ws, predicate, timeoutMs = 15000) {
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

async function runSoak({ scenario, durationS, bridgeUrl }) {
  const spec = SCENARIOS[scenario];
  if (!spec) {
    throw new Error(`Unknown scenario ${scenario}`);
  }

  const downstreamIn = new Int16Array(BUFFER_LEN);
  const upstreamIn = new Int16Array(BUFFER_LEN);
  for (let i = 0; i < BUFFER_LEN; i++) {
    downstreamIn[i] = Math.sin(i / 8) * 6000;
    upstreamIn[i] = 0;
  }

  const ws = new WebSocket(bridgeUrl);
  ws.binaryType = 'arraybuffer';
  ws.setMaxListeners(32);
  /** @type {Map<number, { resolve: Function, reject: Function, timer: ReturnType<typeof setTimeout> }>} */
  const pending = new Map();

  ws.on('message', (data) => {
    if (typeof data === 'string') {
      return;
    }
    const buf = Buffer.from(data);
    if (buf.length < 8) {
      return;
    }
    const inner = buf.subarray(4);
    try {
      const parsed = parseExchangeResponse(inner);
      const entry = pending.get(parsed.sequence);
      if (!entry) {
        return;
      }
      clearTimeout(entry.timer);
      pending.delete(parsed.sequence);
      entry.resolve(parsed);
    } catch (_) {}
  });

  function relayOne(requestBytes, sequence) {
    return new Promise((resolve, reject) => {
      const timer = setTimeout(() => {
        pending.delete(sequence);
        reject(new Error('Exchange timeout'));
      }, 20000);
      pending.set(sequence, { resolve, reject, timer });
      ws.send(requestBytes);
    });
  }

  await new Promise((resolve, reject) => {
    ws.on('open', resolve);
    ws.on('error', reject);
  });

  const started = Date.now();
  let exchanges = 0;
  let statusErrors = 0;
  let seqGaps = 0;
  let sustainedUnderrunProxy = 0;
  let firmwareKind = null;
  let windowUnderruns = 0;
  let windowPeriods = 0;
  let windowStart = started;

  try {
    ws.send(JSON.stringify({ type: 'ping' }));
    await waitForJson(ws, (m) => m.type === 'status' || m.type === 'pong', 30000);
    ws.send(JSON.stringify({ type: 'session', action: 'start', mode: 'pwa' }));
    const sessionMsg = await waitForJson(ws, (m) => m.action === 'started', 45000);
    firmwareKind = sessionMsg.firmwareModuleKind;

    const deadline = started + durationS * 1000;
    let sequence = 0;
    let inFlight = [];

    while (Date.now() < deadline) {
      while (inFlight.length < 2) {
        const seq = sequence++;
        const req = buildExchangeRequest({ sequence: seq, downstreamIn, upstreamIn });
        const p = relayOne(req, seq).then((parsed) => {
          exchanges += 1;
          if ((parsed.status & 0x0040) !== 0) {
            seqGaps += 1;
          }
          if ((parsed.status & 0x0001) === 0) {
            statusErrors += 1;
          }
          windowPeriods += 1;
          if ((parsed.status & 0x0020) !== 0) {
            windowUnderruns += 1;
          }
          const now = Date.now();
          if (now - windowStart >= 30000) {
            if (windowPeriods > 0 && windowUnderruns / windowPeriods > 0.05) {
              sustainedUnderrunProxy += 1;
            }
            windowStart = now;
            windowUnderruns = 0;
            windowPeriods = 0;
          }
        }).catch(() => {
          statusErrors += 1;
        }).finally(() => {
          inFlight = inFlight.filter((x) => x !== p);
        });
        inFlight.push(p);
        if (Date.now() >= deadline) {
          break;
        }
      }
      await Promise.race(inFlight.length ? inFlight : [new Promise((r) => setTimeout(r, PERIOD_MS))]);
      await new Promise((r) => setTimeout(r, Math.max(0, PERIOD_MS - 2)));
    }

    await Promise.allSettled(inFlight);
    ws.send(JSON.stringify({ type: 'session', action: 'stop' }));
  } finally {
    ws.close();
  }

  const elapsedS = (Date.now() - started) / 1000;
  const moduleMatch = !firmwareKind || firmwareKind === spec.moduleKind;
  const transientRate = exchanges > 0 ? statusErrors / exchanges : 0;
  const pass = seqGaps === 0
    && sustainedUnderrunProxy === 0
    && transientRate <= 0.0005
    && elapsedS >= durationS * 0.98
    && exchanges >= Math.floor(durationS * (1000 / PERIOD_MS) * 0.5)
    && moduleMatch;

  return {
    id: scenario,
    platform: `${os.type()} ${os.release()} (${os.arch()})`,
    bridgeUrl,
    chainLayout: spec.chainLayout,
    expectedModuleKind: spec.moduleKind,
    firmwareModuleKind: firmwareKind,
    mic: spec.mic,
    durationS: elapsedS,
    targetDurationS: durationS,
    exchanges,
    dropCount: statusErrors + seqGaps,
    transientErrorRate: transientRate,
    statusErrors,
    seqGaps,
    sustainedUnderrunWindows: sustainedUnderrunProxy,
    latencyNotes: `Bridge pipelined relay @ ${PERIOD_MS.toFixed(2)} ms period; PWA mixed-chain adds ring+inter-unit delays per published budget`,
    pass,
    pwaNote: 'Confirm full topology in host UI; this harness validates sustained bridge+USB duplex for the scenario module kind.',
  };
}

async function main() {
  const opts = parseArgs(process.argv.slice(2));
  const all = process.argv.includes('--all');

  if (all) {
    const rows = [];
    for (const id of Object.keys(SCENARIOS)) {
      console.error(`==> Soak ${id} (${opts.durationS}s)`);
      rows.push(await runSoak({ ...opts, scenario: id }));
    }
    console.log(JSON.stringify({ matrix: rows, pass: rows.every((r) => r.pass) }, null, 2));
    process.exit(rows.every((r) => r.pass) ? 0 : 1);
    return;
  }

  const row = await runSoak(opts);
  console.log(JSON.stringify(row, null, 2));
  console.log(row.pass ? 'OVERALL: PASS' : 'OVERALL: FAIL');
  process.exit(row.pass ? 0 : 1);
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
