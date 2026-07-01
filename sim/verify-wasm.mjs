#!/usr/bin/env node
/**
 * Headless WASM smoke checks for module-chain simulator builds.
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import {
  ChainScheduler,
  BUFFER_LEN,
  MERGER_STRESS_UNDERRUN,
  MERGER_STRESS_OVERRUN,
} from './host/chain-scheduler.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const OUT = path.join(__dirname, 'wasm', 'out');
const SAMPLE_RATE = 44100;

const VARIANTS = ['passthrough', 'delay', 'merger', 'debug_tone', 'cutoff'];

function instantiateFactory(jsName, wasmName) {
  const jsPath = path.join(OUT, jsName);
  const wasmBytes = fs.readFileSync(path.join(OUT, wasmName));
  return import(pathToFileURL(jsPath).href).then((mod) =>
    mod.default({
      instantiateWasm: (imports, receiveInstance) => {
        WebAssembly.instantiate(wasmBytes, imports).then((result) =>
          receiveInstance(result.instance),
        );
        return {};
      },
    }),
  );
}

function wasmBindings(module) {
  return {
    module,
    heap16: module.HEAP16,
    heapF32: module.HEAPF32,
    ptrs: {
      upstreamInPtr: module._sim_get_upstream_in(),
      upstreamOutPtr: module._sim_get_upstream_out(),
      downstreamInPtr: module._sim_get_downstream_in(),
      downstreamOutPtr: module._sim_get_downstream_out(),
    },
    potsPtr: module._sim_get_pots(),
    bufferLen: module._sim_get_buffer_len(),
  };
}

function writeImpulse(heap16, ptr) {
  const base = ptr >> 1;
  for (let i = 0; i < BUFFER_LEN; i++) {
    heap16[base + i] = i === 0 ? 20000 : 0;
  }
}

function rms(heap16, ptr) {
  const base = ptr >> 1;
  let sum = 0;
  for (let i = 0; i < BUFFER_LEN; i++) {
    const s = heap16[base + i];
    sum += s * s;
  }
  return Math.sqrt(sum / BUFFER_LEN);
}

function floatMicImpulse() {
  const mic = new Float32Array(BUFFER_LEN);
  mic[0] = 0.85;
  return mic;
}

async function smokeProcess(name) {
  const m = await instantiateFactory(`${name}.js`, `${name}.wasm`);
  m._sim_setup();
  const dsIn = m._sim_get_downstream_in();
  const dsOut = m._sim_get_downstream_out();
  const usIn = m._sim_get_upstream_in();
  const usOut = m._sim_get_upstream_out();

  writeImpulse(m.HEAP16, dsIn);
  m.HEAP16.fill(0, usIn >> 1, (usIn >> 1) + BUFFER_LEN);
  m._sim_process_upstream();
  m._sim_process_downstream();

  if (typeof m._sim_get_buffer_len !== 'function' || m._sim_get_buffer_len() !== BUFFER_LEN) {
    throw new Error(`${name}: unexpected buffer length`);
  }
  console.log(`PASS ${name}: setup and buffer processing callable`);
}

async function verifyPassthrough() {
  const m = await instantiateFactory('passthrough.js', 'passthrough.wasm');
  m._sim_setup();
  const dsIn = m._sim_get_downstream_in();
  const usIn = m._sim_get_upstream_in();
  const usOut = m._sim_get_upstream_out();
  const dsOut = m._sim_get_downstream_out();
  let prevDs = new Int16Array(BUFFER_LEN);

  for (let b = 0; b < 3; b++) {
    if (b === 1) {
      writeImpulse(m.HEAP16, dsIn);
    } else {
      m.HEAP16.fill(0, dsIn >> 1, (dsIn >> 1) + BUFFER_LEN);
    }
    for (let i = 0; i < BUFFER_LEN; i++) {
      m.HEAP16[(usIn >> 1) + i] = prevDs[i];
    }
    m._sim_process_upstream();
    m._sim_process_downstream();
    for (let i = 0; i < BUFFER_LEN; i++) {
      prevDs[i] = m.HEAP16[(dsOut >> 1) + i];
    }
  }

  const outRms = rms(m.HEAP16, usOut);
  if (outRms < 100) {
    throw new Error(`passthrough upstream output too quiet: rms=${outRms}`);
  }
  console.log('PASS passthrough: upstream output carries prior downstream energy');
}

async function verifyDelay() {
  const m = await instantiateFactory('delay.js', 'delay.wasm');
  m._sim_setup();
  const frames = m._sim_get_delay_buffer_frames();
  if (frames < SAMPLE_RATE / 10) {
    throw new Error(`delay ring too small: ${frames} frames`);
  }
  console.log(`PASS delay ring: ${frames} frames (${((frames * 1000) / SAMPLE_RATE).toFixed(1)} ms)`);

  const potsPtr = m._sim_get_pots();
  const potBase = potsPtr >> 2;
  m.HEAPF32[potBase + 2] = 0.0;
  m.HEAPF32[potBase + 5] = 0;

  const dsIn = m._sim_get_downstream_in();
  const dsOut = m._sim_get_downstream_out();
  const usIn = m._sim_get_upstream_in();
  const usOut = m._sim_get_upstream_out();
  let prevDs = new Int16Array(BUFFER_LEN);

  let heardDelayed = false;
  const buffersToRun = 40;

  for (let b = 0; b < buffersToRun; b++) {
    const impulse = b === 2;
    for (let i = 0; i < BUFFER_LEN; i++) {
      m.HEAP16[(dsIn >> 1) + i] = impulse && i === 0 ? 25000 : 0;
      m.HEAP16[(usIn >> 1) + i] = prevDs[i];
    }
    m._sim_process_upstream();
    m._sim_process_downstream();
    for (let i = 0; i < BUFFER_LEN; i++) {
      prevDs[i] = m.HEAP16[(dsOut >> 1) + i];
    }
    if (b > 4 && rms(m.HEAP16, usOut) > 100) {
      heardDelayed = true;
    }
  }

  if (!heardDelayed) {
    throw new Error('delay did not produce audible energy on upstream output');
  }
  console.log('PASS delay: impulse energy reaches upstream output after buffer periods');
}

async function verifyDebugTone() {
  const m = await instantiateFactory('debug_tone.js', 'debug_tone.wasm');
  m._sim_setup();
  const dsOut = m._sim_get_downstream_out();
  m.HEAP16.fill(0, m._sim_get_downstream_in() >> 1, (m._sim_get_downstream_in() >> 1) + BUFFER_LEN);
  m.HEAP16.fill(0, m._sim_get_upstream_in() >> 1, (m._sim_get_upstream_in() >> 1) + BUFFER_LEN);
  m._sim_process_upstream();
  m._sim_process_downstream();
  if (rms(m.HEAP16, dsOut) < 50) {
    throw new Error('debug_tone did not generate downstream tone energy');
  }
  console.log('PASS debug_tone: generates downstream tone energy');
}

/** Buffers to push upstream ring past capacity (64 frames in merger.h). */
const MERGER_RING_OVERFLOW_BUFFERS = 70;

