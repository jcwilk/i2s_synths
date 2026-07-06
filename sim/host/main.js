import { SimAudioEngine } from './audio-engine.js';
import {
  ChainScheduler,
  MAX_PROCESSING_UNITS,
  MERGER_STRESS_UNDERRUN,
  MERGER_STRESS_OVERRUN,
  MERGER_STRESS_REV_OVERRUN,
  HARDWARE_STRESS_UNDERRUN,
} from './chain-scheduler.js';
import { POT_POLL_MS, applyPotStep } from './pot-simulator.js';
import {
  LevelGraph,
  LevelGraphSampler,
  LEVEL_GRAPH_CHANNELS,
  LEVEL_GRAPH_SPEEDS,
} from './level-graph.js';
import { BridgeClient } from './bridge-client.js';
import { HardwareSlotAdapter } from './hardware-slot-adapter.js';
import { SAMPLE_RATE } from '../shared/audio-constants.js';

const MODULES = {
  passthrough: { label: 'Passthrough', artifact: '../wasm/out/passthrough.js' },
  delay: { label: 'Delay', artifact: '../wasm/out/delay.js' },
  merger: { label: 'Merger', artifact: '../wasm/out/merger.js' },
  debug_tone: { label: 'Debug Tone', artifact: '../wasm/out/debug_tone.js' },
  cutoff: { label: 'Cutoff', artifact: '../wasm/out/cutoff.js' },
};

const MODULE_KEYS = Object.keys(MODULES);

const ui = {
  unitsContainer: document.getElementById('units-container'),
  addUnitBtn: document.getElementById('add-unit-btn'),
  unitCountHint: document.getElementById('unit-count-hint'),
  micToggle: document.getElementById('mic-toggle'),
  startBtn: document.getElementById('start-btn'),
  status: document.getElementById('status'),
  bridgeUrlInput: document.getElementById('bridge-url'),
};

const DEFAULT_BRIDGE_URL = 'ws://localhost:8765';
const bridgeClient = new BridgeClient(
  localStorage.getItem('bridgeUrl') ?? DEFAULT_BRIDGE_URL,
);
const hardwareAdapter = new HardwareSlotAdapter(bridgeClient);

const engine = new SimAudioEngine();
const scheduler = new ChainScheduler();
scheduler.setHardwareAdapter(hardwareAdapter);
engine.setChainScheduler(scheduler);

engine.onStatus = (msg) => {
  ui.status.textContent = msg;
};

/** @type {ProcessingUnit[]} */
const units = [];

let potPollTimer = null;
let lastPotPollMs = 0;
let potDragRafId = null;
let potDragPointers = 0;
let rebuildInProgress = false;
/** @type {ProcessingUnit | null} */
let draggedUnit = null;

