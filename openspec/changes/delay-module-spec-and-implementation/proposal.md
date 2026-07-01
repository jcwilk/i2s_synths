## Why

The delay module has substantial uncommitted implementation work that partially aligns with the informal `ai/modules/delay.md` guide but introduces dead code, behavioral gaps (speed control, wrap fades, startup span), and competes with that document as an informal source of truth. The project also lacks OpenSpec living specs for the modular audio architecture and existing modules, making agent-assisted work harder to review and verify.

Establishing behavior contracts under `openspec/specs/` and implementing delay against them replaces ad-hoc iteration with a reviewable target and a coherent spec tree for the rest of the module system.

## What Changes

- Introduce OpenSpec living specs (via archive after apply) for:
  - modular audio architecture (selection, dual-path I/O contract, shared controls),
  - delay module (target behavior from the prior informal delay guide),
  - merger, cutoff, debug-tone, and passthrough modules (brownfield behavior as implemented today).
- Implement the delay module to satisfy the delay spec instead of landing the current uncommitted `delay.h` rewrite as-is.
- **Remove** `ai/modules/delay.md` so OpenSpec becomes the sole authoritative delay behavior document (informal guide retired, not duplicated).
- Leave other `ai/` guides (`code_structure.md`, `functional_programming.md`, `hardware.md`) in place as implementation-pattern references; they do not define module behavior contracts.

## Capabilities

### New Capabilities

- `module-architecture`: How sketches select and invoke exactly one audio module across upstream and downstream I2S paths, including the shared control surface passed into loop handlers.
- `delay-module`: Variable tape-loop delay with length and speed controls, drag/hysteresis filtering, clickless retime, and startup behavior.
- `merger-module`: Upstream/downstream merge with ring-buffered cross-path mixing and optional downstream-to-upstream forwarding (as implemented today).
- `cutoff-module`: State-variable-filter tone shaping on the downstream path with pot-mapped cutoff and LP/HP blend.
- `debug-tone-module`: Test oscillator with configurable upstream/downstream injection and waveshape blending.
- `passthrough-module`: Identity copy on both paths (baseline/reference module).

### Modified Capabilities

<!-- None — greenfield OpenSpec specs for this repo -->

## Impact

- **Specs:** New domains under `openspec/specs/` after archive; removal of `ai/modules/delay.md`.
- **Code:** `src/modules/delay.h` rewritten or heavily revised to match delay spec; possible small `src/config/constants.h` tunables for delay design-time limits.
- **Uncommitted work:** Current uncommitted `delay.h`, related `constants.h` edits, and `ai/modules/delay.md` are superseded by this change — apply should not treat them as the shipping baseline.
- **Other modules:** Spec-only documentation of existing behavior; no required code changes unless spec drafting reveals obvious drift worth noting in tasks.