async function verifyMergerStressExport() {
  const merger = await instantiateFactory('merger.js', 'merger.wasm');
  merger._sim_setup();

  if (typeof merger._sim_consume_merger_stress !== 'function') {
    throw new Error('merger WASM missing sim_consume_merger_stress export');
  }

  merger.HEAP16.fill(0, merger._sim_get_upstream_in() >> 1, (merger._sim_get_upstream_in() >> 1) + BUFFER_LEN);
  writeImpulse(merger.HEAP16, merger._sim_get_downstream_in());
  merger._sim_process_downstream();

  const underrunFlags = merger._sim_consume_merger_stress();
  if ((underrunFlags & MERGER_STRESS_UNDERRUN) === 0) {
    throw new Error('merger stress export: expected underrun when downstream runs with empty capture ring');
  }

  for (let i = 0; i < MERGER_RING_OVERFLOW_BUFFERS; i++) {
    writeImpulse(merger.HEAP16, merger._sim_get_upstream_in());
    merger._sim_process_upstream();
  }
  const overrunFlags = merger._sim_consume_merger_stress();
  if ((overrunFlags & MERGER_STRESS_OVERRUN) === 0) {
    throw new Error('merger stress export: expected overrun after sustained upstream capture');
  }
  console.log('PASS merger stress export: underrun and overrun flags observable from harness');
}

