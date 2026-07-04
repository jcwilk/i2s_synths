#!/usr/bin/env node
/** Generate deterministic offline test vectors and WASM golden references. */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { BUFFER_LEN } from './frame-protocol.js';
import { runReferenceSequence, referenceToJson } from './reference-runner.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const VECTORS_DIR = path.join(__dirname, 'vectors');

function stereoRamp(phase, amplitude = 12000) {
  const samples = new Array(BUFFER_LEN);
  for (let i = 0; i < BUFFER_LEN; i += 2) {
    const frame = (phase * BUFFER_LEN + i) / 2;
    const value = Math.round((frame % 128) / 127 * amplitude);
    samples[i] = value;
    samples[i + 1] = -value;
  }
  return samples;
}

function stereoImpulse(position = 0, amplitude = 20000) {
  const samples = new Array(BUFFER_LEN).fill(0);
  if (position >= 0 && position < BUFFER_LEN) {
    samples[position] = amplitude;
    if (position + 1 < BUFFER_LEN) {
      samples[position + 1] = amplitude;
    }
  }
  return samples;
}

function stereoSine(phase, amplitude = 14000) {
  const samples = new Array(BUFFER_LEN);
  for (let i = 0; i < BUFFER_LEN; i += 2) {
    const t = (phase * BUFFER_LEN + i) / BUFFER_LEN;
    const value = Math.round(Math.sin(t * Math.PI * 2) * amplitude);
    samples[i] = value;
    samples[i + 1] = value;
  }
  return samples;
}

function buildPassthroughVector() {
  const periods = [];
  for (let period = 0; period < 6; period++) {
    periods.push({
      primary: 0.5,
      secondary: 0.25,
      downstreamIn: stereoRamp(period, 10000 + period * 500),
      upstreamIn: stereoSine(period, 8000),
    });
  }
  return {
    moduleKind: 'passthrough',
    description: 'Deterministic non-silent passthrough offline vector',
    warmupPeriods: 0,
    compareFromPeriod: 0,
    defaultPrimary: 0.5,
    defaultSecondary: 0.25,
    periods,
  };
}

function buildDelayVector() {
  const warmupPeriods = 22;
  const comparePeriods = 18;
  const periods = [];

  for (let period = 0; period < warmupPeriods; period++) {
    periods.push({
      primary: 0.35,
      secondary: 0.0,
      downstreamIn: stereoSine(period, period < 4 ? 0 : 6000),
      upstreamIn: new Array(BUFFER_LEN).fill(0),
    });
  }

  for (let period = 0; period < comparePeriods; period++) {
    const absolutePeriod = warmupPeriods + period;
    periods.push({
      primary: period < 6 ? 0.35 : 0.55,
      secondary: 0.0,
      downstreamIn: period === 2 || period === 8
        ? stereoImpulse(0, 22000)
        : stereoRamp(absolutePeriod, 5000),
      upstreamIn: stereoSine(absolutePeriod, 3000),
    });
  }

  return {
    moduleKind: 'delay',
    description: 'Delay offline vector with warm-up fill and length control step',
    warmupPeriods,
    compareFromPeriod: warmupPeriods,
    defaultPrimary: 0.35,
    defaultSecondary: 0.0,
    periods,
  };
}

async function writeVectorBundle(name, vector) {
  const vectorPath = path.join(VECTORS_DIR, `${name}.json`);
  fs.mkdirSync(VECTORS_DIR, { recursive: true });
  fs.writeFileSync(vectorPath, `${JSON.stringify(vector, null, 2)}\n`);

  const reference = await runReferenceSequence(vector.moduleKind, vector);
  const goldenPath = path.join(VECTORS_DIR, `${name}.golden.json`);
  fs.writeFileSync(
    goldenPath,
    `${JSON.stringify({ moduleKind: vector.moduleKind, reference: referenceToJson(reference) }, null, 2)}\n`,
  );

  console.log(`Wrote ${vectorPath}`);
  console.log(`Wrote ${goldenPath}`);
}

async function main() {
  await writeVectorBundle('passthrough', buildPassthroughVector());
  await writeVectorBundle('delay', buildDelayVector());
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
