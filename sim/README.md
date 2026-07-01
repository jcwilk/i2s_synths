# Module Chain Simulator (Phase 0)

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
    ├── index.html         # Spike UI (module select, pots, level graphs)
    ├── main.js            # WASM lifecycle, pot poll loop, level sampler
    ├── audio-engine.js    # AudioContext + 512-sample accumulator + peak feed
    ├── level-graph.js     # Scrolling log-scaled peak history (canvas)
    ├── pot-simulator.js   # Firmware-equivalent pot EMA at buffer poll cadence
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
2. Adjust primary / secondary pot sliders (0–100%). Numeric labels show **smoothed** values; sliders move instantly to the raw target.
3. Toggle microphone input.
4. Click **Start audio** (user gesture required for `AudioContext`).
5. Watch the level graphs while audio runs (see **Level metering** below).

## Audio wiring

- **Sample rate:** 44100 Hz (`SAMPLE_RATE` in firmware `constants.h`).
- **Buffer size:** 512 interleaved stereo int16 samples per path call (`BUFFER_LEN`).
- **Path order:** upstream, then downstream (matches firmware `i2sLoop`).
- **Downstream input:** microphone when enabled; silence when disabled or denied.
- **Speaker output:** upstream path output.
- **Single-unit loopback model:** previous buffer’s downstream output feeds the next buffer’s upstream input so effect modules (delay) remain audible on the upstream speaker tap without a multi-unit chain.

## Pot controls (firmware-equivalent EMA)

Pot sliders represent the simulated **raw** ADC reading in `[0, 1]`. Smoothed state written to WASM `DualPotsState` uses the same time-scaled exponential moving average as firmware `potsUpdate` in `src/input/pots.h`:

- `EMA_ALPHA = 0.008` per millisecond
- Effective alpha from elapsed milliseconds: `1 - (1 - EMA_ALPHA) ** deltaMs`
- Poll interval ≈ one I2S buffer period at 44.1 kHz (`512 / 44100` ≈ 11.6 ms)

While dragging a slider, the raw target syncs continuously (animation frame loop); EMA stepping runs only on the poll timer. Modules read `smoothed` fields, so delay length and other mappings ease toward the new position rather than jumping.

## Level metering

While audio is running, three scrolling peak-history graphs show downstream **In** (blue) and upstream **Out** (orange) on the same canvas:

| Window | Span |
|--------|------|
| Fast | 4 s |
| Medium | 16 s |
| Slow | 64 s |

- **Peak cadence:** one linear peak per path per firmware buffer period (~86 Hz), computed in `audio-engine.js` when each 512-sample buffer completes — not on display refresh.
- **Display refresh:** graphs redraw at 30 Hz; pixel columns advance on peak arrival so horizontal scroll stays stable.
- **Scale:** logarithmic amplitude with adaptive ceiling (`max(−18 dBFS floor, loudest peak in view)`); Y-axis easing avoids jumps between frames.

Meters advance continuously during playback without restarting the audio engine.

## Acceptance

| Scenario | Evidence |
|----------|----------|
| Passthrough + mic path | `verify-wasm.mjs`: impulse on downstream input appears on upstream output after one buffer-period loopback. |
| Delay + feedback | `verify-wasm.mjs`: 88200-frame (~2000 ms) ring allocates; impulse energy reaches upstream output after fill time. |
| Pot EMA | `pot-simulator.js` mirrors `pots.h` alpha model; `main.js` polls at `POT_POLL_MS`; labels read `heapF32[smoothed]`. |
| Level metering | `level-graph.js` dual-path peaks at `AUDIO_PEAK_HZ`; three simultaneous windows; log scale with `MIN_SCALE_LINEAR` floor. |
| Module re-init | UI `change` handler reloads WASM factory and calls `sim_setup()` per module type. |

**Manual browser check:** With mic enabled, speak or tap near the mic — passthrough should sound immediate; delay should produce audible repeats. While delay is active, drag the primary pot and confirm (1) numeric label eases toward the slider, (2) delay time changes audibly as smoothed value settles, (3) In/Out level graphs scroll during playback.

## Limitations and Phase 1 handoff

See [`ai/planning/web-module-chain-simulator.md`](../ai/planning/web-module-chain-simulator.md) for the full roadmap. Spike gaps intentionally left for later phases:

| Area | Phase 0 behavior | Phase 1+ target |
|------|------------------|-----------------|
| Chain topology | One virtual unit | Gateway + N units, horizontal UI |
| Loopback | Implicit one-buffer `ds_out → us_in` | Explicit rightmost-unit toggle |
| Module catalog | Passthrough + delay only | Full `MODULE_*` dropdown per slot |
| Merger / neopixel / ADC | Shimmed or unused | Fidelity + timing (Phase 2) |
| Tooling | Shell build + manual serve | `npm run dev`, CI WASM builds (Phase 3) |
| Gateway | Collapsed into single unit | Separate I/O card at index 0 |

**Phase 1 should:** reuse `sim/wasm/harness.cpp` pattern, load one WASM instance per chain slot, wire `ds_in[i+1] = ds_out[i]` and optional loopback on the last unit, and add the chain MVP UI described in planning.
