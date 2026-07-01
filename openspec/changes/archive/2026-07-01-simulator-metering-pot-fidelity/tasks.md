## Required for this change

## 1. Spec reconciliation prep

- [x] 1.1 Replace the TBD **Purpose** paragraph in `openspec/specs/module-chain-simulator/spec.md` during archive with a one-paragraph summary of the Phase 0 browser simulator (single unit, WASM modules, controls, metering)

## 2. Level metering

- [x] 2.1 Verify dual-path peak history (downstream in, upstream out) advances during playback without restarting audio
- [x] 2.2 Verify peak samples arrive at firmware buffer cadence (one per buffer period per path), not only on display refresh
- [x] 2.3 Verify at least two distinct history time windows are shown simultaneously (short and long spans)
- [x] 2.4 Verify log-scaled presentation with adaptive vertical range remains readable for both quiet and loud material in the same window

## 3. Pot fidelity

- [x] 3.1 Verify smoothed pot state uses firmware-equivalent time-scaled EMA (same alpha model as device pot polling)
- [x] 3.2 Verify pot poll interval is aligned to approximately one firmware buffer period at 44.1 kHz
- [x] 3.3 Verify numeric labels beside controls show smoothed values, not instantaneous slider position
- [x] 3.4 Verify control drag updates raw target continuously while EMA stepping remains on the poll cadence only
- [x] 3.5 Verify effect modules (delay primary pot) respond audibly as smoothed value settles

## 4. Documentation

- [x] 4.1 Update `sim/README.md`: pot EMA behavior, level metering overview, and host file list (`level-graph.js`, `pot-simulator.js`)
- [x] 4.2 Remove or correct outdated spike text that claims direct `smoothed` injection with no EMA

## 5. Apply completion

- [x] 5.1 Run `openspec validate simulator-metering-pot-fidelity --type change` and fix any reported issues
- [x] 5.2 Manual browser acceptance: start delay + mic, observe meters and pot labels while dragging primary control; record pass/fail in apply notes

## Explicitly deferred

- Implicit single-unit loopback wiring (documented in README; not part of this change's requirements)
- Automated canvas/visual regression tests
- Phase 1 multi-unit chain UI
