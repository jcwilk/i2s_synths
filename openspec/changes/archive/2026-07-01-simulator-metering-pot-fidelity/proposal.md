## Why

The Phase 0 module chain simulator spike shipped with minimal controls and no level visualization. Post-spike work added scrolling peak meters and firmware-style pot smoothing, but those behaviors are not yet captured in the living spec. Codifying them now prevents drift before Phase 1 chain work and gives reviewers an explicit contract for observability and control feel.

## What Changes

- Add requirements for **real-time level metering** on the simulator host: peak history for upstream (out) and downstream (in) paths, multiple time windows, and continuous updates while audio runs.
- **Modify** the simulated potentiometer requirement so controls use time-scaled exponential smoothing aligned with firmware poll timing, and the operator sees the smoothed value (not the raw slider position) as the authoritative displayed control reading.
- Update the module-chain-simulator **Purpose** section (still placeholder from spike archive).
- Align simulator documentation with agreed behavior (deferred to apply tasks; not a spec delta).

**Non-goals for this change:** multi-unit chain UI, explicit loopback toggle, merger timing fidelity, CI audio regression, or changes to on-device firmware.

## Capabilities

### New Capabilities

<!-- None — extending existing module-chain-simulator domain -->

### Modified Capabilities

- `module-chain-simulator`: add level metering requirements; strengthen pot control requirements for EMA smoothing and displayed smoothed values; replace TBD purpose text.

## Impact

- **Specs:** Delta to `openspec/specs/module-chain-simulator/spec.md` (level metering ADDED; pot controls MODIFIED; Purpose updated).
- **Repo:** Simulator host UI (level graphs, pot poll loop) already largely implemented; apply work is primarily spec reconciliation, README alignment, and any gaps against the new requirements.
- **Dependencies:** None beyond existing browser Web Audio spike.
- **Phases:** Pulls two items forward from planning Phase 2 (pot EMA) into the Phase 0 capability spec; level metering was exploratory and becomes durable intent here.
