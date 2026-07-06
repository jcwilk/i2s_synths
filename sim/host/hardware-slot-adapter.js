import { BUFFER_LEN, PERIOD_MS, buildExchangeRequest, parseExchangeResponse } from './frame-protocol.js';

const DEFAULT_RING_DEPTH = 2;

/**
 * Async hardware slot: ring buffers between chain scheduler and bridge relay worker.
 */
export class HardwareSlotAdapter {
  /**
   * @param {import('./bridge-client.js').BridgeClient} bridgeClient
   */
  constructor(bridgeClient) {
    this.bridge = bridgeClient;
    this.sequence = 0;
    this.connected = false;
    this.underrun = false;
    this.exchangeInFlight = false;
    /** @type {ReturnType<typeof setInterval> | null} */
    this.timer = null;
    this.silenceDs = new Int16Array(BUFFER_LEN);
    this.silenceUs = new Int16Array(BUFFER_LEN);
    this.lastDsIn = new Int16Array(BUFFER_LEN);
    this.lastUsOut = new Int16Array(BUFFER_LEN);
    this.pendingOutbound = null;
    /** @type {{ downstreamOut: Int16Array, upstreamOut: Int16Array } | null} */
    this.latestInbound = null;
    this.inboundQueue = [];
    this.maxInbound = DEFAULT_RING_DEPTH;
  }

  get pipelineDelayPeriods() {
    return DEFAULT_RING_DEPTH;
  }

  start() {
    this.connected = true;
    this.sequence = 0;
    this.inboundQueue = [];
    this.latestInbound = null;
    this.pendingOutbound = null;
    this.stopTimer();
    this.timer = setInterval(() => {
      this._tickExchange().catch(() => {});
    }, PERIOD_MS);
  }

  stop() {
    this.connected = false;
    this.stopTimer();
    this.inboundQueue = [];
    this.latestInbound = null;
    this.pendingOutbound = null;
  }

  stopTimer() {
    if (this.timer !== null) {
      clearInterval(this.timer);
      this.timer = null;
    }
  }

  pushPeriod(downstreamIn, upstreamIn) {
    this.lastDsIn.set(downstreamIn);
    this.pendingOutbound = {
      downstreamIn: Int16Array.from(downstreamIn),
      upstreamIn: Int16Array.from(upstreamIn),
    };
  }

  popOutputs() {
    if (this.inboundQueue.length > 0) {
      const inbound = this.inboundQueue.shift();
      this.latestInbound = inbound;
      this.lastUsOut.set(inbound.upstreamOut);
      this.underrun = false;
      return { ...inbound, underrun: false };
    }
    this.underrun = true;
    return {
      downstreamOut: this.silenceDs,
      upstreamOut: this.silenceUs,
      underrun: true,
    };
  }

  getMeterLevels() {
    return {
      in: peakInt16(this.lastDsIn),
      out: peakInt16(this.lastUsOut),
      underrun: this.underrun,
    };
  }

  async _tickExchange() {
    if (!this.connected || this.exchangeInFlight || !this.bridge.sessionActive) {
      return;
    }
    const outbound = this.pendingOutbound;
    if (!outbound) {
      return;
    }
    this.pendingOutbound = null;
    this.exchangeInFlight = true;
    try {
      const request = buildExchangeRequest({
        sequence: this.sequence++,
        downstreamIn: outbound.downstreamIn,
        upstreamIn: outbound.upstreamIn,
      });
      const responseBuf = await this.bridge.relayExchange(request);
      const wire = new Uint8Array(responseBuf);
      const inner = wire.subarray(4);
      const parsed = parseExchangeResponse(inner);
      const entry = {
        downstreamOut: parsed.downstreamOut,
        upstreamOut: parsed.upstreamOut,
      };
      this.inboundQueue.push(entry);
      while (this.inboundQueue.length > this.maxInbound) {
        this.inboundQueue.shift();
      }
    } catch (_) {
      this.underrun = true;
    } finally {
      this.exchangeInFlight = false;
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
