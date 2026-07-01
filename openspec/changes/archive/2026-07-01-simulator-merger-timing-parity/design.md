## Context

Merger module logic (`src/modules/merger.h`) captures upstream audio into a ring on the upstream path and consumes it on the downstream path, with optional reverse forwarding from downstream back to upstream. The WASM build runs the same code; parity gaps come from the **chain host**, not from reimplemented merge math.

Today `chain-scheduler.js` processes each slot in one firmware-sized period by:

1. Writing downstream input (gateway output or previous unit's **prior-period** downstream output).
2. Writing upstream input (next unit's upstream output from the **same** period sweep, or loopback from prior-period downstream).
3. Calling upstream then downstream processing immediately.

That collapses path timing: upstream and downstream inputs for a slot reflect the same scheduling tick, whereas on hardware the duplex I2S upstream and downstream streams advance independently through the module chain. Merger rings therefore see artificially synchronized fill/drain, changing latency, feedback strength, and how often underrun/overrun recovery runs.

Pot EMA and level metering parity were addressed in archived Phase 0 fidelity work. Merger timing is the remaining Phase 2 item called out in `ai/planning/web-module-chain-simulator.md` and `sim/README.md`.

## Goals / Non-Goals

**Goals:**

- Model **separate per-path input delay state** for each processing unit so upstream inputs and downstream inputs advance on the timing model the physical chain uses (including existing one-buffer downstream handoff between units).
- Ensure merger slots in multi-unit chains produce **audible and dynamic behavior consistent with firmware merger-module requirements** (delayed blend, forward path, limiting, underrun/overrun handling).
- Expose **operator-visible stress feedback** when merger underrun or overrun occurs (browser-appropriate substitute for NeoPixel flashes).
- Document **remaining intentional deltas** after the fix (e.g., startup mute still absent).
- Add **automated checks** for merger ring behavior in a minimal chain configuration.

**Non-Goals:**

- Refactoring merger WASM or firmware sources unless verification proves spec drift.
- Solving Web Audio 128-frame render quantum vs 512-sample firmware buffer mismatch (separate fidelity item).
- Startup mute (~1 s silence on boot).
- Phase 3 developer-experience tooling.

## Decisions

### 1. Fix host path timing, not merger WASM

**Choice:** Change chain scheduler routing buffers so upstream and downstream inputs for each slot are sourced from path-appropriate delayed state, matching the physical left-to-right / right-to-left propagation model.

**Rationale:** Merger math already matches firmware when inputs are fed correctly (same `merger.h` in WASM). Rewriting merger in JS would violate the compiled-module requirement.

**Alternative considered:** Insert synthetic delay inside WASM harness — rejected because it would diverge from on-device module code paths and complicate all module types.

### 2. Per-slot dual delay lines

**Choice:** Maintain separate previous-buffer (or ring) state for upstream-input feeds and downstream-input feeds per slot index, rather than a single `prevDsOut` array reused for both path semantics.

**Rationale:** Chain MVP used one `prevDsOut` for inter-unit downstream links and loopback; upstream feeds currently pull same-period `usOut[slot+1]`. Physical modules receive upstream from the right on a timeline decoupled from downstream-from-left; modeling both with explicit delay state restores merger ring dynamics.

### 3. Underrun/overrun visibility via unit card status

**Choice:** When compiled merger processing would trigger underrun or overrun recovery (detected via a small WASM export or host-side observation of repeated silence substitution patterns), show a brief inline status cue on that unit card (e.g., colored badge or status text), clearing after a short hold.

**Rationale:** Firmware uses NeoPixel color flashes; browser has no NeoPixel. Operators need feedback when tuning merge coefficients near buffer limits.

**Alternative considered:** Silent parity only — rejected because underrun/overrun are part of observable merger behavior in the module spec.

### 4. Verification: isolated merger chain smoke

**Choice:** Extend headless verification with a short synthetic chain run: passthrough or silence feed into a merger slot with controlled upstream injection, asserting delayed upstream energy and stable output bounds after ring fill.

**Rationale:** `verify-wasm.mjs` today smoke-tests merger WASM in isolation with same-period upstream/downstream calls — insufficient to catch host wiring regressions.

## Risks / Trade-offs

- **[Subtle timing bugs]** → Mitigate with headless chain test plus manual matrix entry (passthrough → merger → delay with loopback).
- **[Stress UI false positives]** → Tie indicators to merger module stress events only, not generic clipping.
- **[Interaction with pane delete/reorder change]** → Scheduler API changes should be orthogonal; structural rebuild from that change resets delay state cleanly.
- **[Not full hardware clone]** → README still lists startup mute and render-quantum gaps explicitly after this change.

## Migration Plan

Static host asset update; operators refresh browser. No WASM rebuild required unless harness export is added for stress detection. Re-run `./sim/build-wasm.sh` only if harness changes.

## Open Questions

- Whether stress detection should use a dedicated WASM export (cleanest) or infer from host-side level/meter heuristics (no harness change). Default in tasks: prefer explicit export if minimal.
