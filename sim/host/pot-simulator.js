/** Mirrors `src/input/pots.h` EMA timing for simulated knobs. */

export const EMA_ALPHA = 0.008;

const SAMPLE_RATE = 44100;
const BUFFER_LEN = 512;

/** Firmware polls pots once per main loop, roughly aligned with one I2S buffer period. */
export const POT_POLL_MS = (BUFFER_LEN / SAMPLE_RATE) * 1000;

export function potsEffectiveAlpha(deltaMs) {
  if (deltaMs <= 0) {
    return 0;
  }
  const powTerm = (1 - EMA_ALPHA) ** deltaMs;
  const alpha = 1 - powTerm;
  return Math.max(0, Math.min(1, alpha));
}

/**
 * One potsUpdate step for a single PotState in WASM linear memory (float indices).
 * @param {Float32Array} heapF32
 * @param {number} rawIndex
 * @param {number} smoothedIndex
 * @param {number} targetRaw01 slider position in [0, 1]
 * @param {number} deltaMs elapsed since last poll
 */
export function applyPotStep(heapF32, rawIndex, smoothedIndex, targetRaw01, deltaMs) {
  const alpha = potsEffectiveAlpha(deltaMs);
  const raw = Math.max(0, Math.min(1, targetRaw01));
  const prevSmoothed = heapF32[smoothedIndex];
  heapF32[rawIndex] = raw;
  heapF32[smoothedIndex] = alpha * raw + (1 - alpha) * prevSmoothed;
}
