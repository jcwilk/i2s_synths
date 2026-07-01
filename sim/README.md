# Module Chain Simulator (Phase 0 Spike)

Browser proof-of-concept that compiles firmware audio modules to WebAssembly and drives them through a real-time Web Audio path. This tree implements **Phase 0 only**; multi-unit chains and production UX are deferred per [`ai/planning/web-module-chain-simulator.md`](../ai/planning/web-module-chain-simulator.md).

## Prerequisites

| Tool | Purpose |
|------|---------|
| [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) | Compile `src/modules/*.h` to `.wasm` |
| Node.js ≥ 20 | Run `verify-wasm.mjs` and a static dev server |

Activate Emscripten before building:

```bash
source /path/to/emsdk/emsdk_env.sh
```

Microphone capture requires a secure context (`https://` or `http://localhost`).

## Layout

```
sim/
├── build-wasm.sh          # Build passthrough + delay artifacts
├── verify-wasm.mjs        # Headless WASM smoke checks
├── shim/                  # Arduino / ESP32 API stubs for WASM compile
├── wasm/
│   ├── harness.cpp        # Exported setup / upstream / downstream API
│   └── out/               # Generated .js + .wasm (after build)
└── host/
    ├── index.html         # Spike UI
    ├── main.js            # WASM lifecycle + controls
    ├── audio-engine.js    # AudioContext + 512-sample accumulator
    └── module-chain-worklet.js
```

## Build WASM

From the repository root:

```bash
./sim/build-wasm.sh
```

This produces `sim/wasm/out/passthrough.{js,wasm}` and `sim/wasm/out/delay.{js,wasm}`.

Delay ring allocation is verified at setup time. Example build output:

```
Delay module active. Allocated frames=88200 (2000.0 ms)
```

Run headless checks:

```bash
node sim/verify-wasm.mjs
```

## Run locally

Serve the `sim/` directory (any static server works):

```bash
npx serve sim -p 8765
```

Open [http://localhost:8765/host/](http://localhost:8765/host/).

1. Choose **Passthrough** or **Delay**.
2. Adjust primary / secondary pot sliders (0–100%).
3. Toggle microphone input.
4. Click **Start audio** (user gesture required for `AudioContext`).

## Spike audio wiring

- **Sample rate:** 44100 Hz (`SAMPLE_RATE` in firmware `constants.h`).
- **Buffer size:** 512 interleaved stereo int16 samples per path call (`BUFFER_LEN`).
- **Path order:** upstream, then downstream (matches firmware `i2sLoop`).
- **Downstream input:** microphone when enabled; silence when disabled or denied.
- **Speaker output:** upstream path output.
- **Single-unit loopback model:** previous buffer’s downstream output feeds the next buffer’s upstream input so effect modules (delay) remain audible on the upstream speaker tap without a multi-unit chain.

Pot sliders write directly to `DualPotsState.primary.smoothed` and `.secondary.smoothed` (no EMA — Phase 2).

## Spike acceptance (task 5.1)

Verified on branch `cursor/4b53e06e`:

| Scenario | Evidence |
|----------|----------|
| Passthrough + mic path | `verify-wasm.mjs`: impulse on downstream input appears on upstream output after one buffer-period loopback; static server serves host + WASM (HTTP 200). |
| Delay + feedback | `verify-wasm.mjs`: 88200-frame (~2000 ms) ring allocates in WASM heap; impulse energy reaches upstream output after fill time at minimum delay span. |
| Pots change delay | `verify-wasm.mjs`: runtime `HEAPF32` writes to `DualPotsState.smoothed` accepted; UI sliders call the same injection path in `audio-engine.js`. |
| Module re-init | UI `change` handler reloads WASM factory and calls `sim_setup()` per module type. |

**Manual browser check:** With mic enabled, speak or tap near the mic — passthrough should sound immediate; delay should produce audible repeats. Move the primary pot while delay is active to change delay length (drag/hysteresis applies per firmware).

## Limitations and Phase 1 handoff

See [`ai/planning/web-module-chain-simulator.md`](../ai/planning/web-module-chain-simulator.md) for the full roadmap. Spike gaps intentionally left for later phases:

| Area | Spike behavior | Phase 1+ target |
|------|----------------|-----------------|
| Chain topology | One virtual unit | Gateway + N units, horizontal UI |
| Loopback | Implicit one-buffer `ds_out → us_in` | Explicit rightmost-unit toggle |
| Module catalog | Passthrough + delay only | Full `MODULE_*` dropdown per slot |
| Pot controls | Direct `smoothed` injection | `potsUpdate` EMA (Phase 2) |
| Merger / neopixel / ADC | Shimmed or unused | Fidelity + timing (Phase 2) |
| Tooling | Shell build + manual serve | `npm run dev`, CI WASM builds (Phase 3) |
| Gateway | Collapsed into single unit | Separate I/O card at index 0 |

**Phase 1 should:** reuse `sim/wasm/harness.cpp` pattern, load one WASM instance per chain slot, wire `ds_in[i+1] = ds_out[i]` and optional loopback on the last unit, and add the chain MVP UI described in planning.

## Guides read during implementation

- `AGENTS.md` — repo discipline and OpenSpec apply rules
- `ai/code_structure.md` — header-only module layout
- `ai/planning/web-module-chain-simulator.md` — Phase 0 scope and handoff
