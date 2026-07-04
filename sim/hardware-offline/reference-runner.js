import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { BUFFER_LEN } from './frame-protocol.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const WASM_OUT = path.join(__dirname, '..', 'wasm', 'out');

export async function loadWasmModule(moduleKind) {
  const jsPath = path.join(WASM_OUT, `${moduleKind}.js`);
  const wasmPath = path.join(WASM_OUT, `${moduleKind}.wasm`);
  if (!fs.existsSync(jsPath) || !fs.existsSync(wasmPath)) {
    throw new Error(`WASM artifacts missing for ${moduleKind}; run ./sim/build-wasm.sh`);
  }
  const wasmBytes = fs.readFileSync(wasmPath);
  const mod = await import(pathToFileURL(jsPath).href);
  return mod.default({
    instantiateWasm: (imports, receiveInstance) => {
      WebAssembly.instantiate(wasmBytes, imports).then((result) => receiveInstance(result.instance));
      return {};
    },
  });
}

function writeInt16Buffer(heap16, ptr, samples) {
  const base = ptr >> 1;
  for (let i = 0; i < BUFFER_LEN; i++) {
    heap16[base + i] = samples[i] ?? 0;
  }
}

function readInt16Buffer(heap16, ptr) {
  const base = ptr >> 1;
  const out = new Int16Array(BUFFER_LEN);
  for (let i = 0; i < BUFFER_LEN; i++) {
    out[i] = heap16[base + i];
  }
  return out;
}

function setInjectedPots(heapF32, potsPtr, primary, secondary) {
  const base = potsPtr >> 2;
  heapF32[base + 1] = primary;
  heapF32[base + 2] = primary;
  heapF32[base + 4] = secondary;
  heapF32[base + 5] = secondary;
}

export function int16ArrayFromJson(values) {
  return Int16Array.from(values);
}

export async function runReferenceSequence(moduleKind, vector) {
  const module = await loadWasmModule(moduleKind);
  module._sim_setup();

  const dsInPtr = module._sim_get_downstream_in();
  const usInPtr = module._sim_get_upstream_in();
  const dsOutPtr = module._sim_get_downstream_out();
  const usOutPtr = module._sim_get_upstream_out();
  const potsPtr = module._sim_get_pots();

  const reference = [];
  for (let period = 0; period < vector.periods.length; period++) {
    const periodSpec = vector.periods[period];
    const primary = periodSpec.primary ?? vector.defaultPrimary ?? 0;
    const secondary = periodSpec.secondary ?? vector.defaultSecondary ?? 0;
    setInjectedPots(module.HEAPF32, potsPtr, primary, secondary);

    const downstreamIn = int16ArrayFromJson(periodSpec.downstreamIn);
    const upstreamIn = int16ArrayFromJson(periodSpec.upstreamIn);
    writeInt16Buffer(module.HEAP16, dsInPtr, downstreamIn);
    writeInt16Buffer(module.HEAP16, usInPtr, upstreamIn);

    module._sim_process_upstream();
    module._sim_process_downstream();

    reference.push({
      downstreamOut: readInt16Buffer(module.HEAP16, dsOutPtr),
      upstreamOut: readInt16Buffer(module.HEAP16, usOutPtr),
    });
  }

  return reference;
}

export function referenceToJson(reference) {
  return reference.map((period) => ({
    downstreamOut: Array.from(period.downstreamOut),
    upstreamOut: Array.from(period.upstreamOut),
  }));
}
