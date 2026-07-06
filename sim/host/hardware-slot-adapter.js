import {
  BUFFER_LEN,
  PERIOD_MS,
  buildExchangeRequest,
  parseExchangeResponse,
  BRIDGE_STATUS_SEQ_GAP,
} from './frame-protocol.js';

/** Default ring depth per Phase 3 design (periods each direction). */
export const DEFAULT_RING_DEPTH = 3;
export const ADAPTIVE_RING_DEPTH = 4;
const SUSTAINED_UNDERRUN_WINDOW_MS = 30000;
const SUSTAINED_UNDERRUN_RATE = 0.05;
const TRANSIENT_CLEAR_PERIODS = 3;
const PIPELINE_SPIKE_RTT_FACTOR = 1.5;
const RTT_SAMPLE_CAP = 120;

/**
 * Async hardware slot: ring buffers between chain scheduler and bridge relay worker.
 * Phase 3: depth-3 default, adaptive depth-4, pipelined exchanges, recovery stats.
 */
export class HardwareSlotAdapter {
  /**
   * @param {import('./bridge-client.js').BridgeClient} bridgeClient
   */
  constructor(bridgeClient) {
    this.bridge = bridgeClient;
    this.sequence = 0;
    this.connected = false;
    this.transientUnderrun = false;
    this.sustainedUnderrunWarning = false;
    /** @type {ReturnType<typeof setInterval> | null} */
    this.timer = null;
    this.silenceDs = new Int16Array(BUFFER_LEN);
    this.silenceUs = new Int16Array(BUFFER_LEN);
    this.lastDsIn = new Int16Array(BUFFER_LEN);
    this.lastUsOut = new Int16Array(BUFFER_LEN);
  /** @type {{ downstreamIn: Int16Array, upstreamIn: Int16Array } | null} */
    this.pendingOutbound = null;
    /** @type {{ downstreamOut: Int16Array, upstreamOut: Int16Array } | null} */
    this.latestInbound = null;
    this.inboundQueue = [];
    this.maxInbound = DEFAULT_RING_DEPTH;
    /** @type {{ downstreamIn: Int16Array, upstreamIn: Int16Array, sequence: number }[]} */
    this.outboundQueue = [];
    this.inFlightCount = 0;
    this.maxInFlight = 1;
    this.rttMsSamples = [];
    this.underrunCount = 0;
    this.overrunCount = 0;
    this.goodPeriodStreak = 0;
    this.windowUnderruns = 0;
    this.windowPeriods = 0;
    this.windowStartMs = 0;
    this.telemetry = { primary: 0.5, secondary: 0.5 };
    this.fatalError = null;
    this.onFatal = () => {};
  }

  get pipelineDelayPeriods() {
    return this.maxInbound;
  }

  get soakSummary() {
    return {
      underrunCount: this.underrunCount,
      overrunCount: this.overrunCount,
      sustainedUnderrunWarning: this.sustainedUnderrunWarning,
      ringDepth: this.maxInbound,
      maxInFlight: this.maxInFlight,
    };
  }

  start() {
    this.connected = true;
    this.sequence = 0;
    this.inboundQueue = [];
    this.outboundQueue = [];
    this.latestInbound = null;
    this.pendingOutbound = null;
    this.inFlightCount = 0;
    this.maxInFlight = 1;
    this.rttMsSamples = [];
    this.underrunCount = 0;
    this.overrunCount = 0;
    this.transientUnderrun = false;
    this.sustainedUnderrunWarning = false;
    this.goodPeriodStreak = 0;
    this.windowUnderruns = 0;
    this.windowPeriods = 0;
    this.windowStartMs = performance.now();
    this.maxInbound = DEFAULT_RING_DEPTH;
    this.fatalError = null;
    this.stopTimer();
    this.timer = setInterval(() => {
      this._tickExchange().catch(() => {});
    }, PERIOD_MS);
  }

  stop() {
    this.connected = false;
    this.stopTimer();
    this.inboundQueue = [];
    this.outboundQueue = [];
    this.latestInbound = null;
    this.pendingOutbound = null;
    this.inFlightCount = 0;
  }

  stopTimer() {
    if (this.timer !== null) {
      clearInterval(this.timer);
      this.timer = null;
    }
  }

  pushPeriod(downstreamIn, upstreamIn) {
    this.lastDsIn.set(downstreamIn);
    const frame = {
      downstreamIn: Int16Array.from(downstreamIn),
      upstreamIn: Int16Array.from(upstreamIn),
    };
    if (!this.pendingOutbound) {
      this.pendingOutbound = frame;
      return;
    }
    this.outboundQueue.push(frame);
    while (this.outboundQueue.length > this.maxInbound) {
      this.outboundQueue.shift();
      this.overrunCount += 1;
    }
  }

