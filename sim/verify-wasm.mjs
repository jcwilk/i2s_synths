#!/usr/bin/env node
/**
 * Headless WASM smoke checks for module-chain simulator builds.
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const OUT = path.join(__dirname, 'wasm', 'out');
const SAMPLE_RATE = 44100;
const BUFFER_LEN = 512;

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

async function main() {
  for (const name of VARIANTS) {
    await smokeProcess(name);
  }
  await verifyPassthrough();
  await verifyDelay();
  await verifyDebugTone();
  console.log('All WASM verification checks passed.');
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
