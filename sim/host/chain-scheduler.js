import { peakInt16Interleaved } from './level-graph.js';
import { SAMPLE_RATE, BUFFER_LEN } from '../shared/audio-constants.js';

export { SAMPLE_RATE, BUFFER_LEN };

export const MAX_PROCESSING_UNITS = 8;

/** Merger stress flags returned from WASM harness (sim_consume_merger_stress). */
export const MERGER_STRESS_UNDERRUN = 1;
export const MERGER_STRESS_OVERRUN = 2;
export const MERGER_STRESS_REV_OVERRUN = 4;

/** Hardware underrun indicator for unit cards. */
export const HARDWARE_STRESS_UNDERRUN = 8;
export const HARDWARE_STRESS_OVERRUN = 16;
export const HARDWARE_STRESS_SUSTAINED = 32;

/**
 * Routes firmware-sized buffers through gateway (index 0) plus N processing slots.
 * Inter-unit downstream links use one-buffer delayed handoff left-to-right; upstream
 * return-path feeds use separate one-buffer delayed state right-to-left.
 * Hardware slots add ring-buffer pipeline delay (see hardwareAdapter.pipelineDelayPeriods).
 */
export class ChainScheduler {
  constructor() {
    /** @type {import('./hardware-slot-adapter.js').HardwareSlotAdapter | null} */
    this.hardwareAdapter = null;
    /** @type {object[]} */
    this.slots = [];
    this.loopbackEnabled = false;
    /** @type {Int16Array[]} */
    this.prevDsOut = [];
    /** @type {Int16Array[]} */
    this.prevUsOut = [];
    /** Extra delay lines for hardware pipeline depth */
    this.hwDelayDs = [];
    this.hwDelayUs = [];
    this.gatewayDsIn = new Int16Array(BUFFER_LEN);
    this.gatewayDsOut = new Int16Array(BUFFER_LEN);
    this.gatewayUsIn = new Int16Array(BUFFER_LEN);
    this.gatewayUsOut = new Int16Array(BUFFER_LEN);
  }

  setHardwareAdapter(adapter) {
    this.hardwareAdapter = adapter;
  }

  setSlots(slots) {
    this.slots = slots;
    while (this.prevDsOut.length < slots.length) {
      this.prevDsOut.push(new Int16Array(BUFFER_LEN));
    }
    while (this.prevUsOut.length < slots.length) {
      this.prevUsOut.push(new Int16Array(BUFFER_LEN));
    }
    while (this.hwDelayDs.length < slots.length) {
      this.hwDelayDs.push([]);
    }
    while (this.hwDelayUs.length < slots.length) {
      this.hwDelayUs.push([]);
    }
    this.prevDsOut.length = slots.length;
    this.prevUsOut.length = slots.length;
    this.hwDelayDs.length = slots.length;
    this.hwDelayUs.length = slots.length;
    this.resetPathDelayState();
  }

