## Context

The module chain simulator (chain MVP) renders a fixed gateway card plus appendable processing unit cards laid out in a responsive grid the UI treats as chain panes. Operators can add units and change module types, but cannot delete or reorder units without reloading the page. Module type changes currently re-initialize only that unit's WASM instance while the Web Audio graph keeps running. The user request adds delete and drag-reorder (with an explicit grab handle for touch) and requires tearing down and rebuilding the audio stream whenever chain topology or unit configuration changes—except pot slider moves, which should remain live.

Current touch points: `sim/host/main.js` (unit cards, scheduler sync), `sim/host/audio-engine.js` (start/stop lifecycle), `sim/host/chain-scheduler.js` (slot ordering), `sim/host/index.html` (layout/styles).

## Goals / Non-Goals

**Goals:**

- Let operators remove any processing unit when more than one unit exists (or when audio is stopped).
- Let operators reorder units via pointer drag initiated from a dedicated handle control on each unit card.
- On touch devices, restrict drag initiation to the handle so vertical scrolling the page does not accidentally reorder units.
- After delete or reorder, refresh DOM order, unit indices/labels, scheduler slot order, rightmost loopback visibility, and level-meter associations.
- Define a single "structural reconfiguration" category covering add, delete, reorder, module type change, and loopback toggle; when audio is running, stop the engine, clear playback/meter state, re-sync the chain, and auto-restart (preserving mic toggle and per-unit control positions).
- Keep pot slider adjustments live without audio restart during playback.

**Non-Goals:**

- Undo/redo, preset persistence, gateway card manipulation, changing `MAX_PROCESSING_UNITS`, firmware or WASM build changes, animated drag previews beyond minimal affordance, or keyboard-only reorder shortcuts (may follow later).

## Decisions

### 1. Structural reconfiguration triggers full audio restart

**Choice:** Centralize a `rebuildAudioIfRunning()` helper invoked from add, delete, reorder, module-type change, and loopback toggle handlers.

**Rationale:** Chain wiring and WASM instances depend on slot order; stale buffers in the playback worklet and level graphs can produce glitches after topology changes. Full `audio-engine.stop()` → re-sync scheduler → `audio-engine.start()` matches user expectation of "clear and rebuild."

**Alternative considered:** Hot-swap scheduler bindings without stopping AudioContext—rejected because playback queue and meter samplers would need careful partial reset and prior bugs favored full teardown.

### 2. Pot sliders excluded from rebuild

**Choice:** Pot `input`/`change` handlers continue to update targets and smoothed WASM state only; they do not call the rebuild helper.

**Rationale:** Explicit user constraint; pot smoothing already works incrementally per buffer period.

### 3. Delete control on each unit card

**Choice:** A delete button (or icon button with accessible label) in the unit card header row, disabled when only one unit remains while audio is running (operator must stop audio or add another unit first). When audio is stopped, allow deleting down to zero units.

**Rationale:** Matches common card UI patterns; prevents leaving the chain in a no-unit state while audio expects at least one slot (current start guard).

### 4. HTML5 drag-and-drop with handle-only drag start

**Choice:** Use native `draggable` on unit cards but set `draggable=false` by default; on pointer down on the handle, enable drag for that gesture. For touch, use pointer events on the handle with `touch-action: none` on the handle element.

**Alternative considered:** Full pointer-based reorder library—rejected to avoid new dependencies; native DnD plus handle is sufficient for a small N of units.

### 5. Reorder updates in-memory array and DOM together

**Choice:** Maintain `units[]` as source of truth; on drop, splice array, re-append cards in order, renumber displayed unit labels (Unit 1, Unit 2, …), call `syncBindingsToScheduler()`, then structural rebuild.

**Rationale:** Scheduler already maps slots by array index; label refresh avoids stale "Unit 3" after deleting unit 2.

### 6. Level graphs and WASM instances on delete

**Choice:** Reuse existing `ProcessingUnit.destroy()` on delete; on reorder, keep instances attached to unit objects (identity follows the unit, not DOM position).

**Rationale:** WASM state should travel with the unit when reordered; only wiring order changes.

## Risks / Trade-offs

- **[Brief audio gap on structural edit]** → Acceptable; status line can show "Restarting audio…" during async stop/start.
- **[Auto-restart may fail (mic permission, autoplay)]** → Surface error in status; leave chain config applied but audio stopped; operator taps Start again.
- **[Native DnD on mobile browsers varies]** → Handle uses pointer capture and `touch-action: none`; manual QA on iOS Safari and Android Chrome listed in tasks.
- **[Delete last unit while running]** → Disabled by UI guard aligned with existing "at least one unit to start" rule.

## Migration Plan

No data migration. Deploy updated static host assets; operators refresh the simulator page. No WASM rebuild required unless verification scripts reference DOM structure.

## Open Questions

- None blocking apply; confirm during review whether loopback toggle should be structural (included per user "any reconfiguration aside from sliders").
