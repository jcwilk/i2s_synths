#!/usr/bin/env node
/**
 * Phase 1 sustained USB realtime acceptance harness.
 */
import os from 'node:os';
import {
  ACCEPTANCE_DURATION_S,
  MIN_EXCHANGES,
  LATENCY_P50_MAX_MS,
  LATENCY_P99_MAX_MS,
  PERIOD_MS,
  RESPONSE_DEADLINE_MS,
  arraysEqual,
  deterministicPattern,
  percentile,
} from './frame-protocol.js';
import {
  closeAudioPort,
  enterUsbNeighborMode,
  exitUsbNeighborMode,
  exchangeUsbPeriod,
  openAudioPort,
  runUsbLoopbackSelfTest,
} from './usb-transport.js';

function parseArgs(argv) {
  const command = argv[0];
  const audioIndex = argv.indexOf('--audio-port');
  const audioPort = audioIndex >= 0 ? argv[audioIndex + 1] : process.env.FIRMWARE_PORT;
  const durationIndex = argv.indexOf('--duration');
  const durationS = durationIndex >= 0 ? Number(argv[durationIndex + 1]) : ACCEPTANCE_DURATION_S;
  return { command, audioPort, durationS };
}

function usage() {
  console.log(`Usage:
  node sim/hardware-realtime/acceptance.mjs loopback --audio-port <path>
  node sim/hardware-realtime/acceptance.mjs sustained --audio-port <path> [--duration 60]`);
}

function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

async function cmdLoopback(portPath) {
  const port = await openAudioPort(portPath);
  try {
    await enterUsbNeighborMode(port);
    const periodSpec = {
      ...deterministicPattern(0),
      primary: 0.5,
      secondary: 0.5,
    };
    const response = await runUsbLoopbackSelfTest(port, periodSpec, 0);
    const pass = arraysEqual(response.downstreamOut, periodSpec.downstreamIn)
      && arraysEqual(response.upstreamOut, periodSpec.upstreamIn);
    console.log(pass ? 'PASS loopback: transport echoed inputs' : 'FAIL loopback: echo mismatch');
    await exitUsbNeighborMode(port);
    process.exit(pass ? 0 : 1);
  } finally {
    await closeAudioPort(port);
  }
}

async function cmdSustained(portPath, durationS) {
  const port = await openAudioPort(portPath);
  const latencies = [];
  let exchanges = 0;
  let drops = 0;
  let identityFailures = 0;
  let firstViolation = null;
  const endAt = performance.now() + durationS * 1000;
  let nextSendAt = performance.now();

  try {
    await enterUsbNeighborMode(port);

    while (performance.now() < endAt) {
      const now = performance.now();
      if (now < nextSendAt) {
        await sleep(nextSendAt - now);
      }
      nextSendAt += PERIOD_MS;

      const periodSpec = {
        ...deterministicPattern(exchanges),
        primary: 0.5,
        secondary: 0.5,
      };

      try {
        const response = await exchangeUsbPeriod(port, periodSpec, exchanges);
        if (response.roundTripMs > RESPONSE_DEADLINE_MS) {
          drops += 1;
          if (!firstViolation) {
            firstViolation = `response deadline exceeded (${response.roundTripMs.toFixed(2)} ms)`;
          }
        } else {
          latencies.push(response.roundTripMs);
        }
        if (!arraysEqual(response.downstreamOut, periodSpec.downstreamIn)
          || !arraysEqual(response.upstreamOut, periodSpec.upstreamIn)) {
          identityFailures += 1;
          if (!firstViolation) {
            firstViolation = `identity mismatch at sequence ${exchanges}`;
          }
        }
        exchanges += 1;
      } catch (error) {
        drops += 1;
        if (!firstViolation) {
          firstViolation = error.message;
        }
        break;
      }
    }

    await exitUsbNeighborMode(port);
  } finally {
    await closeAudioPort(port);
  }

  latencies.sort((a, b) => a - b);
  const p50 = percentile(latencies, 50);
  const p99 = percentile(latencies, 99);
  const pass = drops === 0
    && identityFailures === 0
    && exchanges >= MIN_EXCHANGES
    && p50 <= LATENCY_P50_MAX_MS
    && p99 <= LATENCY_P99_MAX_MS;

  const summary = {
    pass,
    hostOs: `${os.type()} ${os.release()} (${os.arch()})`,
    audioPort: portPath,
    durationS,
    exchanges,
    drops,
    identityFailures,
    latencyMs: { p50, p99, min: latencies[0] ?? 0, max: latencies[latencies.length - 1] ?? 0 },
    thresholds: {
      minExchanges: MIN_EXCHANGES,
      p50MaxMs: LATENCY_P50_MAX_MS,
      p99MaxMs: LATENCY_P99_MAX_MS,
      periodMs: PERIOD_MS,
    },
    firstViolation,
  };

  console.log(JSON.stringify(summary, null, 2));
  console.log(pass ? 'OVERALL: PASS' : `OVERALL: FAIL (${firstViolation ?? 'threshold violated'})`);
  process.exit(pass ? 0 : 1);
}

async function main() {
  const { command, audioPort, durationS } = parseArgs(process.argv.slice(2));
  if (!command || !audioPort) {
    usage();
    process.exit(1);
  }

  if (command === 'loopback') {
    await cmdLoopback(audioPort);
    return;
  }

  if (command === 'sustained') {
    await cmdSustained(audioPort, durationS);
    return;
  }

  usage();
  process.exit(1);
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
