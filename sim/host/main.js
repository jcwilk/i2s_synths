import { SimAudioEngine } from './audio-engine.js';

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

function potSliderTo01(slider) {
  return Number(slider.value) / 100;
}

function updatePotLabels() {
  ui.primaryValue.textContent = `${ui.primaryPot.value}%`;
  ui.secondaryValue.textContent = `${ui.secondaryPot.value}%`;
  engine.updatePots(potSliderTo01(ui.primaryPot), potSliderTo01(ui.secondaryPot));
}

async function loadWasmModule(moduleKey) {
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

  const factory = (await import(spec.artifact)).default;
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
  updatePotLabels();

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
    await engine.stop();
    ui.startBtn.textContent = 'Start audio';
    ui.startBtn.disabled = false;
    return;
  }

  if (!wasmBindings) {
    await loadWasmModule(ui.moduleSelect.value);
  }

  ui.startBtn.disabled = true;
  try {
    await engine.start();
    engine.setMicEnabled(ui.micToggle.checked);
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

ui.primaryPot.addEventListener('input', updatePotLabels);
ui.secondaryPot.addEventListener('input', updatePotLabels);

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
