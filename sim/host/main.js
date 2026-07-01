import { SimAudioEngine } from './audio-engine.js';
import { POT_POLL_MS, applyPotStep } from './pot-simulator.js';
import { LevelGraph, LevelGraphSampler, LEVEL_GRAPH_CHANNELS, LEVEL_GRAPH_SPEEDS } from './level-graph.js';

const MODULES = {
  passthrough: { label: 'Passthrough', artifact: '../wasm/out/passthrough.js' },
  delay: { label: 'Delay', artifact: '../wasm/out/delay.js' },
};

let activeModule = null;
let wasmModule = null;
let wasmBindings = null;

const ui = {
  moduleSelect: document.getElementById('module-select'),
  primaryPot: document.getElementById('primary-pot'),
  secondaryPot: document.getElementById('secondary-pot'),
  primaryValue: document.getElementById('primary-value'),
  secondaryValue: document.getElementById('secondary-value'),
  micToggle: document.getElementById('mic-toggle'),
  startBtn: document.getElementById('start-btn'),
  status: document.getElementById('status'),
  delayInfo: document.getElementById('delay-info'),
};

const engine = new SimAudioEngine();
engine.onStatus = (msg) => {
  ui.status.textContent = msg;
};

const levelGraphs = LEVEL_GRAPH_SPEEDS.map((speed) => {
  const canvas = document.getElementById(`level-graph-${speed.id}`);
  return new LevelGraph(canvas, {
    durationSec: speed.durationSec,
    title: speed.title,
    channels: LEVEL_GRAPH_CHANNELS,
  });
});

const levelSampler = new LevelGraphSampler(levelGraphs);

engine.onLevels = (levels) => {
  levelSampler.feed(levels);
};

const potTargets = { primary: 0.5, secondary: 0 };
let potPollTimer = null;
let lastPotPollMs = 0;
let potDragRafId = null;
let potDragPointers = 0;

function potSliderTo01(slider) {
  return Number(slider.value) / 100;
}

function syncPotTargetsFromSliders() {
  potTargets.primary = potSliderTo01(ui.primaryPot);
  potTargets.secondary = potSliderTo01(ui.secondaryPot);
}

function updatePotValueLabels() {
  if (!wasmBindings) {
    return;
  }
  const { heapF32, potsPtr } = wasmBindings;
  const base = potsPtr >> 2;
  ui.primaryValue.textContent = `${(heapF32[base + 2] * 100).toFixed(1)}%`;
  ui.secondaryValue.textContent = `${(heapF32[base + 5] * 100).toFixed(1)}%`;
}

function tickPots(deltaMs) {
  if (!wasmBindings) {
    return;
  }
  const { heapF32, potsPtr } = wasmBindings;
  const base = potsPtr >> 2;
  applyPotStep(heapF32, base + 1, base + 2, potTargets.primary, deltaMs);
  applyPotStep(heapF32, base + 4, base + 5, potTargets.secondary, deltaMs);
  updatePotValueLabels();
}

function stopPotDragLoop() {
  if (potDragRafId !== null) {
    cancelAnimationFrame(potDragRafId);
    potDragRafId = null;
  }
}

function startPotDragLoop() {
  if (potDragRafId !== null) {
    return;
  }
  const frame = () => {
    syncPotTargetsFromSliders();
    potDragRafId = requestAnimationFrame(frame);
  };
  potDragRafId = requestAnimationFrame(frame);
}

function onPotSliderPointerDown(event) {
  event.currentTarget.setPointerCapture?.(event.pointerId);
  potDragPointers += 1;
  startPotDragLoop();
  syncPotTargetsFromSliders();
}

function onPotSliderPointerUp(event) {
  if (event.currentTarget.hasPointerCapture?.(event.pointerId)) {
    event.currentTarget.releasePointerCapture(event.pointerId);
  }
  potDragPointers = Math.max(0, potDragPointers - 1);
  if (potDragPointers === 0) {
    stopPotDragLoop();
  }
  syncPotTargetsFromSliders();
}

function bindPotSlider(slider) {
  slider.addEventListener('input', syncPotTargetsFromSliders);
  slider.addEventListener('change', syncPotTargetsFromSliders);
  slider.addEventListener('pointerdown', onPotSliderPointerDown);
  slider.addEventListener('pointerup', onPotSliderPointerUp);
  slider.addEventListener('pointercancel', onPotSliderPointerUp);
}

function stopPotPoller() {
  if (potPollTimer !== null) {
    clearInterval(potPollTimer);
    potPollTimer = null;
  }
  stopPotDragLoop();
  potDragPointers = 0;
  lastPotPollMs = 0;
}

function startPotPoller() {
  stopPotPoller();
  lastPotPollMs = performance.now();
  tickPots(POT_POLL_MS);
  potPollTimer = setInterval(() => {
    const now = performance.now();
    const deltaMs = now - lastPotPollMs;
    lastPotPollMs = now;
    tickPots(deltaMs);
  }, POT_POLL_MS);
}