  popOutputs() {
    this._advanceUnderrunWindow(false);

    if (this.inboundQueue.length > 0) {
      const inbound = this.inboundQueue.shift();
      this.latestInbound = inbound;
      this.lastUsOut.set(inbound.upstreamOut);
      this.transientUnderrun = false;
      this.goodPeriodStreak += 1;
      if (this.goodPeriodStreak >= TRANSIENT_CLEAR_PERIODS) {
        this.transientUnderrun = false;
      }
      return { ...inbound, underrun: false, overrun: false, sustained: false };
    }

    this.transientUnderrun = true;
    this.underrunCount += 1;
    this.goodPeriodStreak = 0;
    this._advanceUnderrunWindow(true);
    return {
      downstreamOut: this.silenceDs,
      upstreamOut: this.silenceUs,
      underrun: true,
      overrun: false,
      sustained: this.sustainedUnderrunWarning,
    };
  }

  _advanceUnderrunWindow(hadUnderrun) {
    const now = performance.now();
    if (this.windowStartMs === 0) {
      this.windowStartMs = now;
    }
    this.windowPeriods += 1;
    if (hadUnderrun) {
      this.windowUnderruns += 1;
    }
    if (now - this.windowStartMs >= SUSTAINED_UNDERRUN_WINDOW_MS) {
      const rate = this.windowPeriods > 0
        ? this.windowUnderruns / this.windowPeriods
        : 0;
      if (rate > SUSTAINED_UNDERRUN_RATE) {
        this.sustainedUnderrunWarning = true;
      }
      this.windowStartMs = now;
      this.windowUnderruns = 0;
      this.windowPeriods = 0;
    }
  }

  getMeterLevels() {
    return {
      in: peakInt16(this.lastDsIn),
      out: peakInt16(this.lastUsOut),
      underrun: this.transientUnderrun,
      sustained: this.sustainedUnderrunWarning,
      overrun: this.outboundQueue.length > 0 && this.inFlightCount >= this.maxInFlight,
    };
  }

  _maybeAdaptRingDepth() {
    if (this.sustainedUnderrunWarning && this.maxInbound < ADAPTIVE_RING_DEPTH) {
      this.maxInbound = ADAPTIVE_RING_DEPTH;
    }
  }

  _maybeEnablePipelining() {
    if (this.rttMsSamples.length < 20) {
      return;
    }
    const sorted = [...this.rttMsSamples].sort((a, b) => a - b);
    const p50 = sorted[Math.floor(sorted.length * 0.5)] ?? 0;
    if (p50 > PERIOD_MS * PIPELINE_SPIKE_RTT_FACTOR) {
      this.maxInFlight = 2;
    }
  }

  _recordRtt(ms) {
    this.rttMsSamples.push(ms);
    while (this.rttMsSamples.length > RTT_SAMPLE_CAP) {
      this.rttMsSamples.shift();
    }
    this._maybeEnablePipelining();
  }

  _handleFatalStatus(status, sequence) {
    if ((status & BRIDGE_STATUS_SEQ_GAP) !== 0) {
      this.fatalError = `sequence gap at ${sequence}`;
      this.onFatal(this.fatalError);
      this.stop();
      this.bridge.stopSession().catch(() => {});
    }
  }

  async _submitExchange(outbound, seq) {
    const t0 = performance.now();
    this.inFlightCount += 1;
    try {
      const request = buildExchangeRequest({
        sequence: seq,
        downstreamIn: outbound.downstreamIn,
        upstreamIn: outbound.upstreamIn,
      });
      const responseBuf = await this.bridge.relayExchange(request, seq);
      const wire = new Uint8Array(responseBuf);
      const inner = wire.subarray(4);
      const parsed = parseExchangeResponse(inner);
      this._recordRtt(performance.now() - t0);

      if (parsed.primaryTelemetry !== undefined) {
        this.telemetry.primary = parsed.primaryTelemetry;
        this.telemetry.secondary = parsed.secondaryTelemetry ?? this.telemetry.secondary;
      }

      if ((parsed.status & BRIDGE_STATUS_SEQ_GAP) !== 0) {
        this._handleFatalStatus(parsed.status, parsed.sequence);
        return;
      }

      const entry = {
        downstreamOut: parsed.downstreamOut,
        upstreamOut: parsed.upstreamOut,
      };
      this.inboundQueue.push(entry);
      while (this.inboundQueue.length > this.maxInbound) {
        this.inboundQueue.shift();
      }
      this._maybeAdaptRingDepth();
    } catch (_) {
      this.transientUnderrun = true;
    } finally {
      this.inFlightCount -= 1;
    }
  }

  async _tickExchange() {
    if (!this.connected || !this.bridge.sessionActive || this.fatalError) {
      return;
    }

    while (this.inFlightCount < this.maxInFlight) {
      let outbound = null;
      let seq = 0;
      if (this.pendingOutbound) {
        outbound = this.pendingOutbound;
        this.pendingOutbound = null;
        seq = this.sequence++;
      } else if (this.outboundQueue.length > 0) {
        const queued = this.outboundQueue.shift();
        outbound = queued;
        seq = this.sequence++;
      } else {
        break;
      }
      await this._submitExchange(outbound, seq);
    }
  }
}

function peakInt16(samples) {
  let peak = 0;
  for (let i = 0; i < samples.length; i++) {
    const abs = Math.abs(samples[i]);
    if (abs > peak) {
      peak = abs;
    }
  }
  return peak / 32768;
}
