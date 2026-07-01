/** Canvas redraw rate (Hz). */
export const LEVEL_RENDER_HZ = 30;

/** Peak samples arrive with each firmware audio buffer (~512 @ 44.1 kHz). */
export const AUDIO_PEAK_HZ = 44100 / 512;

/** Bottom of the log scale (dBFS). */
export const DB_FLOOR = -54;

/**
 * Minimum linear ceiling for the top of the scale: max(X, peak in view).
 * Stops quiet/static from magnifying tiny spikes to full height.
 * ~0.12 ≈ -18 dBFS.
 */
export const MIN_SCALE_LINEAR = 0.12;

/** Y-axis scale easing per render frame (attack / release). */
const SCALE_ATTACK = 0.12;
const SCALE_RELEASE = 0.04;

export function peakFloatInterleaved(floatSamples) {
  let peak = 0;
  for (let i = 0; i < floatSamples.length; i++) {
    const a = Math.abs(floatSamples[i]);
    if (a > peak) {
      peak = a;
    }
  }
  return peak;
}

export function peakInt16Interleaved(heap16, baseIndex, sampleCount) {
  let peak = 0;
  for (let i = 0; i < sampleCount; i++) {
    const a = Math.abs(heap16[baseIndex + i]);
    if (a > peak) {
      peak = a;
    }
  }
  return peak / 32768;
}

function linearToDb(linear) {
  if (linear <= 1e-8) {
    return DB_FLOOR;
  }
  return 20 * Math.log10(linear);
}

/** Map linear amplitude to [0, 1] using log scale with adaptive ceiling. */
export function linearToDisplay(linear, scaleTopLinear) {
  const top = Math.max(MIN_SCALE_LINEAR, scaleTopLinear, 1e-8);
  const dbTop = Math.min(0, linearToDb(top));
  const db = linearToDb(linear);
  const span = dbTop - DB_FLOOR;
  if (span <= 0) {
    return 0;
  }
  return Math.max(0, Math.min(1, (db - DB_FLOOR) / span));
}

function peakInHistories(histories, channels) {
  let peak = 0;
  for (const ch of channels) {
    const arr = histories[ch.key];
    for (let i = 0; i < arr.length; i++) {
      if (arr[i] > peak) {
        peak = arr[i];
      }
    }
  }
  return peak;
}

/**
 * Scrolling level history for one or more signals (newest on the right).
 * Stores linear peak per buffer; draw uses adaptive log scaling.
 */
export class LevelGraph {
  /**
   * @param {HTMLCanvasElement} canvas
   * @param {{ durationSec: number, title: string, channels: { key: string, label: string, color: string, fill?: string }[] }} options
   */
  constructor(canvas, options) {
    this.canvas = canvas;
    this.durationSec = options.durationSec;
    this.title = options.title;
    this.channels = options.channels;
    const sampleHz = options.sampleRateHz ?? AUDIO_PEAK_HZ;
    this.maxSamples = Math.ceil(this.durationSec * sampleHz);
    this.history = Object.fromEntries(this.channels.map((c) => [c.key, []]));
    this.displayScaleTop = MIN_SCALE_LINEAR;
    this.pixelCols = null;
    this.pixelColsWidth = 0;
    this.bucket = Object.fromEntries(this.channels.map((c) => [c.key, 0]));
    this.sampleAccum = 0;
    this.samplesPerPixel = 1;

    this.ctx = canvas.getContext('2d', { alpha: false });
    this._resize = () => this.resize();
    this.resize();
    window.addEventListener('resize', this._resize);
  }

  destroy() {
    window.removeEventListener('resize', this._resize);
  }

  resize() {
    const rect = this.canvas.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    const w = Math.max(1, Math.round(rect.width * dpr));
    const h = Math.max(1, Math.round(rect.height * dpr));
    if (this.canvas.width !== w || this.canvas.height !== h) {
      this.canvas.width = w;
      this.canvas.height = h;
      this.draw();
    }
  }

