## 1. Baseline and documentation hygiene

- [ ] 1.1 Revert uncommitted changes to `src/modules/delay.h` and `src/config/constants.h` to last committed versions (do not use the in-progress rewrite as the implementation baseline)
- [ ] 1.2 Delete `ai/modules/delay.md` and remove any `ai/modules/` references to it from `AGENTS.md` (point module behavior to `openspec/specs/` instead)
- [ ] 1.3 Remove empty `ai/modules/` directory if no other module guides remain

## 2. Delay module implementation

- [ ] 2.1 Implement frame-indexed tape loop with read-before-write, upstream passthrough, and downstream-only delay processing per `delay-module` spec
- [ ] 2.2 Implement primary length mapping with drag/hysteresis after smoothed pot input
- [ ] 2.3 Implement secondary speed control with fractional frame accumulator and weighted averaging above nominal speed
- [ ] 2.4 Implement clickless shorten, lengthen (staged tail lifecycle), and normal wrap crossfades using fixed scratch buffers (no heap alloc in span-change path)
- [ ] 2.5 Implement PSRAM-first ring allocation with halving fallback and passthrough when allocation fails below minimum span
- [ ] 2.6 Initialize startup state: cleared buffer, maximum span, effective length mapped to maximum, silent output until history fills

## 3. Configuration

- [ ] 3.1 Add delay design-time tunables to `src/config/constants.h` (maximum delay duration, minimum span, crossfade duration) aligned with design defaults unless human overrides during apply

## 4. Verification

- [ ] 4.1 Build firmware with `ACTIVE_MODULE` set to delay and confirm clean compile
- [ ] 4.2 Hardware smoke test: boot silence, length sweep without clicks, speed sweep changes effective delay, rapid knob jitter does not cause retime chatter

## Explicitly deferred

- Code changes to merger, cutoff, debug-tone, or passthrough modules (spec-only in this change unless critical drift is discovered during review)
- Committing duplicate OpenSpec stock Cursor skills created by `openspec init` (out of scope for this change)
