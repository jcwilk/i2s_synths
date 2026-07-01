## Context

The repository uses header-only audio modules selected at compile time. An informal delay guide at `ai/modules/delay.md` describes intended tape-loop delay behavior including length, speed, drag/hysteresis, and clickless retime. Uncommitted work on `src/modules/delay.h` partially implements that vision but omits speed control and normal wrap fades, contains unused helpers, uses heap allocation on span changes, and has a startup mapping inconsistency.

No OpenSpec living specs exist yet for module architecture or individual modules. This change establishes the spec tree and targets a clean delay implementation aligned with the informal guide's intent.

## Goals / Non-Goals

**Goals:**

- Archive behavior contracts for module architecture and all current major modules.
- Implement delay module to satisfy `delay-module` spec, superseding uncommitted `delay.h` work rather than polishing it in place.
- Retire `ai/modules/delay.md` to eliminate competing delay documentation.
- Keep brownfield module specs faithful to shipped behavior (merger, cutoff, debug-tone, passthrough).

**Non-Goals:**

- Rewriting non-delay modules unless spec drafting reveals critical safety bugs.
- Gateway/I2S pipeline changes beyond what delay allocation may require.
- Moving `ai/code_structure.md` or other pattern guides into OpenSpec (they remain contributor references).

## Decisions

### 1. Discard uncommitted delay work as the implementation baseline

**Choice:** Apply begins by reverting uncommitted `delay.h` and related tunable edits, then implements fresh against the OpenSpec delay spec.

**Rationale:** The uncommitted rewrite is mid-refactor (dead code, missing speed/wrap behaviors, malloc in audio path). A clean implementation from a validated contract is lower risk than incremental cleanup.

**Alternatives considered:** Land uncommitted changes then patch gaps — rejected because it bakes in structural debt.

### 2. Frame-indexed tape model with fractional speed accumulator

**Choice:** Bookkeeping in whole stereo frames; speed > nominal tracked via fractional frame position and remainder across buffers.

**Rationale:** Matches informal guide terminology and avoids perpetual even-sample alignment checks.

### 3. Fixed scratch for retime crossfades

**Choice:** Use a statically sized scratch region sized from maximum crossfade frames for shorten/lengthen operations.

**Rationale:** Avoids heap allocation during span changes (present in uncommitted work). Bounded fade window keeps scratch size predictable.

### 4. External RAM first for delay storage

**Choice:** Attempt largest configured delay buffer in external RAM, halve until allocation succeeds, fall back to internal RAM, then passthrough if below minimum.

**Rationale:** ESP32-S3 modules in this project typically have PSRAM; long delays require it. Behavior is specified in the delay spec; this is the expected implementation strategy.

### 5. Spec domains split per module

**Choice:** Separate OpenSpec capabilities for architecture and each module rather than one monolithic audio spec.

**Rationale:** One independently testable capability per spec file; agents can scope work to delay without loading merger detail.

### 6. Retire informal delay guide via deletion task

**Choice:** Delete `ai/modules/delay.md` during apply after delay spec content is captured in OpenSpec.

**Rationale:** User requirement for single source of truth. `AGENTS.md` guide index should drop the `ai/modules/` pointer for delay (note in tasks — actual AGENTS edit may be follow-up or included in apply if tasks say so).

## Risks / Trade-offs

- **[Risk] Speed control adds DSP complexity** → Implement nominal 1:1 path first, then fractional-frame averaging; verify with tone sweep on hardware.
- **[Risk] Lengthen staging still hard to verify by ear** → Acceptance uses click/pop listen test plus logic checks on wrap/stage lifecycle.
- **[Risk] Brownfield module specs may omit edge cases** → Specs describe observable behavior today; drift notes go in tasks, not speculative requirements.
- **[Risk] `openspec init` added stock Cursor skills alongside existing OSF bundle** → Do not commit duplicate stock skills as part of this change; only the change folder and any required `openspec/config.yaml` if not yet tracked.

## Migration Plan

1. Approve change artifacts.
2. Apply on isolated branch: archive specs, implement delay, delete `ai/modules/delay.md`, update `AGENTS.md` guide index to point at `openspec/specs/` for module behavior.
3. Hardware smoke test with delay module active: silence at boot, length sweep, speed sweep, rapid knob jitter without clicks.
4. Rollback: restore previous `delay.h` from last release commit and re-add informal guide from git history if needed.

## Open Questions

- Exact maximum delay duration and crossfade milliseconds — propose defaults (~2000 ms max, ~8 ms crossfade) in `constants.h` during apply unless human specifies otherwise.
- Whether `AGENTS.md` guide index should reference `openspec/specs/` generically or list module spec paths — prefer generic pointer plus README note.