  resetPathDelayState() {
    for (const buf of this.prevDsOut) {
      buf.fill(0);
    }
    for (const buf of this.prevUsOut) {
      buf.fill(0);
    }
    for (const lines of this.hwDelayDs) {
      lines.length = 0;
    }
    for (const lines of this.hwDelayUs) {
      lines.length = 0;
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

  isHardwareSlot(slot) {
    return !!(slot.hardwareDesignated && slot.hardwareConnected && this.hardwareAdapter);
  }

  pushHardwareDelay(slotIndex, dsBuf, usBuf) {
    const depth = this.hardwareAdapter?.pipelineDelayPeriods ?? 2;
    const dsLines = this.hwDelayDs[slotIndex];
    const usLines = this.hwDelayUs[slotIndex];
    dsLines.push(Int16Array.from(dsBuf));
    usLines.push(Int16Array.from(usBuf));
    while (dsLines.length > depth) {
      dsLines.shift();
    }
    while (usLines.length > depth) {
      usLines.shift();
    }
    return {
      ds: dsLines.length > 0 ? dsLines[0] : dsBuf,
      us: usLines.length > 0 ? usLines[0] : usBuf,
    };
  }

  /**
   * @param {Float32Array} micFloatSamples mono float from Web Audio
   * @param {boolean} micEnabled
   */
  processBuffer(micFloatSamples, micEnabled) {
    const n = this.slots.length;
    const levels = new Array(n);

    for (let i = 0; i < BUFFER_LEN; i++) {
      this.gatewayDsIn[i] = micEnabled ? this.floatToInt16(micFloatSamples[i]) : 0;
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

    const playbackUs = Int16Array.from(this.prevUsOut[0]);
    const dsOut = Array.from({ length: n }, () => new Int16Array(BUFFER_LEN));
    const usOut = Array.from({ length: n }, () => new Int16Array(BUFFER_LEN));

    for (let slot = n - 1; slot >= 0; slot--) {
      const slotConfig = this.slots[slot];

      let dsIn;
      if (slot === 0) {
        dsIn = this.gatewayDsOut;
      } else {
        dsIn = this.prevDsOut[slot - 1];
      }

      let usIn;
      if (slot === n - 1) {
        usIn = this.loopbackEnabled
          ? this.prevDsOut[slot]
          : new Int16Array(BUFFER_LEN);
      } else {
        usIn = this.prevUsOut[slot + 1];
      }

      const inLevel = peakInt16Mono(dsIn);

      if (this.isHardwareSlot(slotConfig)) {
        const delayed = this.pushHardwareDelay(slot, dsIn, usIn);
        this.hardwareAdapter.pushPeriod(delayed.ds, delayed.us);
        const hwResult = this.hardwareAdapter.popOutputs();
        dsOut[slot].set(hwResult.downstreamOut);
        usOut[slot].set(hwResult.upstreamOut);
        const meter = this.hardwareAdapter.getMeterLevels();
        let stress = 0;
        if (hwResult.underrun) {
          stress |= HARDWARE_STRESS_UNDERRUN;
        }
        if (hwResult.sustained || meter.sustained) {
          stress |= HARDWARE_STRESS_SUSTAINED;
        }
        if (meter.overrun) {
          stress |= HARDWARE_STRESS_OVERRUN;
        }
        levels[slot] = {
          in: meter.in,
          out: meter.out,
          stress,
          telemetry: { ...this.hardwareAdapter.telemetry },
        };
      } else if (slotConfig.hardwareDesignated && !slotConfig.hardwareConnected) {
        dsOut[slot].fill(0);
        usOut[slot].fill(0);
        levels[slot] = { in: inLevel, out: 0 };
      } else if (slotConfig.bindings) {
        const { module, heap16, ptrs } = slotConfig.bindings;
        const { upstreamInPtr, upstreamOutPtr, downstreamInPtr, downstreamOutPtr } = ptrs;

        this.writeBuffer(heap16, downstreamInPtr, dsIn);
        this.writeBuffer(heap16, upstreamInPtr, usIn);

        module._sim_process_upstream();
        module._sim_process_downstream();

        let stress = 0;
        if (typeof module._sim_consume_merger_stress === 'function') {
          stress = module._sim_consume_merger_stress();
        }

        this.copyBuffer(dsOut[slot], heap16, downstreamOutPtr);
        this.copyBuffer(usOut[slot], heap16, upstreamOutPtr);
        const outLevel = peakInt16Interleaved(heap16, upstreamOutPtr >> 1, BUFFER_LEN);
        levels[slot] = { in: inLevel, out: outLevel, stress };
      } else {
        dsOut[slot].fill(0);
        usOut[slot].fill(0);
        levels[slot] = { in: inLevel, out: 0 };
      }

      this.prevDsOut[slot].set(dsOut[slot]);
      this.prevUsOut[slot].set(usOut[slot]);
    }

    for (let i = 0; i < BUFFER_LEN; i++) {
      this.gatewayUsIn[i] = playbackUs[i];
      this.gatewayUsOut[i] = playbackUs[i];
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

function peakInt16Mono(samples) {
  let peak = 0;
  for (let i = 0; i < samples.length; i++) {
    const abs = Math.abs(samples[i]);
    if (abs > peak) {
      peak = abs;
    }
  }
  return peak / 32768;
}
