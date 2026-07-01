# Web module chain simulator — planning notes

Phased roadmap for a browser SPA that tests firmware audio modules via WebAssembly. **Phase 0** is captured in the OpenSpec change `web-module-chain-simulator-spike`. This document holds Phases 1–3 for later proposals.

## Context (from exploration)

Physical chain topology:

```
  [speakers]  <── upstream ──  [GATEWAY]  ── downstream ──>  [Unit A] ──> ... ──> [Unit N]
  (line out)                    (line in)                                           (loopback?)
```

Each ESP32 module runs two buffer processors per loop: upstream (right → left) and downstream (left → right). Modules share the firmware contract: `moduleSetup`, `moduleLoopUpstream`, `moduleLoopDownstream`, with `DualPotsState` and 512-sample stereo interleaved int16 buffers at 44.1 kHz.

WASM strategy favored for multi-unit future: **one compiled artifact per module type**, each virtual unit instantiated separately so per-module static globals stay isolated without an immediate C++ instance-state refactor.

---

## Phase 0 — Spike (OpenSpec: `web-module-chain-simulator-spike`)

**Goal:** Prove the riskiest assumptions end-to-end.

- Compile passthrough + one real module (e.g. delay) to WebAssembly.
- Single virtual unit in the browser.
- Simulated primary/secondary pot sliders (direct `smoothed` values).
- Optional mic → downstream input of the unit; upstream output → speakers.
- Match firmware sample rate (44100 Hz) and buffer size (512 int16 stereo samples).
- Arduino/ESP shims: noop Serial/neopixel, `malloc` instead of PSRAM helpers.

**Exit criteria:** Hear processed audio in browser; pot sliders audibly change module behavior; spike documented for Phase 1 handoff.

---

## Phase 1 — Chain MVP

**Goal:** Interactive virtual chain matching the physical left-to-right layout.

### UI

- Fixed **gateway** card at the left (I/O only: mic toggle, speaker tap).
- **+ Add unit** appends module cards to the right.
- Per unit: module type dropdown (passthrough, delay, merger, cutoff, debug_tone).
- Per unit: two pot sliders (primary / secondary, 0–100%).
- **Loopback toggle** on the rightmost unit only: when on, that unit's downstream output feeds its own upstream input (U-turn connector).

### Audio / sim engine

- Run upstream then downstream per unit per buffer (same order as firmware `i2sLoop`).
- Chain wiring:
  - `ds_in[0]` = mic (if enabled) or silence.
  - `ds_in[i+1] = ds_out[i]` for `i >= 0`.
  - `us_in[i] = us_out[i+1]` for interior units.
  - `us_in[last] = loopback ? ds_out[last] : silence`.
  - Speaker output = `us_out[0]` (gateway upstream out).
- One WASM instance per chain slot; load the artifact matching that slot's selected module type.
- Re-instantiate or reset module state when module type changes.

### Scope limits (acceptable for MVP)

- Pot sliders set `smoothed` directly (no EMA).
- Merger cross-path timing approximated (both paths same buffer); note divergence from hardware ring-buffer decoupling.
- Soft cap on chain length (e.g. 8 units) for heap/CPU sanity.

### Deliverables

- Dev server for local testing.
- Rebuild script for all module WASM artifacts.
- Short README section: how to run sim, how it relates to firmware modules.

---

## Phase 2 — Fidelity

**Goal:** Closer parity with hardware behavior where it affects sound or tuning.

- **Pot EMA:** Run `potsUpdate` with elapsed ms between UI updates (or fixed block rate) so control feel matches hardware smoothing.
- **Merger timing:** Model upstream/downstream ring-buffer decoupling so merger underrun/overrun behavior and latency align with firmware.
- **Startup mute:** Optional ~1 s silence on start (firmware startup mute policy).
- **Buffer quantum:** Internal ring between Web Audio render quantum (often 128 frames) and firmware 512-sample buffers to avoid glitches.
- **Gateway modes:** Clarify whether gateway is always transparent passthrough or can host a module (default: I/O shim only).
- **Level metering:** Simple peak/RMS per unit for debugging clipping (merger compressor).

### Testing

- Manual test matrix: each module type in isolation and in short chains.
- Document known sim-vs-hardware deltas in planning or module guides.

---

## Phase 3 — Developer experience

**Goal:** Sustainable workflow for module authors and agents.

- `npm run dev` / `npm run build:wasm` as standard entrypoints.
- CI: compile all WASM variants; optional headless buffer smoke (no browser audio).
- Auto-discover module list from firmware `MODULE_*` constants or shared manifest.
- Hot reload for UI; documented WASM rebuild when `src/modules/` changes.
- Optional: link from `README.md` and `AGENTS.md` guide index.
- Consider follow-on OpenSpec change to extend `module-chain-simulator` spec with Phase 1+ requirements (or separate `web-module-chain-simulator-mvp` change).

### Future options (not committed)

- **Single `sim_core.wasm` with registry** — refactor static globals to instance structs (aligns with `ai/functional_programming.md`); better JS API, larger upfront refactor.
- **Hardware A/B** — same buffer injected to sim and serial-captured from device for regression.
- **Preset chains** — save/load JSON chain configs.

---

## Open questions (carry forward)

| Question | Phase | Default if undecided |
|----------|-------|----------------------|
| Gateway as selectable module? | 1–2 | I/O-only passthrough at index 0 |
| Instant vs EMA pots | 1 vs 2 | Instant in Phase 1 |
| Max chain length | 1 | 8 units |
| Which module for Phase 0 spike besides passthrough | 0 | delay (most representative state) |
