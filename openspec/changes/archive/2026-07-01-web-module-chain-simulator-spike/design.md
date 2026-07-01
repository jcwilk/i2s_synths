## Context

Firmware audio modules live under `src/modules/` as header-only C++ with a stable dual-path API (setup, upstream loop, downstream loop). They depend on Arduino and ESP32 APIs for logging, ADC, PSRAM, and UI side effects. There is no browser or WASM toolchain today.

Exploration (`/osf-explore`) mapped the target end state: a left-to-right virtual chain with mic at the gateway, loopback on the rightmost unit, and per-unit module selection. Phases 1–3 are documented in `ai/planning/web-module-chain-simulator.md`. This design covers **Phase 0 only**: validate that the same processing logic can run in a browser and drive real-time audio I/O.

## Goals / Non-Goals

**Goals:**

- Prove WASM compilation of at least passthrough and one stateful effect module (delay).
- Drive modules through the same buffer contract as firmware: 44.1 kHz, stereo interleaved 16-bit PCM, 512-sample buffers.
- Expose simulated primary/secondary pot values to downstream processing.
- Optional microphone capture as downstream input; play upstream output to speakers.
- Document shim strategy and spike limitations for Phase 1.

**Non-Goals:**

- Multi-unit chains, loopback toggle, or horizontal chain UI (Phase 1).
- Pot EMA smoothing, merger ring timing fidelity, startup mute (Phase 2).
- CI WASM builds, npm DX polish, README/AGENTS integration (Phase 3).
- Changing on-device firmware behavior or module-architecture requirements.

## Decisions

### 1. Repo layout: `sim/` host tree

Place browser host, WASM build scripts, and platform shims under `sim/` at repo root. Keeps Arduino sketch layout untouched and parallels the upcoming `firmware-build-toolchain` scripts pattern.

**Alternatives:** Top-level `web/` (rejected: `sim` signals non-production test harness); embedding in `src/` (rejected: pollutes firmware compile).

### 2. One WASM artifact per module type (spike: two builds)

Compile separate WASM binaries for passthrough and delay (minimum), each with `ACTIVE_MODULE` fixed at build time—mirroring firmware's one-module-per-build model.

Instantiate one WASM `Instance` for the spike's single virtual unit. Multiple instances of the same artifact isolate static globals without refactoring module state into structs.

**Alternatives:** Single registry WASM (deferred to Phase 3 option; requires instance-state refactor).

### 3. Emscripten as the WASM toolchain

Use Emscripten to compile a thin C++ harness that includes the module header, `constants.h` (with `ENABLE_GATEWAY` and sim defines), and shim headers replacing Arduino/ESP APIs.

Export C-callable wrappers: setup, process upstream, process downstream, with pointers into WASM linear memory for in/out buffers and pot struct.

**Alternatives:** WASI + clang without Emscripten (more glue); transpile to JS (loses fidelity).

### 4. Platform shims (`sim/shim/`)

| Firmware API | Spike shim |
|--------------|------------|
| `Serial.print*` | No-op or optional `console` hook |
| `neopixelSetTimedColor` | No-op |
| `analogRead` / `analogSetAttenuation` | Unused if pots passed by value; stub if moduleSetup calls them |
| `ps_malloc` / `psramFound` | `malloc` / return true |
| `Arduino.h` types | Minimal typedefs / `stdint` |

Modules that read pots only from passed `DualPotsState` need no ADC shim in the audio path.

### 5. Browser audio: AudioWorklet + internal 512-sample accumulator

Run audio at 44100 Hz. AudioWorklet render quantum is often 128 frames; accumulate float samples until 512 int16 stereo samples, call WASM downstream (and upstream for spike: passthrough upstream or copy), emit upstream output to speakers.

Spike runs **one** virtual unit: downstream input = mic (if enabled) or silence; upstream output = speaker feed. Both paths invoked per buffer to match firmware call order (upstream first, then downstream).

**Alternatives:** ScriptProcessor (deprecated); main-thread polling (latency).

### 6. Pot simulation: direct `smoothed` injection

UI sliders write `DualPotsState.primary.smoothed` and `.secondary.smoothed` in [0, 1]. Skip `potsUpdate` EMA for spike simplicity; document as Phase 2 item.

### 7. Minimal spike UI

Plain HTML or lightweight Vite page: module type selector (passthrough / delay), two range inputs, mic on/off, start/stop audio. No chain graph.

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| Emscripten build friction on agent/CI hosts | Document host prerequisites in `sim/README.md`; spike task includes local compile evidence |
| Delay module heap size (~2s buffer) exhausts WASM memory | Set Emscripten `INITIAL_MEMORY` / allow growth; verify allocation in spike |
| Web Audio quantum ≠ 512 samples | Internal accumulator; accept added latency ≤ one firmware buffer |
| Merger/neopixel side effects in other modules | Noop shims; spike only builds passthrough + delay |
| Sim diverges from hardware over time | Spec references firmware buffer contract; Phase 2 fidelity pass |
| Mic permission denied | Spec: graceful silence when mic requested but unavailable |

## Migration Plan

Greenfield addition under `sim/`. No firmware migration. Rollback = remove `sim/` tree; living spec can be reverted on archive rollback if needed.

## Open Questions

- **Delay vs cutoff for second spike module?** Default delay (stateful, representative); cutoff is lighter if delay heap blocks spike.
- **Stereo mic mixing?** Default: downmix input channels to stereo int16 for downstream in.
- **HTTPS requirement for mic?** Document that local dev may need `localhost` or TLS for `getUserMedia`.