/**
 * Emscripten MODULARIZE builds expose createSimModule. With EXPORT_ES6 that is
 * the module default export; legacy builds only assign a global / CommonJS export.
 */
async function loadEmscriptenFactory(artifactUrl, cacheKey) {
  let factory;
  try {
    const mod = await import(artifactUrl);
    factory = mod.default ?? mod.createSimModule;
  } catch (_) {
    factory = undefined;
  }
  if (typeof factory === 'function') {
    return factory;
  }

  const bustedUrl = `${artifactUrl}${artifactUrl.includes('?') ? '&' : '?'}sim=${encodeURIComponent(cacheKey)}`;

  return new Promise((resolve, reject) => {
    const script = document.createElement('script');
    script.src = bustedUrl;
    script.async = true;
    script.onload = () => {
      const factory = globalThis.createSimModule;
      if (typeof factory !== 'function') {
        reject(new Error('createSimModule not found after script load'));
        return;
      }
      resolve(factory);
    };
    script.onerror = () => reject(new Error(`Failed to load ${bustedUrl}`));
    document.head.appendChild(script);
  });
}

async function loadWasmModule(moduleKey) {
  stopPotPoller();
  if (wasmModule) {
    try {
      if (engine.running) {
        await engine.stop();
        ui.startBtn.textContent = 'Start audio';
      }
    } catch (_) {
      /* ignore */
    }
    wasmModule = null;
    wasmBindings = null;
  }

  const spec = MODULES[moduleKey];
  ui.status.textContent = `Loading ${spec.label} WASM…`;

  const artifactUrl = new URL(spec.artifact, import.meta.url).href;
  const factory = await loadEmscriptenFactory(artifactUrl, moduleKey);
  const module = await factory({
    locateFile: (path) => new URL(`../wasm/out/${path}`, import.meta.url).href,
  });

  module._sim_setup();

  const bufferLen = module._sim_get_buffer_len();
  const upstreamInPtr = module._sim_get_upstream_in();
  const upstreamOutPtr = module._sim_get_upstream_out();
  const downstreamInPtr = module._sim_get_downstream_in();
  const downstreamOutPtr = module._sim_get_downstream_out();
  const potsPtr = module._sim_get_pots();

  wasmModule = module;
  wasmBindings = {
    module,
    heap16: module.HEAP16,
    heapF32: module.HEAPF32,
    ptrs: {
      upstreamInPtr,
      upstreamOutPtr,
      downstreamInPtr,
      downstreamOutPtr,
    },
    potsPtr,
    bufferLen,
  };

  engine.setWasmBindings(wasmBindings);
  syncPotTargetsFromSliders();
  startPotPoller();

  if (moduleKey === 'delay' && module._sim_get_delay_buffer_frames) {
    const frames = module._sim_get_delay_buffer_frames();
    const ms = ((frames * 1000) / 44100).toFixed(1);
    ui.delayInfo.textContent = `Delay ring: ${frames} frames (${ms} ms capacity)`;
    ui.delayInfo.hidden = false;
  } else {
    ui.delayInfo.hidden = true;
  }

  ui.status.textContent = `${spec.label} module ready`;
}

async function onModuleChange() {
  const key = ui.moduleSelect.value;
  if (key === activeModule) {
    return;
  }
  activeModule = key;
  await loadWasmModule(key);
}

async function onStartClick() {
  if (engine.running) {
    levelSampler.stop();
    await engine.stop();
    ui.startBtn.textContent = 'Start audio';
    ui.startBtn.disabled = false;
    for (const graph of levelGraphs) {
      graph.clear();
    }
    return;
  }

  if (!wasmBindings) {
    await loadWasmModule(ui.moduleSelect.value);
  }

  ui.startBtn.disabled = true;
  try {
    await engine.start();
    engine.setMicEnabled(ui.micToggle.checked);
    levelSampler.start();
    ui.startBtn.textContent = 'Stop audio';
  } catch (err) {
    ui.status.textContent = `Failed to start audio: ${err.message}`;
  } finally {
    ui.startBtn.disabled = false;
  }
}

ui.moduleSelect.addEventListener('change', () => {
  onModuleChange().catch((err) => {
    ui.status.textContent = `Module load failed: ${err.message}`;
  });
});

bindPotSlider(ui.primaryPot);
bindPotSlider(ui.secondaryPot);

ui.micToggle.addEventListener('change', () => {
  engine.setMicEnabled(ui.micToggle.checked);
});

ui.startBtn.addEventListener('click', () => {
  onStartClick().catch((err) => {
    ui.status.textContent = `Audio error: ${err.message}`;
  });
});

activeModule = ui.moduleSelect.value;
loadWasmModule(activeModule).catch((err) => {
  ui.status.textContent = `Initial load failed: ${err.message}`;
});
