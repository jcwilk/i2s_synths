## 1. WASM build extension

- [x] 1.1 Extend `sim/build-wasm.sh` to emit merger, cutoff, and debug_tone artifacts alongside passthrough and delay
- [x] 1.2 Extend `sim/verify-wasm.mjs` with smoke checks for each new WASM variant (setup succeeds, buffer processing callable)

## 2. Chain audio engine

- [x] 2.1 Refactor host audio path to model gateway (index 0) plus N processing slots with per-slot WASM instances
- [x] 2.2 Implement planning wiring rules: gateway `ds_in` from mic/silence, `ds_in[i+1] = ds_out[i]`, interior `us_in[i] = us_out[i+1]`, rightmost `us_in` from loopback or silence, speakers from `us_out[0]`
- [x] 2.3 Invoke upstream then downstream per unit per buffer in chain order consistent with firmware
- [x] 2.4 Re-instantiate and call setup when a slot's module type changes without restarting the audio engine
- [x] 2.5 Enforce soft maximum of eight processing units (host constant)

## 3. Chain UI

- [x] 3.1 Replace single-unit layout with horizontal gateway card (mic toggle, start/stop audio) plus unit cards
- [x] 3.2 Add **Add unit** control (disabled at max length); initial load includes gateway plus one unit
- [x] 3.3 Per-unit module dropdown (all five types), dual pot sliders with smoothed labels, and per-unit level graphs
- [x] 3.4 Add loopback toggle on the rightmost unit card only; default off

## 4. Documentation and acceptance

- [x] 4.1 Update `sim/README.md` for chain MVP layout, wiring, module catalog, loopback, and limitations (merger timing approximation)
- [x] 4.2 Document manual acceptance matrix: passthrough chain, delay with loopback, multi-unit mix, module type swap mid-playback
- [x] 4.3 Run `./sim/build-wasm.sh` and `node sim/verify-wasm.mjs` with evidence noted in apply verification

## Explicitly deferred

- Merger upstream/downstream ring-buffer decoupling and startup mute (Phase 2 fidelity)
- `package.json`, `npm run dev`, CI WASM matrix, auto-discover module manifest (Phase 3 DX)
- Remove-unit UI, gateway as selectable effect module, preset save/load