async function verifyMergerChainRouting() {
  const passthrough = await instantiateFactory('passthrough.js', 'passthrough.wasm');
  const merger = await instantiateFactory('merger.js', 'merger.wasm');
  passthrough._sim_setup();
  merger._sim_setup();

  const potBase = merger._sim_get_pots() >> 2;
  merger.HEAPF32[potBase + 1] = 0.05;
  merger.HEAPF32[potBase + 2] = 0.05;
  merger.HEAPF32[potBase + 4] = 1.0;
  merger.HEAPF32[potBase + 5] = 1.0;

  const scheduler = new ChainScheduler();
  scheduler.setSlots([
    { bindings: wasmBindings(passthrough) },
    { bindings: wasmBindings(merger) },
  ]);
  scheduler.setLoopbackEnabled(true);

  let heardDelayedBlend = false;
  let heardForwardPath = false;
  const silentMic = new Float32Array(BUFFER_LEN);

  for (let b = 0; b < 96; b++) {
    const mic = b === 12 ? floatMicImpulse() : silentMic;
    const { levels, playback } = scheduler.processBuffer(mic, b === 12);
    const mergerLevel = levels[1];
    if (b > 40 && mergerLevel?.out > 0.004) {
      heardDelayedBlend = true;
    }
    if (b > 50 && rmsFromFloatPlayback(playback) > 0.002) {
      heardForwardPath = true;
    }
  }

  if (!heardDelayedBlend) {
    throw new Error('merger chain: delayed upstream blend did not reach downstream output');
  }
  if (!heardForwardPath) {
    throw new Error('merger chain: forward-path energy did not reach gateway playback');
  }
  console.log('PASS merger chain: delayed blend and forward-path routing');
}

function rmsFromFloatPlayback(playback) {
  let sum = 0;
  for (let i = 0; i < playback.length; i++) {
    sum += playback[i] * playback[i];
  }
  return Math.sqrt(sum / playback.length) * 32767;
}

async function verifyDualPathDelay() {
  const passthroughA = await instantiateFactory('passthrough.js', 'passthrough.wasm');
  const passthroughB = await instantiateFactory('passthrough.js', 'passthrough.wasm');
  passthroughA._sim_setup();
  passthroughB._sim_setup();

  const scheduler = new ChainScheduler();
  scheduler.setSlots([
    { bindings: wasmBindings(passthroughA) },
    { bindings: wasmBindings(passthroughB) },
  ]);
  scheduler.setLoopbackEnabled(true);

  let slot0ImpulsePeriod = -1;
  let slot1ImpulsePeriod = -1;
  let slot0ReturnEnergyPeriod = -1;
  const silentMic = new Float32Array(BUFFER_LEN);

  for (let b = 0; b < 16; b++) {
    const mic = b === 2 ? floatMicImpulse() : silentMic;
    const { levels } = scheduler.processBuffer(mic, b === 2);
    if (levels[0]?.in > 0.5 && slot0ImpulsePeriod < 0) {
      slot0ImpulsePeriod = b;
    }
    if (levels[1]?.in > 0.5 && slot1ImpulsePeriod < 0) {
      slot1ImpulsePeriod = b;
    }
    if (levels[0]?.out > 0.5 && slot0ReturnEnergyPeriod < 0) {
      slot0ReturnEnergyPeriod = b;
    }
  }

  if (slot0ImpulsePeriod !== 2) {
    throw new Error(`dual-path delay: slot 0 expected impulse at period 2, got ${slot0ImpulsePeriod}`);
  }
  if (slot1ImpulsePeriod !== 3) {
    throw new Error(
      `dual-path delay: slot 1 expected delayed downstream impulse at period 3, got ${slot1ImpulsePeriod}`,
    );
  }
  if (slot0ReturnEnergyPeriod < 4) {
    throw new Error(
      `dual-path delay: slot 0 upstream return energy expected after loopback path delay, got period ${slot0ReturnEnergyPeriod}`,
    );
  }
  console.log('PASS dual-path delay: downstream and upstream feeds use separate one-buffer handoffs');
}

async function main() {
  for (const name of VARIANTS) {
    await smokeProcess(name);
  }
  await verifyPassthrough();
  await verifyDelay();
  await verifyDebugTone();
  await verifyDualPathDelay();
  await verifyMergerStressExport();
  await verifyMergerChainRouting();
  console.log('All WASM verification checks passed.');
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
