## Context

The Phase 0 spike (`web-module-chain-simulator-spike`) delivered a single-unit WASM simulator with basic pot sliders and no level visualization. Post-spike iteration added:

- Scrolling peak meters for downstream (in) and upstream (out) paths at firmware buffer rate, with multiple history windows and log-scaled display.
- Pot controls that mirror firmware `potsUpdate`: time-scaled EMA at roughly one poll per I2S buffer period, with UI labels showing smoothed values.

This behavior exists in the simulator host but is not yet in the living `module-chain-simulator` spec. Planning notes originally deferred pot EMA to Phase 2; this change pulls that fidelity into the Phase 0 capability definition alongside new metering requirements.

## Goals / Non-Goals

**Goals:**

- Capture level metering and pot smoothing as durable, testable requirements in `module-chain-simulator`.
- Align apply tasks with existing host implementation where it already satisfies the contract.
- Update simulator README so operators and Phase 1 workers see accurate control/metering behavior.

**Non-Goals:**

- Multi-unit chain UI, loopback toggle, or module catalog expansion.
- Merger ring-buffer timing, startup mute, or Web Audio quantum bridging (remaining Phase 2 items).
- Changing on-device firmware pot or I2S behavior.
- Automated visual regression tests for canvas rendering.

## Decisions

### Level peaks at firmware buffer rate

**Decision:** Record one peak per path per firmware buffer period (downstream input peak from capture path; upstream output peak from processed int16 buffer), not at display refresh rate.

**Rationale:** Matches when module processing runs and avoids false gaps or jitter from decoupled sampling timers.

**Alternatives considered:** Fixed UI sample rate (rejected — caused gaps and horizontal jitter when misaligned with audio callbacks).

### Pixel-committed scroll buffer for display

**Decision:** Advance meter columns on peak arrival; redraw at a lower fixed rate (e.g. ~30 Hz) without recomputing column boundaries each frame.

**Rationale:** Stable horizontal scrolling; display refresh decoupled from peak cadence.

**Alternatives considered:** Re-bin history every redraw (rejected — visible jitter).

### Pot smoothing mirrors firmware EMA

**Decision:** Use the same time-invariant EMA alpha model as `src/input/pots.h` (`EMA_ALPHA` per millisecond, effective alpha from elapsed ms between polls). Poll interval aligned to one buffer period at 44.1 kHz / 512 samples.

**Rationale:** Control feel and module response match hardware; modules already read `smoothed` fields.

**Alternatives considered:** Direct write to `smoothed` (spike shortcut — rejected for this change).

### UI shows smoothed value, slider shows raw target

**Decision:** Range input position is the simulated raw reading; numeric label beside each slider shows smoothed value after each poll step.

**Rationale:** Operator sees what modules actually consume; slider still moves instantly while smoothed value catches up.

### Drag-time target sync

**Decision:** While pointer is down on a slider, continuously sync raw target from slider value (e.g. animation frame loop); EMA stepping remains on the buffer-period poller only.

**Rationale:** Prevents “updates only on release” without double-advancing EMA per frame.

## Risks / Trade-offs

- **[Risk] Meter rendering is host-specific (canvas)** → Spec states observable metering behavior only; implementation may change graphics API in Phase 1.
- **[Risk] README / spec drift if apply only archives** → Task row to align `sim/README.md` with reconciled spec.
- **[Risk] Purpose paragraph still TBD in living spec** → Apply task to replace placeholder Purpose text when archiving.

## Migration Plan

1. Archive delta into `openspec/specs/module-chain-simulator/spec.md`.
2. Verify existing host code against new scenarios (manual browser check).
3. Update `sim/README.md` pot and metering sections.

No firmware migration. No WASM harness API changes.

## Open Questions

- None blocking — implicit single-unit loopback remains out of scope for this change (delay audibility already documented in README, not part of metering/pot fidelity).