  getPlotWInt() {
    const w = this.canvas.width;
    const dpr = window.devicePixelRatio || 1;
    const plotLeft = Math.round(34 * dpr);
    const plotRight = w - Math.round(6 * dpr);
    return Math.max(1, Math.floor(plotRight - plotLeft));
  }

  resetPixelScroll(plotWInt) {
    if (this.pixelColsWidth === plotWInt && this.pixelCols) {
      return;
    }
    this.pixelColsWidth = plotWInt;
    this.pixelCols = Object.fromEntries(
      this.channels.map((c) => [c.key, new Float32Array(plotWInt)]),
    );
    this.bucket = Object.fromEntries(this.channels.map((c) => [c.key, 0]));
    this.sampleAccum = 0;
    this.samplesPerPixel = this.maxSamples / plotWInt;
  }

  /**
   * Advance the pixel column buffer on each audio peak (not each redraw).
   * Committed columns stay fixed between shifts — redraw only repaints them.
   */
  advancePixelScroll(levels) {
    const plotWInt = this.getPlotWInt();
    this.resetPixelScroll(plotWInt);

    for (const ch of this.channels) {
      this.bucket[ch.key] = Math.max(this.bucket[ch.key], levels[ch.key] ?? 0);
    }
    this.sampleAccum += 1;

    let shifts = 0;
    while (this.sampleAccum >= this.samplesPerPixel) {
      this.sampleAccum -= this.samplesPerPixel;
      shifts += 1;
      for (const ch of this.channels) {
        const cols = this.pixelCols[ch.key];
        cols.copyWithin(0, 1);
        cols[cols.length - 1] = this.bucket[ch.key];
      }
    }
    if (shifts > 0) {
      for (const ch of this.channels) {
        this.bucket[ch.key] = 0;
      }
      for (const ch of this.channels) {
        this.bucket[ch.key] = Math.max(this.bucket[ch.key], levels[ch.key] ?? 0);
      }
    }
  }

  pixelColumnPeak(ch, x, plotWInt) {
    const cols = this.pixelCols?.[ch.key];
    if (!cols) {
      return 0;
    }
    const committed = cols[x];
    if (x === plotWInt - 1) {
      return Math.max(committed, this.bucket[ch.key]);
    }
    return committed;
  }

  /** Append a sample to history without redrawing. */
  push(levels) {
    for (const ch of this.channels) {
      const arr = this.history[ch.key];
      arr.push(Math.max(0, levels[ch.key] ?? 0));
      while (arr.length > this.maxSamples) {
        arr.shift();
      }
    }
    this.advancePixelScroll(levels);
  }

  clear() {
    for (const ch of this.channels) {
      this.history[ch.key].length = 0;
    }
    this.displayScaleTop = MIN_SCALE_LINEAR;
    this.pixelCols = null;
    this.pixelColsWidth = 0;
    this.bucket = Object.fromEntries(this.channels.map((c) => [c.key, 0]));
    this.sampleAccum = 0;
    this.draw();
  }

  /** Ease the log-scale ceiling so the Y axis does not jump every frame. */
  smoothScaleTop(target) {
    const goal = Math.max(MIN_SCALE_LINEAR, target);
    const rate = goal > this.displayScaleTop ? SCALE_ATTACK : SCALE_RELEASE;
    this.displayScaleTop += rate * (goal - this.displayScaleTop);
    return this.displayScaleTop;
  }

