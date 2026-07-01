## Context

The module chain simulator under `sim/` currently hosts one virtual unit with passthrough and delay WASM builds, firmware-equivalent pot EMA, dual-path level metering, and an implicit single-buffer loopback so effect modules remain audible on the speaker tap. Living spec `module-chain-simulator` still describes Phase 0 single-unit scope for the topology requirement.

Planning Phase 1 (`ai/planning/web-module-chain-simulator.md`) defines the chain MVP: gateway I/O card, appendable processing units, full module dropdown per unit, explicit rightmost loopback, and left-to-right wiring that mirrors firmware chain semantics. Phases 2–3 (merger timing fidelity, npm/CI DX) remain deferred.

## Goals / Non-Goals

**Goals:**

- Horizontal chain UI: gateway at the left, processing units appended to the right.
- Per-unit module type selection across all five firmware module kinds.
- One WASM instance per occupied slot; re-initialize module state when type changes.
- Chain buffer routing per planning wiring rules at 44.1 kHz / 512-sample contract.
- Rightmost-unit loopback toggle (off by default).
- Soft cap of eight processing units (configurable constant in host).
- Extend `build-wasm.sh` and headless verification for merger, cutoff, and debug_tone artifacts.
- Update `sim/README.md` for chain operation and acceptance matrix.

**Non-Goals:**

- Merger upstream/downstream ring decoupling fidelity (Phase 2).
- Startup mute, buffer-quantum ring redesign beyond existing accumulator (Phase 2).
- `package.json`, `npm run dev`, CI WASM matrix (Phase 3).
- Removing units from the chain (planning specifies add-only for MVP).
- Gateway hosting a selectable effect module (default I/O shim only).
- Changing on-device firmware or module-architecture specs.

## Decisions

### 1. Slot model: gateway index 0, units 1..N

Treat the gateway as a non-WASM I/O shim at index 0. Processing units occupy slots 1 through N. Audio wiring follows planning:

| Signal | Source |
|--------|--------|
| `ds_in[0]` | Microphone when enabled; silence otherwise |
| `ds_in[i+1]` | `ds_out[i]` for `i >= 0` |
| `us_in[i]` (interior) | `us_out[i+1]` |
| `us_in[last]` | `ds_out[last]` when loopback on; silence when off |
| Speakers | `us_out[0]` (gateway upstream output) |

Processing order per buffer period: for each unit from right to left, run upstream then downstream (matching firmware per-module call order within the chain sweep). Gateway does not invoke WASM.

**Alternatives:** Collapse gateway into unit 0 WASM passthrough (rejected: planning default is I/O-only gateway).

### 2. Reuse one WASM artifact per module type, one instance per slot

Keep the Phase 0 pattern: separate Emscripten build per `ACTIVE_MODULE`, but instantiate independently per occupied slot so static globals stay isolated. Slot holds `{ moduleType, wasmInstance, potState }`.

On module type change: tear down prior instance, load matching factory, call setup, reset pot smoothed state to current slider positions.

**Alternatives:** Single registry WASM (Phase 3 option; requires instance-state refactor).

### 3. Extend `build-wasm.sh` for merger (2), cutoff (4), debug_tone (3)

Add three build variants mirroring `MODULE_*` IDs in `constants.h`. Export any module-specific introspection symbols only where needed (delay already exports ring size).

### 4. Chain engine refactor in host

Extract multi-slot buffer routing from `audio-engine.js` / worklet into a small chain scheduler that:

- Maintains per-slot downstream/upstream buffers.
- Invokes each slot's WASM upstream/downstream exports with that slot's `DualPotsState`.
- Feeds peak samples to per-unit level graph instances (reuse `level-graph.js` per card).

Retain existing 512-sample accumulator bridging Web Audio quantum to firmware buffer size.

### 5. UI layout

Plain HTML/CSS horizontal flex row:

- **Gateway card:** mic toggle, start/stop audio, no module selector.
- **Unit cards:** module dropdown, primary/secondary sliders with smoothed numeric labels, In/Out level graphs (three windows as today).
- **Add unit** button after the rightmost card (disabled at cap).
- **Loopback** toggle rendered only on the rightmost unit card.

Initial state: gateway + one unit (delay or passthrough) to avoid empty-chain edge cases.

### 6. Pot EMA and metering carry forward

Do not revert to instant `smoothed` injection. Each unit card owns a `PotSimulator` poll loop (shared timing) writing that slot's WASM pot struct. Level graphs bind to that slot's per-buffer peaks.

### 7. Merger MVP approximation

Merger WASM runs with both paths fed from the same buffer period (no inter-path ring delay). Document as known sim-vs-hardware delta; Phase 2 addresses timing.

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| CPU load with many WASM units | Soft cap at eight units; document performance expectations |
| WASM memory × N instances | Monitor total linear memory; delay rings are largest consumer |
| UI complexity without component framework | Keep vanilla JS; horizontal card template cloned per unit |
| Chain wiring bugs vs firmware | Headless routing test with mocked WASM; manual acceptance matrix in README |
| debug_tone / merger side effects | Existing shims (noop neopixel, etc.) |

## Migration Plan

Incremental evolution of `sim/host/`. Phase 0 single-unit path is replaced by chain MVP in place; no firmware migration. Rollback = revert `sim/` changes and archive rollback of spec delta.

## Open Questions

- **Initial unit count on first load:** Default one processing unit (recommended) vs empty chain requiring Add unit before audio.
- **Unit removal:** Deferred; MVP is add-only per planning. Revisit if UX needs shrink.
