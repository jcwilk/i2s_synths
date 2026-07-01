## Why

Firmware audio modules can only be exercised on physical hardware today, which slows iteration when exploring module behavior, chain topology, and control mapping. A browser-based simulator that runs the same module processing logic as firmware would let developers hear and tune modules without soldering or flashing, and de-risks the larger goal of an interactive virtual chain tester.

This change scopes **Phase 0 only**: a minimal spike that proves compiled module logic can run in the browser, process audio buffers at the firmware sample rate, and play the result to speakers. Phases 1–3 (full chain UI, fidelity, developer experience) are captured separately under `ai/planning/` and are explicitly out of scope here.

## What Changes

- Introduce a **module-chain-simulator** capability spec describing Phase 0 spike behavior: single virtual unit, one selectable module type, simulated pot controls, microphone optional input, speaker output.
- Add a browser-hosted proof-of-concept that compiles at least two firmware module variants (passthrough plus one effect module) to WebAssembly and drives them from a real-time audio path.
- Document spike architecture, WASM build approach, and Arduino/ESP shim strategy in `design.md`.
- Defer multi-unit chains, rightmost loopback, arbitrary module mixing per slot, merger timing fidelity, and production UX to later phases (see `ai/planning/web-module-chain-simulator.md`).

## Capabilities

### New Capabilities

- `module-chain-simulator`: Browser-based real-time execution of firmware module processing logic at 44.1 kHz stereo, with simulated controls and optional microphone capture feeding a single virtual unit whose upstream output is played to speakers.

### Modified Capabilities

<!-- None — greenfield capability; firmware module-architecture contract is referenced but not altered -->

## Impact

- **Specs:** New `module-chain-simulator` living spec after archive (Phase 0 requirements only).
- **Repo:** New simulator host tree, WASM build glue, and platform shims for Arduino/ESP-only APIs; no change to on-device firmware behavior.
- **Dependencies:** Browser with Web Audio and microphone permission; host toolchain capable of compiling C++ modules to WebAssembly.
- **Agents:** Complements the in-flight `firmware-build-toolchain` change (headless firmware compile) without overlapping scope.
- **Non-goals (this change):** Multi-unit horizontal chain UI, loopback connector toggle, full module catalog in one page, CI audio regression suite.
