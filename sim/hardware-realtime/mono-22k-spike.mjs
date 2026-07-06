#!/usr/bin/env node
/**
 * Spike: 44.1 kHz stereo int16 (512/path) vs 22.05 kHz mono int16 (128/path).
 * Both target ~5.8 ms chain period; mono+half-rate should cut wire bytes ~4×.
 */
import os from 'node:os';
import {
  PERIOD_MS,
  SPIKE_MONO_PERIOD_MS,
  deterministicPattern,
  deterministicMono22kPattern,
  arraysEqual,
  percentile,
  SPIKE_MONO_REQUEST_SIZE,
  BRIDGE_EXCHANGE_REQUEST_SIZE,
} from './frame-protocol.js';
import {
  closeAudioPort,
  enterUsbNeighborMode,
  exitUsbNeighborMode,
  exchangeUsbPeriod,
  exchangeUsb22kMono,
  openAudioPort,
} from './usb-transport.js';

const DURATION_S = Number(process.env.SPIKE_DURATION_S ?? 30);
const portPath = process.argv[2] ?? process.env.FIRMWARE_PORT;

async function runStereo44k(port, durationS) {
  const latencies = [];
  let periods = 0;
  let errors = 0;
  let identityFailures = 0;
  const endAt = performance.now() + durationS * 1000;

  while (performance.now() < endAt) {
    try {
      const periodSpec = { ...deterministicPattern(periods), primary: 0.5, secondary: 0.5 };
      const response = await exchangeUsbPeriod(port, periodSpec, periods);
      latencies.push(response.roundTripMs);
      if (!arraysEqual(response.downstreamOut, periodSpec.downstreamIn)
        || !arraysEqual(response.upstreamOut, periodSpec.upstreamIn)) {
        identityFailures += 1;
      }
      periods += 1;
    } catch (error) {
      errors += 1;
      if (errors === 1) console.error('stereo44k:', error.message);
      if (errors >= 3) break;
    }
  }

  return summarize('stereo44k', durationS, periods, PERIOD_MS, latencies, errors, identityFailures, BRIDGE_EXCHANGE_REQUEST_SIZE);
}

async function runMono22k(port, durationS) {
  const latencies = [];
  let periods = 0;
  let errors = 0;
  let identityFailures = 0;
  const endAt = performance.now() + durationS * 1000;

  while (performance.now() < endAt) {
    try {
      const periodSpec = { ...deterministicMono22kPattern(periods), primary: 0.5, secondary: 0.5 };
      const response = await exchangeUsb22kMono(port, periodSpec, periods);
      latencies.push(response.roundTripMs);
      if (!arraysEqual(response.downstreamOut, periodSpec.downstreamIn)
        || !arraysEqual(response.upstreamOut, periodSpec.upstreamIn)) {
        identityFailures += 1;
      }
      periods += 1;
    } catch (error) {
      errors += 1;
      if (errors === 1) console.error('mono22k:', error.message);
      if (errors >= 3) break;
    }
  }

  return summarize('mono22k', durationS, periods, SPIKE_MONO_PERIOD_MS, latencies, errors, identityFailures, SPIKE_MONO_REQUEST_SIZE);
}

function summarize(mode, durationS, periods, periodMs, latencies, errors, identityFailures, innerRequestBytes) {
  latencies.sort((a, b) => a - b);
  const audioSeconds = (periods * periodMs) / 1000;
  const p50 = percentile(latencies, 50);
  return {
    mode,
    sampleRate: mode === 'mono22k' ? 22050 : 44100,
    channels: mode === 'mono22k' ? 1 : 2,
    periodMs,
    innerRequestBytes,
    durationS,
    periods,
    roundTrips: periods,
    audioSeconds,
    realTimeRatio: audioSeconds / durationS,
    rttEfficiency: p50 > 0 ? periodMs / p50 : 0,
    errors,
    identityFailures,
    latencyMs: {
      p50,
      p99: percentile(latencies, 99),
      min: latencies[0] ?? 0,
      max: latencies[latencies.length - 1] ?? 0,
      mean: latencies.length ? latencies.reduce((a, b) => a + b, 0) / latencies.length : 0,
    },
  };
}

async function main() {
  if (!portPath) {
    console.error('Usage: node mono-22k-spike.mjs <serial-port>');
    process.exit(1);
  }

  const port = await openAudioPort(portPath);
  try {
    console.log(`22.05kHz mono spike on ${portPath} (${DURATION_S}s/mode), ${os.type()} ${os.release()}`);
    console.log(`Wire inner request bytes: stereo=${BRIDGE_EXCHANGE_REQUEST_SIZE} mono22k=${SPIKE_MONO_REQUEST_SIZE}`);

    await enterUsbNeighborMode(port);
    const stereo = await runStereo44k(port, DURATION_S);
    await exitUsbNeighborMode(port);
    await enterUsbNeighborMode(port);
    const mono = await runMono22k(port, DURATION_S);
    await exitUsbNeighborMode(port);

    const report = {
      stereo44k: stereo,
      mono22k: mono,
      comparison: {
        wireBytesRatio: BRIDGE_EXCHANGE_REQUEST_SIZE / SPIKE_MONO_REQUEST_SIZE,
        rttRatio: mono.latencyMs.p50 / stereo.latencyMs.p50,
        realTimeGain: mono.realTimeRatio / stereo.realTimeRatio,
        periodsGain: mono.periods / stereo.periods,
        monoMeetsRealTime: mono.realTimeRatio >= 1.0,
      },
    };

    console.log(JSON.stringify(report, null, 2));
    console.log(`\nStereo 44.1k: ${stereo.realTimeRatio.toFixed(2)}× realtime, p50 ${stereo.latencyMs.p50.toFixed(1)} ms, ${stereo.periods} periods`);
    console.log(`Mono 22.05k: ${mono.realTimeRatio.toFixed(2)}× realtime, p50 ${mono.latencyMs.p50.toFixed(1)} ms, ${mono.periods} periods`);
    console.log(`Gain: ${report.comparison.realTimeGain.toFixed(2)}× realtime throughput`);
    process.exit(mono.identityFailures === 0 && mono.errors === 0 && stereo.errors === 0 ? 0 : 1);
  } finally {
    await closeAudioPort(port);
  }
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
