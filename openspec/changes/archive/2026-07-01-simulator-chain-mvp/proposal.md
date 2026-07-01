## Why

Phase 0 proved that firmware module logic runs in the browser with real-time audio, pot smoothing, and level metering—but only for a single collapsed virtual unit. Developers still cannot exercise multi-module chain topology, per-slot module selection, or explicit rightmost loopback, which are the primary reasons to build a simulator. Phase 1 from `ai/planning/web-module-chain-simulator.md` delivers the interactive chain MVP so module and chain behavior can be explored without hardware.

## What Changes

- **Replace** the Phase 0 single-unit constraint with a **gateway plus one or more processing units** laid out left-to-right, matching the physical chain mental model.
- Add **chain UI**: fixed gateway I/O card (mic toggle, speaker tap), **Add unit** control, per-unit module type selector and dual pot controls, and a **loopback toggle on the rightmost unit only**.
- Add **chain audio wiring** per planning: mic or silence at the gateway downstream input, downstream outputs feeding the next unit's downstream input, upstream inputs fed from the next unit's upstream output, optional loopback from the rightmost downstream output into that unit's upstream input, and speaker output from the gateway upstream path.
- Extend WASM coverage to **all firmware module types** offered in planning (passthrough, delay, merger, cutoff, debug tone): one compiled instance per occupied chain slot, with state reset when the operator changes that slot's module type.
- Enforce a **soft maximum chain length** (default eight processing units beyond the gateway) to keep heap and CPU use bounded.
- Update simulator documentation and verification for the chain MVP.

**Non-goals (this change):** merger ring-buffer timing fidelity, startup mute policy, npm/CI developer-experience polish (Phase 3), gateway as a selectable effect module, hardware A/B regression, preset save/load, or on-device firmware changes.

## Capabilities

### New Capabilities

<!-- None — extending existing module-chain-simulator domain -->

### Modified Capabilities

- `module-chain-simulator`: supersede Phase 0 single-unit scope with gateway + multi-unit chain layout, chain wiring, per-slot module catalog, rightmost loopback control, per-unit WASM instances, and chain-length limits; clarify gateway-owned I/O and per-unit level metering in a multi-unit layout.

## Impact

- **Specs:** Delta to `openspec/specs/module-chain-simulator/spec.md` (ADDED chain requirements; REMOVED Phase 0 single-unit requirement; MODIFIED module catalog, microphone/speaker, and level-metering requirements where multi-unit layout changes observable behavior).
- **Repo:** `sim/` host (chain engine, UI, WASM build script), extended WASM artifacts for additional module types, README and headless verification updates.
- **Dependencies:** Existing Emscripten toolchain and browser Web Audio; no new runtime dependencies required for MVP (static dev server remains acceptable).
- **Prior work:** Builds on archived Phase 0 spike and simulator metering/pot-fidelity changes; retains firmware-equivalent pot EMA and level graphs rather than reverting to instant pot injection.