/**
 * Emscripten MODULARIZE builds expose createSimModule.
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
      const f = globalThis.createSimModule;
      if (typeof f !== 'function') {
        reject(new Error('createSimModule not found after script load'));
        return;
      }
      resolve(f);
    };
    script.onerror = () => reject(new Error(`Failed to load ${bustedUrl}`));
    document.head.appendChild(script);
  });
}

async function createWasmBindings(moduleKey) {
  const spec = MODULES[moduleKey];
  const artifactUrl = new URL(spec.artifact, import.meta.url).href;
  const factory = await loadEmscriptenFactory(artifactUrl, moduleKey);
  const module = await factory({
    locateFile: (path) => new URL(`../wasm/out/${path}`, import.meta.url).href,
  });

  module._sim_setup();

  return {
    module,
    heap16: module.HEAP16,
    heapF32: module.HEAPF32,
    ptrs: {
      upstreamInPtr: module._sim_get_upstream_in(),
      upstreamOutPtr: module._sim_get_upstream_out(),
      downstreamInPtr: module._sim_get_downstream_in(),
      downstreamOutPtr: module._sim_get_downstream_out(),
    },
    potsPtr: module._sim_get_pots(),
    bufferLen: module._sim_get_buffer_len(),
    moduleKey,
  };
}

function potSliderTo01(slider) {
  return Number(slider.value) / 100;
}

function syncBindingsToScheduler() {
  scheduler.setSlots(units.map((u) => ({
    bindings: u.hardwareDesignated ? null : u.bindings,
    hardwareDesignated: u.hardwareDesignated,
    hardwareConnected: u.hardwareConnected,
  })));
  const rightmost = units[units.length - 1];
  scheduler.setLoopbackEnabled(rightmost?.loopbackEnabled ?? false);
}

function hardwareDesignatedUnit() {
  return units.find((u) => u.hardwareDesignated) ?? null;
}

function updateHardwareToggleStates() {
  const designated = hardwareDesignatedUnit();
  for (const unit of units) {
    const canDesignate = !designated || designated === unit;
    unit.hardwareToggle.disabled = !canDesignate && !unit.hardwareDesignated;
  }
}

function refreshBridgeStatusChips() {
  for (const unit of units) {
    unit.updateHardwareStatus();
  }
}

bridgeClient.onStatus = () => {
  refreshBridgeStatusChips();
};

bridgeClient.onError = (message) => {
  ui.status.textContent = `Bridge: ${message}`;
};

if (ui.bridgeUrlInput) {
  ui.bridgeUrlInput.value = bridgeClient.url;
  ui.bridgeUrlInput.addEventListener('change', () => {
    const url = ui.bridgeUrlInput.value.trim() || DEFAULT_BRIDGE_URL;
    bridgeClient.url = url;
    localStorage.setItem('bridgeUrl', url);
    bridgeClient.disconnect();
    refreshBridgeStatusChips();
  });
}

function updateUnitCountHint() {
  ui.unitCountHint.textContent = `${units.length} / ${MAX_PROCESSING_UNITS} processing units`;
  ui.addUnitBtn.disabled = units.length >= MAX_PROCESSING_UNITS;
}

function updateLoopbackVisibility() {
  for (let i = 0; i < units.length; i++) {
    const isRightmost = i === units.length - 1;
    units[i].card.classList.toggle('is-rightmost', isRightmost);
    units[i].loopbackRow.hidden = !isRightmost;
    if (!isRightmost) {
      units[i].loopbackEnabled = false;
      units[i].loopbackCheckbox.checked = false;
    }
  }
  syncBindingsToScheduler();
}

function refreshUnitLabels() {
  for (let i = 0; i < units.length; i++) {
    const unit = units[i];
    unit.index = i;
    unit.titleEl.textContent = `Unit ${i + 1}`;
    unit.dragHandle.setAttribute('aria-label', `Drag to reorder unit ${i + 1}`);
    unit.deleteBtn.setAttribute('aria-label', `Delete unit ${i + 1}`);
    const loopbackId = `loopback-u${i}`;
    unit.loopbackCheckbox.id = loopbackId;
    unit.loopbackLabel.setAttribute('for', loopbackId);
    const canvases = unit.levelGraphsContainer.querySelectorAll('.level-graph');
    for (let g = 0; g < canvases.length && g < LEVEL_GRAPH_SPEEDS.length; g++) {
      canvases[g].setAttribute(
        'aria-label',
        `Unit ${i + 1} level graph ${LEVEL_GRAPH_SPEEDS[g].title}, blue in orange out`,
      );
    }
  }
}

function refreshDomOrder() {
  for (const unit of units) {
    ui.unitsContainer.appendChild(unit.card);
  }
}

function updateDeleteButtonStates() {
  const blockSoleDelete = engine.running && units.length <= 1;
  for (const unit of units) {
    unit.deleteBtn.disabled = blockSoleDelete;
  }
}

async function rebuildAudioIfRunning() {
  syncBindingsToScheduler();

  if (!engine.running) {
    updateDeleteButtonStates();
    return;
  }

  if (rebuildInProgress) {
    return;
  }

  rebuildInProgress = true;
  const micWasEnabled = ui.micToggle.checked;
  ui.status.textContent = 'Restarting audio…';

  try {
    for (const unit of units) {
      unit.levelSampler.stop();
      unit.clearLevelGraphs();
    }
    await engine.stop();
    syncBindingsToScheduler();
    scheduler.resetPathDelayState();

    await engine.start();
    engine.setMicEnabled(micWasEnabled);
    for (const unit of units) {
      unit.levelSampler.start();
    }
    ui.startBtn.textContent = 'Stop audio';
    ui.status.textContent = `Audio running at ${SAMPLE_RATE} Hz`;
  } catch (err) {
    ui.startBtn.textContent = 'Start audio';
    ui.status.textContent =
      `Audio restart failed: ${err.message} — tap Start to try again`;
  } finally {
    rebuildInProgress = false;
    updateDeleteButtonStates();
  }
}

class ProcessingUnit {
  constructor(index, moduleKey = 'delay') {
    this.index = index;
    this.moduleKey = moduleKey;
    this.bindings = null;
    this.loopbackEnabled = false;
    this.hardwareDesignated = false;
    this.hardwareConnected = false;
    this.potTargets = { primary: 0.5, secondary: 0 };
    this.levelGraphs = [];
    this.levelSampler = null;
    this.stressTimer = null;

    this.card = document.createElement('div');
    this.card.className = 'chain-card unit-card';
    this.card.dataset.unitKey = String(index);
    this.card.innerHTML = `
      <div class="card-header row">
        <span class="drag-handle" role="button" tabindex="0" aria-label="Drag to reorder unit ${index + 1}" title="Drag to reorder">⠿</span>
        <div class="card-title">Unit ${index + 1}</div>
        <button type="button" class="delete-btn" aria-label="Delete unit ${index + 1}" title="Delete unit">×</button>
      </div>
      <div class="card-controls">
        <label>Module type</label>
        <select class="module-select"></select>
        <div class="hardware-row row">
          <input class="hardware-toggle" type="checkbox" id="hw-u${index}" />
          <label class="hardware-label" for="hw-u${index}" style="margin:0;">Hardware slot</label>
        </div>
        <div class="hardware-controls" hidden>
          <div class="hw-status-chips row" aria-live="polite"></div>
          <div class="row" style="margin-top:0.35rem; gap:0.35rem;">
            <button type="button" class="hw-connect-btn">Connect</button>
            <button type="button" class="hw-disconnect-btn" hidden>Disconnect</button>
          </div>
          <p class="hw-pot-hint hint" hidden>Device pots — physical controls on module</p>
        </div>
        <div class="wasm-pots">
        <label>Primary pot</label>
        <div class="row">
          <input class="primary-pot" type="range" min="0" max="100" value="50" />
          <span class="primary-value">50%</span>
        </div>
        <label>Secondary pot</label>
        <div class="row">
          <input class="secondary-pot" type="range" min="0" max="100" value="0" />
          <span class="secondary-value">0%</span>
        </div>
        </div>
        <div class="loopback-slot">
          <div class="loopback-row row">
            <input class="loopback-toggle" type="checkbox" id="loopback-u${index}" />
            <label class="loopback-label" for="loopback-u${index}" style="margin:0;">Loopback</label>
          </div>
        </div>
        <div class="stress-badge" hidden aria-live="polite"></div>
        <p class="module-info" hidden></p>
      </div>
      <div class="level-graphs"></div>
    `;

    this.titleEl = this.card.querySelector('.card-title');
    this.dragHandle = this.card.querySelector('.drag-handle');
    this.deleteBtn = this.card.querySelector('.delete-btn');
    this.moduleSelect = this.card.querySelector('.module-select');
    this.hardwareToggle = this.card.querySelector('.hardware-toggle');
    this.hardwareLabel = this.card.querySelector('.hardware-label');
    this.hardwareControls = this.card.querySelector('.hardware-controls');
    this.hardwareStatusChips = this.card.querySelector('.hw-status-chips');
    this.hwConnectBtn = this.card.querySelector('.hw-connect-btn');
    this.hwDisconnectBtn = this.card.querySelector('.hw-disconnect-btn');
    this.hwPotHint = this.card.querySelector('.hw-pot-hint');
    this.wasmPots = this.card.querySelector('.wasm-pots');
    this.primaryPot = this.card.querySelector('.primary-pot');
    this.secondaryPot = this.card.querySelector('.secondary-pot');
    this.primaryValue = this.card.querySelector('.primary-value');
    this.secondaryValue = this.card.querySelector('.secondary-value');
    this.loopbackRow = this.card.querySelector('.loopback-row');
    this.loopbackCheckbox = this.card.querySelector('.loopback-toggle');
    this.loopbackLabel = this.card.querySelector('.loopback-label');
    this.stressBadge = this.card.querySelector('.stress-badge');
    this.moduleInfo = this.card.querySelector('.module-info');
    this.levelGraphsContainer = this.card.querySelector('.level-graphs');

    for (const speed of LEVEL_GRAPH_SPEEDS) {
      const canvas = document.createElement('canvas');
      canvas.className = 'level-graph';
      canvas.id = `level-graph-u${index}-${speed.id}`;
      canvas.setAttribute(
        'aria-label',
        `Unit ${index + 1} level graph ${speed.title}, blue in orange out`,
      );
      this.levelGraphsContainer.appendChild(canvas);
      this.levelGraphs.push(
        new LevelGraph(canvas, {
          durationSec: speed.durationSec,
          title: speed.title,
          channels: LEVEL_GRAPH_CHANNELS,
        }),
      );
    }
    this.levelSampler = new LevelGraphSampler(this.levelGraphs);

    for (const key of MODULE_KEYS) {
      const opt = document.createElement('option');
      opt.value = key;
      opt.textContent = MODULES[key].label;
      if (key === moduleKey) {
        opt.selected = true;
      }
      this.moduleSelect.appendChild(opt);
    }

    this.moduleSelect.addEventListener('change', () => {
      if (this.hardwareConnected) {
        this.moduleSelect.value = this.moduleKey;
        return;
      }
      this.changeModuleType(this.moduleSelect.value).catch((err) => {
        ui.status.textContent = `Module load failed: ${err.message}`;
      });
    });

    this.hardwareToggle.addEventListener('change', () => {
      this.setHardwareDesignated(this.hardwareToggle.checked).catch((err) => {
        ui.status.textContent = `Hardware designation failed: ${err.message}`;
      });
    });

    this.hwConnectBtn.addEventListener('click', () => {
      this.connectHardware().catch((err) => {
        ui.status.textContent = `Hardware connect failed: ${err.message}`;
      });
    });

    this.hwDisconnectBtn.addEventListener('click', () => {
      this.disconnectHardware().catch((err) => {
        ui.status.textContent = `Hardware disconnect failed: ${err.message}`;
      });
    });

    this.loopbackCheckbox.addEventListener('change', () => {
      this.loopbackEnabled = this.loopbackCheckbox.checked;
      syncBindingsToScheduler();
      rebuildAudioIfRunning().catch((err) => {
        ui.status.textContent = `Audio rebuild failed: ${err.message}`;
      });
    });

    this.deleteBtn.addEventListener('click', () => {
      deleteUnit(this).catch((err) => {
        ui.status.textContent = `Delete failed: ${err.message}`;
      });
    });

    setupUnitDragDrop(this);

    bindPotSlider(this.primaryPot, () => this.syncPotTargets());
    bindPotSlider(this.secondaryPot, () => this.syncPotTargets());
  }

  syncPotTargets() {
    this.potTargets.primary = potSliderTo01(this.primaryPot);
    this.potTargets.secondary = potSliderTo01(this.secondaryPot);
  }

  resetPotSmoothedToSliders() {
    if (!this.bindings) {
      return;
    }
    const { heapF32, potsPtr } = this.bindings;
    const base = potsPtr >> 2;
    const primary = potSliderTo01(this.primaryPot);
    const secondary = potSliderTo01(this.secondaryPot);
    heapF32[base + 1] = primary;
    heapF32[base + 2] = primary;
    heapF32[base + 4] = secondary;
    heapF32[base + 5] = secondary;
    this.potTargets = { primary, secondary };
    this.updatePotValueLabels();
  }

  updatePotValueLabels() {
    if (!this.bindings) {
      return;
    }
    const { heapF32, potsPtr } = this.bindings;
    const base = potsPtr >> 2;
    this.primaryValue.textContent = `${(heapF32[base + 2] * 100).toFixed(1)}%`;
    this.secondaryValue.textContent = `${(heapF32[base + 5] * 100).toFixed(1)}%`;
  }

  tickPots(deltaMs) {
    if (!this.bindings) {
      return;
    }
    const { heapF32, potsPtr } = this.bindings;
    const base = potsPtr >> 2;
    applyPotStep(heapF32, base + 1, base + 2, this.potTargets.primary, deltaMs);
    applyPotStep(heapF32, base + 4, base + 5, this.potTargets.secondary, deltaMs);
    this.updatePotValueLabels();
  }

  updateModuleInfo() {
    if (!this.bindings) {
      this.moduleInfo.hidden = true;
      return;
    }
    const { module, moduleKey } = this.bindings;
    if (moduleKey === 'delay' && module._sim_get_delay_buffer_frames) {
      const frames = module._sim_get_delay_buffer_frames();
      const ms = ((frames * 1000) / SAMPLE_RATE).toFixed(1);
      this.moduleInfo.textContent = `Delay ring: ${frames} frames (${ms} ms)`;
      this.moduleInfo.hidden = false;
    } else {
      this.moduleInfo.hidden = true;
    }
  }

  showMergerStress(stressFlags) {
    if (!this.stressBadge || !stressFlags) {
      return;
    }
    const isHwUnderrun = (stressFlags & HARDWARE_STRESS_UNDERRUN) !== 0;
    const isUnderrun = (stressFlags & MERGER_STRESS_UNDERRUN) !== 0 || isHwUnderrun;
    const isOverrun =
      (stressFlags & MERGER_STRESS_OVERRUN) !== 0 ||
      (stressFlags & MERGER_STRESS_REV_OVERRUN) !== 0;
    if (!isUnderrun && !isOverrun) {
      return;
    }
    const kind = isOverrun ? 'overrun' : 'underrun';
    this.stressBadge.hidden = false;
    this.stressBadge.dataset.stress = kind;
    this.stressBadge.textContent = kind === 'overrun' ? 'Overrun' : 'Underrun';
    if (this.stressTimer !== null) {
      clearTimeout(this.stressTimer);
    }
    this.stressTimer = setTimeout(() => {
      this.stressBadge.hidden = true;
      this.stressTimer = null;
    }, 800);
  }

  async loadModule(moduleKey) {
    if (this.hardwareDesignated) {
      this.moduleKey = moduleKey;
      syncBindingsToScheduler();
      return;
    }
    this.moduleKey = moduleKey;
    this.bindings = await createWasmBindings(moduleKey);
    this.resetPotSmoothedToSliders();
    this.updateModuleInfo();
    syncBindingsToScheduler();
  }

  updateHardwareStatus() {
    if (!this.hardwareDesignated) {
      return;
    }
    const s = bridgeClient.statusSnapshot();
    const chips = [];
    if (!s.bridgeReachable) {
      chips.push('<span class="hw-chip hw-offline">Bridge offline</span>');
    } else if (!s.deviceAttached) {
      chips.push('<span class="hw-chip">Bridge online</span>');
      chips.push('<span class="hw-chip hw-warn">No device</span>');
    } else {
      chips.push('<span class="hw-chip">Device attached</span>');
    }
    if (this.hardwareConnected && s.sessionActive) {
      chips.push('<span class="hw-chip hw-active">Session active</span>');
    }
    if (s.firmwareModuleKind && this.moduleKey !== s.firmwareModuleKind) {
      chips.push('<span class="hw-chip hw-mismatch">Module mismatch</span>');
    }
    this.hardwareStatusChips.innerHTML = chips.join('');
  }

  refreshHardwareUi() {
    const designated = this.hardwareDesignated;
    this.hardwareControls.hidden = !designated;
    this.card.classList.toggle('is-hardware', designated);
    const connected = this.hardwareConnected;
    this.wasmPots.hidden = designated && connected;
    this.hwPotHint.hidden = !(designated && connected);
    this.primaryPot.disabled = designated && connected;
    this.secondaryPot.disabled = designated && connected;
    this.moduleSelect.disabled = connected;
    this.hwConnectBtn.hidden = connected;
    this.hwDisconnectBtn.hidden = !connected;
    this.updateHardwareStatus();
  }

  async setHardwareDesignated(enabled) {
    if (enabled && hardwareDesignatedUnit() && hardwareDesignatedUnit() !== this) {
      this.hardwareToggle.checked = false;
      return;
    }
    if (this.hardwareConnected) {
      await this.disconnectHardware();
    }
    this.hardwareDesignated = enabled;
    this.hardwareToggle.checked = enabled;
    if (enabled) {
      this.bindings = null;
    } else if (!this.bindings) {
      await this.loadModule(this.moduleKey);
    }
    updateHardwareToggleStates();
    this.refreshHardwareUi();
    syncBindingsToScheduler();
    await rebuildAudioIfRunning();
  }

  async connectHardware() {
    if (!this.hardwareDesignated || this.hardwareConnected) {
      return;
    }
    await bridgeClient.connect();
    const status = await bridgeClient.startSession('pwa');
    if (status.firmwareModuleKind && status.firmwareModuleKind !== this.moduleKey) {
      await bridgeClient.stopSession();
      this.updateHardwareStatus();
      ui.status.textContent =
        `Module mismatch: slot is ${this.moduleKey}, device is ${status.firmwareModuleKind}`;
      return;
    }
    this.hardwareConnected = true;
    hardwareAdapter.start();
    this.refreshHardwareUi();
    syncBindingsToScheduler();
    await rebuildAudioIfRunning();
    ui.status.textContent = `Hardware connected (${status.firmwareModuleKind ?? this.moduleKey})`;
  }

  async disconnectHardware() {
    if (!this.hardwareConnected) {
      return;
    }
    hardwareAdapter.stop();
    await bridgeClient.stopSession();
    this.hardwareConnected = false;
    this.refreshHardwareUi();
    syncBindingsToScheduler();
    await rebuildAudioIfRunning();
  }

  async changeModuleType(moduleKey) {
    if (moduleKey === this.moduleKey && this.bindings) {
      return;
    }
    await this.loadModule(moduleKey);
    await rebuildAudioIfRunning();
    if (!engine.running) {
      ui.status.textContent = `${MODULES[moduleKey].label} ready on unit ${this.index + 1}`;
    }
  }

  clearLevelGraphs() {
    for (const graph of this.levelGraphs) {
      graph.clear();
    }
  }

  resizeLevelGraphs() {
    for (const graph of this.levelGraphs) {
      graph.resize();
    }
  }

  destroy() {
    this.levelSampler.stop();
    for (const graph of this.levelGraphs) {
      graph.destroy();
    }
    this.card.remove();
  }
}

function setupUnitDragDrop(unit) {
  unit.card.draggable = false;

  unit.dragHandle.addEventListener('pointerdown', () => {
    if (unit.hardwareConnected) {
      return;
    }
    unit.card.draggable = true;
  });
  unit.dragHandle.addEventListener('pointerup', () => {
    if (!unit.card.classList.contains('is-dragging')) {
      unit.card.draggable = false;
    }
  });
  unit.dragHandle.addEventListener('pointercancel', () => {
    if (!unit.card.classList.contains('is-dragging')) {
      unit.card.draggable = false;
    }
  });

  unit.card.addEventListener('dragstart', (event) => {
    draggedUnit = unit;
    event.dataTransfer.effectAllowed = 'move';
    event.dataTransfer.setData('text/plain', String(unit.index));
    unit.card.classList.add('is-dragging');
  });

  unit.card.addEventListener('dragend', () => {
    unit.card.draggable = false;
    unit.card.classList.remove('is-dragging');
    draggedUnit = null;
    for (const u of units) {
      u.card.classList.remove('is-drop-target');
    }
  });

  unit.card.addEventListener('dragover', (event) => {
    event.preventDefault();
    event.dataTransfer.dropEffect = 'move';
    if (draggedUnit && draggedUnit !== unit) {
      unit.card.classList.add('is-drop-target');
    }
  });

  unit.card.addEventListener('dragleave', () => {
    unit.card.classList.remove('is-drop-target');
  });

  unit.card.addEventListener('drop', (event) => {
    event.preventDefault();
    unit.card.classList.remove('is-drop-target');
    if (units.some((u) => u.hardwareConnected)) {
      ui.status.textContent = 'Disconnect hardware before reordering';
      return;
    }
    if (!draggedUnit || draggedUnit === unit) {
      return;
    }

    const fromIdx = units.indexOf(draggedUnit);
    const toIdx = units.indexOf(unit);
    if (fromIdx < 0 || toIdx < 0) {
      return;
    }

    units.splice(fromIdx, 1);
    units.splice(toIdx, 0, draggedUnit);

    refreshUnitLabels();
    refreshDomOrder();
    updateLoopbackVisibility();
    rebuildAudioIfRunning().catch((err) => {
      ui.status.textContent = `Reorder rebuild failed: ${err.message}`;
    });
  });
}

async function deleteUnit(unit) {
  const idx = units.indexOf(unit);
  if (idx < 0) {
    return;
  }
  if (engine.running && units.length <= 1) {
    return;
  }

  unit.destroy();
  units.splice(idx, 1);
  refreshUnitLabels();
  refreshDomOrder();
  updateUnitCountHint();
  updateLoopbackVisibility();
  await rebuildAudioIfRunning();
  if (!engine.running) {
    ui.status.textContent =
      units.length === 0
        ? 'All units removed — add a unit to start audio'
        : `Unit removed — ${units.length} processing unit(s) in chain`;
  }
}

function bindPotSlider(slider, onInput) {
  slider.addEventListener('input', onInput);
  slider.addEventListener('change', onInput);
  slider.addEventListener('pointerdown', (event) => {
    event.currentTarget.setPointerCapture?.(event.pointerId);
    potDragPointers += 1;
    startPotDragLoop();
    onInput();
  });
  slider.addEventListener('pointerup', (event) => {
    if (event.currentTarget.hasPointerCapture?.(event.pointerId)) {
      event.currentTarget.releasePointerCapture(event.pointerId);
    }
    potDragPointers = Math.max(0, potDragPointers - 1);
    if (potDragPointers === 0) {
      stopPotDragLoop();
    }
    onInput();
  });
  slider.addEventListener('pointercancel', (event) => {
    if (event.currentTarget.hasPointerCapture?.(event.pointerId)) {
      event.currentTarget.releasePointerCapture(event.pointerId);
    }
    potDragPointers = Math.max(0, potDragPointers - 1);
    if (potDragPointers === 0) {
      stopPotDragLoop();
    }
    onInput();
  });
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
    for (const unit of units) {
      unit.syncPotTargets();
    }
    potDragRafId = requestAnimationFrame(frame);
  };
  potDragRafId = requestAnimationFrame(frame);
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
  for (const unit of units) {
    unit.tickPots(POT_POLL_MS);
  }
  potPollTimer = setInterval(() => {
    const now = performance.now();
    const deltaMs = now - lastPotPollMs;
    lastPotPollMs = now;
    for (const unit of units) {
      unit.tickPots(deltaMs);
    }
  }, POT_POLL_MS);
}

async function addUnit(moduleKey = 'delay') {
  if (units.length >= MAX_PROCESSING_UNITS) {
    return;
  }
  const unit = new ProcessingUnit(units.length, moduleKey);
  units.push(unit);
  ui.unitsContainer.appendChild(unit.card);
  unit.resizeLevelGraphs();
  updateUnitCountHint();
  updateLoopbackVisibility();
  updateDeleteButtonStates();
  await unit.loadModule(moduleKey);
  startPotPoller();
  await rebuildAudioIfRunning();
  if (!engine.running) {
    ui.status.textContent = `${MODULES[moduleKey].label} added — ${units.length} unit(s) in chain`;
  }
}

engine.onLevels = (levels) => {
  for (let i = 0; i < units.length; i++) {
    if (levels[i]) {
      units[i].levelSampler?.feed(levels[i]);
      if (levels[i].stress) {
        units[i].showMergerStress(levels[i].stress);
      }
    }
  }
};

async function onStartClick() {
  if (engine.running) {
    for (const unit of units) {
      unit.levelSampler.stop();
    }
    await engine.stop();
    ui.startBtn.textContent = 'Start audio';
    for (const unit of units) {
      unit.clearLevelGraphs();
    }
    updateDeleteButtonStates();
    return;
  }

  if (units.length === 0) {
    ui.status.textContent = 'Add at least one processing unit';
    return;
  }

  ui.startBtn.disabled = true;
  try {
    syncBindingsToScheduler();
    await engine.start();
    engine.setMicEnabled(ui.micToggle.checked);
    for (const unit of units) {
      unit.levelSampler.start();
    }
    ui.startBtn.textContent = 'Stop audio';
    updateDeleteButtonStates();
    const rightmost = units[units.length - 1];
    if (rightmost && !rightmost.loopbackEnabled) {
      ui.status.textContent +=
        ' — enable loopback on the rightmost unit (or use Debug Tone) to hear effect modules on speakers.';
    }
  } catch (err) {
    ui.status.textContent = `Failed to start audio: ${err.message}`;
  } finally {
    ui.startBtn.disabled = false;
  }
}

ui.micToggle.addEventListener('change', () => {
  engine.setMicEnabled(ui.micToggle.checked);
});

ui.startBtn.addEventListener('click', () => {
  onStartClick().catch((err) => {
    ui.status.textContent = `Audio error: ${err.message}`;
  });
});

ui.addUnitBtn.addEventListener('click', () => {
  addUnit('passthrough').catch((err) => {
    ui.status.textContent = `Add unit failed: ${err.message}`;
  });
});

addUnit('delay')
  .then(() => {
    updateDeleteButtonStates();
    ui.status.textContent = 'Chain ready — gateway plus one unit loaded';
  })
  .catch((err) => {
    ui.status.textContent = `Initial load failed: ${err.message}`;
  });

updateUnitCountHint();
updateDeleteButtonStates();
