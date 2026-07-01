## Required for this change

## 1. Simulator scaffold

- [ ] 1.1 Create `sim/` tree: host app entry, WASM build directory, platform shim headers per `design.md`
- [ ] 1.2 Add `sim/README.md` with host prerequisites (Emscripten, Node), build commands, and how to run the spike locally

## 2. WASM module builds

- [ ] 2.1 Implement C++ harness exporting setup, upstream process, and downstream process with shared buffer and pot-state memory layout
- [ ] 2.2 Add Arduino/ESP shims (Serial, neopixel, PSRAM/malloc, minimal Arduino types) so passthrough and delay module headers compile
- [ ] 2.3 Build WASM artifacts for passthrough and delay module variants; verify delay ring allocation succeeds within WASM heap limits

## 3. Browser audio engine

- [ ] 3.1 Configure AudioContext at 44100 Hz with AudioWorklet (or equivalent) real-time path
- [ ] 3.2 Accumulate render-quantum samples to 512-sample stereo int16 buffers matching firmware `BUFFER_LEN`
- [ ] 3.3 Per buffer: invoke upstream then downstream on the active WASM instance; route upstream output to speakers
- [ ] 3.4 Implement mic capture toggle: when on, feed captured audio to downstream input; when off or denied, use silence

## 4. Spike UI

- [ ] 4.1 Minimal page: module type selector (passthrough / delay), primary and secondary pot sliders (0–100%), mic toggle, start control
- [ ] 4.2 Wire UI to WASM instance lifecycle (re-init on module type change) and live pot state injection into `DualPotsState.smoothed`

## 5. Verification

- [ ] 5.1 Manual spike acceptance: hear passthrough with mic on; hear delay with audible tap/feedback; pot sliders change delay length audibly
- [ ] 5.2 Record spike limitations and Phase 1 handoff notes in `sim/README.md` (reference `ai/planning/web-module-chain-simulator.md`)

## Explicitly deferred

- Phase 1 chain MVP (multi-unit UI, loopback toggle, full module dropdown) — see `ai/planning/web-module-chain-simulator.md`
- Phase 2 fidelity (pot EMA, merger ring timing, startup mute, buffer metering)
- Phase 3 DX (CI WASM builds, npm scripts polish, README/AGENTS guide index links)
- Follow-on OpenSpec change to extend `module-chain-simulator` requirements beyond Phase 0
