## Why

The chain MVP deliberately approximated merger cross-path timing: both upstream and downstream inputs for each processing unit are resolved in the same firmware buffer period, so the merger module's internal capture rings fill and drain under different conditions than on hardware. Operators hear merge latency, feedback, and underrun recovery that do not match a physical module chain, which undermines the simulator's purpose for tuning merger-heavy patches.

Phase 2 planning called out merger ring-buffer decoupling as the highest-impact fidelity gap after pot EMA (already shipped). No OpenSpec change has captured that work yet; this proposal closes that gap for the browser chain host.

## What Changes

- **Decouple per-unit upstream and downstream input timing** in the chain host so cross-path modules receive path inputs with the same relative timing as physical I2S upstream and downstream streams, including one-buffer delayed handoff between adjacent units where the chain MVP already models downstream propagation.
- **Align merger audibility and dynamics** with firmware merger-module behavior when merger occupies one or more chain slots: delayed upstream blend, optional downstream-to-upstream forwarding, gentle peak limiting, underrun silence substitution with stabilization, and overrun drop-oldest policy (module logic already compiled from firmware; host wiring is the fix).
- **Add observable merger stress signals** in the simulator UI when underrun or overrun conditions occur (equivalent to firmware operator feedback, without requiring hardware indicators).
- **Extend verification and documentation** with merger-focused chain checks and an updated known-deltas table.

**Non-goals:** on-device firmware or merger-module algorithm changes unless apply discovers spec drift; startup mute policy; Web Audio render-quantum buffering; npm/CI tooling; preset save/load; hardware A/B regression harness.

## Capabilities

### New Capabilities

<!-- None -->

### Modified Capabilities

- `module-chain-simulator`: add dual-path input timing decoupling and merger parity requirements; document remaining intentional sim-vs-hardware deltas after this change.

## Impact

- **Specs:** Delta to `openspec/specs/module-chain-simulator/spec.md` (ADDED timing-decoupling and merger-parity requirements; MODIFIED multi-unit chain wiring where path timing is currently underspecified).
- **Repo:** `sim/host/` chain scheduler and related audio routing; optional unit-level status for merger stress events; `sim/README.md` limitations table; `sim/verify-wasm.mjs` or companion chain-routing checks as tasks specify.
- **Dependencies:** Existing WASM merger artifact (same `merger.h` sources); no new runtime packages.
- **Related work:** Builds on archived chain MVP and pot/metering fidelity changes; complements in-flight `simulator-pane-delete-reorder` (UI only — no conflict on scheduler timing). `delay-module-spec-and-implementation` defines firmware `merger-module` behavior but does not address simulator host wiring.
