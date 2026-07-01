import { peakInt16Interleaved } from './level-graph.js';

export const SAMPLE_RATE = 44100;
export const BUFFER_LEN = 512;
export const MAX_PROCESSING_UNITS = 8;

/**
 * Routes firmware-sized buffers through gateway (index 0) plus N WASM processing slots.
 * Wiring matches ai/planning/web-module-chain-simulator.md Phase 1.
 */
export class ChainScheduler {
  constructor() {
    /** @type {{ bindings: object }[]} */
    this.slots = [];
    this.loopbackEnabled = false;
    this.gatewayDsIn = new Int16Array(BUFFER_LEN);
    this.gatewayDsOut = new Int16Array(BUFFER_LEN);
    this.gatewayUsIn = new Int16Array(BUFFER_LEN);
    this.gatewayUsOut = new Int16Array(BUFFER_LEN);
  }

  setSlots(slots) {
    this.slots = slots;
  }

  setLoopbackEnabled(enabled) {
    this.loopbackEnabled = !!enabled;
  }

  floatToInt16(sample) {
    const clamped = Math.max(-1, Math.min(1, sample));
    return clamped < 0 ? Math.round(clamped * 32768) : Math.round(clamped * 32767);
  }

  int16ToFloat(sample) {
    return sample / (sample < 0 ? 32768 : 32767);
  }

  copyBuffer(dst, src, heap16, srcPtr) {
    const base = srcPtr >> 1;
    for (let i = 0; i < BUFFER_LEN; i++) {
      dst[i] = heap16[base + i];
    }
  }

  writeBuffer(heap16, dstPtr, src) {
    const base = dstPtr >> 1;
    for (let i = 0; i < BUFFER_LEN; i++) {
      heap16[base + i] = src[i];
    }
  }

  clearBuffer(heap16, ptr) {
    heap16.fill(0, ptr >> 1, (ptr >> 1) + BUFFER_LEN);
  }

  /**
   * @param {Float32Array} micFloatSamples interleaved stereo float from Web Audio
   * @param {boolean} micEnabled
   * @returns {{ playback: Float32Array, levels: { in: number, out: number }[] }}
   */
  processBuffer(micFloatSamples, micEnabled) {
    const n = this.slots.length;
    const levels = [];

    for (let i = 0; i < BUFFER_LEN; i++) {
      this.gatewayDsIn[i] = micEnabled
        ? this.floatToInt16(micFloatSamples[i])
        : 0;
    }

    for (let i = 0; i < BUFFER_LEN; i++) {
      this.gatewayDsOut[i] = this.gatewayDsIn[i];
    }

    if (n === 0) {
      for (let i = 0; i < BUFFER_LEN; i++) {
        this.gatewayUsOut[i] = this.gatewayUsIn[i];
      }
      return { playback: this.gatewayUsOutToFloat(), levels };
    }

    const dsOut = new Array(n);
    const usOut = new Array(n);

    for (let slot = 0; slot < n; slot++) {
      const { heap16, ptrs } = this.slots[slot].bindings;
      if (slot === 0) {
        this.writeBuffer(heap16, ptrs.downstreamInPtr, this.gatewayDsOut);
      } else {
        this.writeBuffer(heap16, ptrs.downstreamInPtr, dsOut[slot - 1]);
      }
    }

    for (let slot = n - 1; slot >= 0; slot--) {
      const { module, heap16, ptrs } = this.slots[slot].bindings;
      const { upstreamInPtr, upstreamOutPtr, downstreamOutPtr } = ptrs;

      if (slot === n - 1) {
        if (this.loopbackEnabled) {
          const dsOutBase = downstreamOutPtr >> 1;
          const usInBase = upstreamInPtr >> 1;
          for (let i = 0; i < BUFFER_LEN; i++) {
            heap16[usInBase + i] = heap16[dsOutBase + i];
          }
        } else {
          this.clearBuffer(heap16, upstreamInPtr);
        }
      } else {
        this.writeBuffer(heap16, upstreamInPtr, usOut[slot + 1]);
      }

      module._sim_process_upstream();
      module._sim_process_downstream();

      this.copyBuffer(dsOut[slot], null, heap16, downstreamOutPtr);
      this.copyBuffer(usOut[slot], null, heap16, upstreamOutPtr);

      const dsInBase = ptrs.downstreamInPtr >> 1;
      const inLevel = peakInt16Interleaved(heap16, dsInBase, BUFFER_LEN);
      const outLevel = peakInt16Interleaved(heap16, upstreamOutPtr >> 1, BUFFER_LEN);
      levels.push({ in: inLevel, out: outLevel });
    }

    for (let i = 0; i < BUFFER_LEN; i++) {
      this.gatewayUsIn[i] = usOut[0][i];
    }
    for (let i = 0; i < BUFFER_LEN; i++) {
      this.gatewayUsOut[i] = this.gatewayUsIn[i];
    }

    return { playback: this.gatewayUsOutToFloat(), levels };
  }

  gatewayUsOutToFloat() {
    const playback = new Float32Array(BUFFER_LEN);
    for (let i = 0; i < BUFFER_LEN; i++) {
      playback[i] = this.int16ToFloat(this.gatewayUsOut[i]);
    }
    return playback;
  }
}
