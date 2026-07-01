import { peakInt16Interleaved } from './level-graph.js';

export const SAMPLE_RATE = 44100;
export const BUFFER_LEN = 512;
export const MAX_PROCESSING_UNITS = 8;

/**
 * Routes firmware-sized buffers through gateway (index 0) plus N WASM processing slots.
 * Inter-unit downstream links and rightmost loopback use one-buffer delayed handoff
 * (previous period's downstream output), matching Phase 0 single-unit audibility.
 */
export class ChainScheduler {
  constructor() {
    /** @type {{ bindings: object }[]} */
    this.slots = [];
    this.loopbackEnabled = false;
    /** @type {Int16Array[]} previous downstream out per slot (loopback + inter-unit ds feed) */
    this.prevDsOut = [];
    this.gatewayDsIn = new Int16Array(BUFFER_LEN);
    this.gatewayDsOut = new Int16Array(BUFFER_LEN);
    this.gatewayUsIn = new Int16Array(BUFFER_LEN);
    this.gatewayUsOut = new Int16Array(BUFFER_LEN);
  }

  setSlots(slots) {
    this.slots = slots;
    while (this.prevDsOut.length < slots.length) {
      this.prevDsOut.push(new Int16Array(BUFFER_LEN));
    }
    this.prevDsOut.length = slots.length;
    for (const buf of this.prevDsOut) {
      buf.fill(0);
    }
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

  copyBuffer(dst, heap16, srcPtr) {
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
    const levels = new Array(n);

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

    const dsOut = Array.from({ length: n }, () => new Int16Array(BUFFER_LEN));
    const usOut = Array.from({ length: n }, () => new Int16Array(BUFFER_LEN));

    // Right-to-left: upstream return path needs us_out[i+1] before processing slot i.
    for (let slot = n - 1; slot >= 0; slot--) {
      const { module, heap16, ptrs } = this.slots[slot].bindings;
      const { upstreamInPtr, upstreamOutPtr, downstreamInPtr, downstreamOutPtr } = ptrs;

      if (slot === 0) {
        this.writeBuffer(heap16, downstreamInPtr, this.gatewayDsOut);
      } else {
        this.writeBuffer(heap16, downstreamInPtr, this.prevDsOut[slot - 1]);
      }

      const dsInBase = downstreamInPtr >> 1;
      const inLevel = peakInt16Interleaved(heap16, dsInBase, BUFFER_LEN);

      if (slot === n - 1) {
        if (this.loopbackEnabled) {
          this.writeBuffer(heap16, upstreamInPtr, this.prevDsOut[slot]);
        } else {
          this.clearBuffer(heap16, upstreamInPtr);
        }
      } else {
        this.writeBuffer(heap16, upstreamInPtr, usOut[slot + 1]);
      }

      module._sim_process_upstream();
      module._sim_process_downstream();

      this.copyBuffer(dsOut[slot], heap16, downstreamOutPtr);
      this.copyBuffer(usOut[slot], heap16, upstreamOutPtr);
      this.prevDsOut[slot].set(dsOut[slot]);

      const outLevel = peakInt16Interleaved(heap16, upstreamOutPtr >> 1, BUFFER_LEN);
      levels[slot] = { in: inLevel, out: outLevel };
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