  draw() {
    const { ctx, canvas, channels, history, title } = this;
    const w = canvas.width;
    const h = canvas.height;
    const dpr = window.devicePixelRatio || 1;

    ctx.imageSmoothingEnabled = false;

    const pad = {
      top: Math.round(10 * dpr),
      bottom: Math.round(18 * dpr),
      left: Math.round(34 * dpr),
      right: Math.round(6 * dpr),
    };
    const plotLeft = pad.left;
    const plotTop = pad.top;
    const plotRight = w - pad.right;
    const plotBottom = h - pad.bottom;
    const plotW = plotRight - plotLeft;
    const plotH = plotBottom - plotTop;

    const viewPeak = peakInHistories(history, channels);
    const scaleTopLinear = this.smoothScaleTop(viewPeak);
    const dbTop = Math.min(0, linearToDb(scaleTopLinear));

    ctx.fillStyle = '#141820';
    ctx.fillRect(0, 0, w, h);

    ctx.strokeStyle = '#2a3140';
    ctx.lineWidth = 1;
    ctx.fillStyle = '#6a7488';
    ctx.font = `${Math.round(9 * dpr)}px system-ui, sans-serif`;
    ctx.textAlign = 'right';

    for (const db of [-12, -24, -36, -48]) {
      if (db < DB_FLOOR || db > dbTop) {
        continue;
      }
      const norm = linearToDisplay(10 ** (db / 20), scaleTopLinear);
      const y = Math.round(plotTop + plotH * (1 - norm)) + 0.5;
      ctx.beginPath();
      ctx.moveTo(plotLeft, y);
      ctx.lineTo(plotRight, y);
      ctx.stroke();
      ctx.fillText(`${db}`, plotLeft - 4, Math.round(y + 3 * dpr));
    }

    const plotWInt = Math.max(1, Math.floor(plotW));
    this.resetPixelScroll(plotWInt);

    for (const ch of channels) {
      if (!this.pixelCols?.[ch.key]) {
        continue;
      }
      ctx.fillStyle = ch.color;
      for (let x = 0; x < plotWInt; x++) {
        const colPeak = this.pixelColumnPeak(ch, x, plotWInt);
        const norm = linearToDisplay(colPeak, scaleTopLinear);
        const yTop = Math.round(plotTop + plotH * (1 - norm));
        const barH = plotBottom - yTop;
        if (barH > 0) {
          ctx.fillRect(plotLeft + x, yTop, 1, barH);
        }
      }
    }

    ctx.fillStyle = '#8a95a8';
    ctx.font = `${Math.round(10 * dpr)}px system-ui, sans-serif`;
    ctx.textAlign = 'left';
    const topDbLabel = dbTop.toFixed(0);
    ctx.fillText(`${title} (−${Math.abs(DB_FLOOR)}…${topDbLabel} dB)`, plotLeft + 2, h - Math.round(4 * dpr));
  }
}

export const LEVEL_GRAPH_CHANNELS = [
  { key: 'out', label: 'Out', color: '#ff8c5a' },
  { key: 'in', label: 'In', color: '#5eb8ff' },
];

export const LEVEL_GRAPH_SPEEDS = [
  { id: 'fast', title: '4 s', durationSec: 4 },
  { id: 'medium', title: '16 s', durationSec: 16 },
  { id: 'slow', title: '64 s', durationSec: 64 },
];

/**
 * Pushes peaks from the audio path; graphs redraw at renderHz.
 */
export class LevelGraphSampler {
  /**
   * @param {LevelGraph[]} graphs
   * @param {number} renderHz
   */
  constructor(graphs, renderHz = LEVEL_RENDER_HZ) {
    this.graphs = graphs;
    this.renderIntervalMs = 1000 / renderHz;
    this.renderTimer = null;
  }

  /** Called once per firmware audio buffer — push immediately, no timer hold. */
  feed(levels) {
    for (const graph of this.graphs) {
      graph.push(levels);
    }
  }

  start() {
    this.stop();
    this.renderTimer = setInterval(() => this.render(), this.renderIntervalMs);
  }

  stop() {
    if (this.renderTimer !== null) {
      clearInterval(this.renderTimer);
      this.renderTimer = null;
    }
  }

  render() {
    for (const graph of this.graphs) {
      graph.draw();
    }
  }
}
