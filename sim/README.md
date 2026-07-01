# Module Chain Simulator (Chain MVP)

Browser simulator that compiles firmware audio modules to WebAssembly and drives them through a real-time Web Audio chain matching the physical left-to-right module topology. **Phase 1 chain MVP** — gateway I/O card, appendable processing units, full module catalog, per-slot WASM instances, and explicit rightmost loopback. See [`ai/planning/web-module-chain-simulator.md`](../ai/planning/web-module-chain-simulator.md) for Phases 2–3.

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
├── build-wasm.sh          # Build all five module WASM variants
├── verify-wasm.mjs        # Headless WASM smoke checks
├── shim/                  # Arduino / ESP32 API stubs for WASM compile
├── wasm/
│   ├── harness.cpp        # Exported setup / upstream / downstream API
│   └── out/               # Generated .js + .wasm (after build)
└── host/
    ├── index.html         # Chain MVP UI (gateway + unit cards)
    ├── main.js            # Unit lifecycle, pot poll, level samplers
    ├── chain-scheduler.js # Multi-slot buffer routing
    ├── audio-engine.js    # AudioContext + 512-sample accumulator
    ├── level-graph.js     # Scrolling log-scaled peak history (canvas)
    ├── pot-simulator.js   # Firmware-equivalent pot EMA at buffer poll cadence
    └── module-chain-worklet.js
```

## Build WASM

From the repository root:

```bash
./sim/build-wasm.sh
```

This produces five artifacts under `sim/wasm/out/`:

| Artifact | Module |
|----------|--------|
| `passthrough.{js,wasm}` | Transparent pass-through |
| `delay.{js,wasm}` | Stereo delay with feedback |
| `merger.{js,wasm}` | Compressor / merge |
| `debug_tone.{js,wasm}` | Test tone generator |
| `cutoff.{js,wasm}` | Low-pass filter |

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

1. **Gateway** (left card): toggle microphone, start/stop audio.
2. **Processing units**: module type dropdown (all five kinds), primary/secondary pot sliders, In/Out level graphs.
3. **Add unit**: append a card to the right (disabled at eight units).
4. **Loopback** (rightmost unit only): when enabled, that unit's upstream input receives its prior downstream output; default off.

Initial load includes gateway plus one delay unit.

## Chain wiring

- **Sample rate:** 44100 Hz (`SAMPLE_RATE` in firmware `constants.h`).
- **Buffer size:** 512 interleaved stereo int16 samples per path call (`BUFFER_LEN`).
- **Gateway (index 0):** I/O shim only — no WASM, transparent passthrough on both paths.
- **Processing units (slots 1..N):** one WASM instance per slot; upstream then downstream per unit per buffer, right-to-left sweep order.

| Signal | Source |
|--------|--------|
| `ds_in[0]` | Microphone when enabled; silence otherwise |
| `ds_in[i+1]` | Prior period `ds_out[i]` |
| `us_in[i]` (interior) | Prior period `us_out[i+1]` |
| `us_in[last]` | Prior period `ds_out[last]` when loopback on; silence when off |
| Speakers | Prior period `us_out[0]` (gateway upstream output) |

Maximum **eight** processing units beyond the gateway (`MAX_PROCESSING_UNITS` in `chain-scheduler.js`).

## Module catalog

Each unit's dropdown offers: passthrough, delay, merger, cutoff, debug tone. Changing a unit's type re-instantiates that slot's WASM and calls `sim_setup()` without restarting the audio engine. Pot smoothed values reset to current slider positions on type change.

## Pot controls (firmware-equivalent EMA)

Pot sliders represent the simulated **raw** ADC reading in `[0, 1]`. Smoothed state written to each slot's WASM `DualPotsState` uses the same time-scaled exponential moving average as firmware `potsUpdate` in `src/input/pots.h`:

- `EMA_ALPHA = 0.008` per millisecond
- Poll interval ≈ one I2S buffer period at 44.1 kHz (`512 / 44100` ≈ 11.6 ms)

Each unit card owns an independent pot poll loop writing that slot's WASM struct.

## Level metering

Each processing unit card shows three scrolling peak-history graphs (4 s / 16 s / 64 s) for downstream **In** (blue) and upstream **Out** (orange). One peak sample per path per firmware buffer period (~86 Hz).

## Limitations

| Area | Current behavior | Future phase |
|------|------------------|--------------|
| Startup mute | Not modeled (~1 s silence on device boot) | Phase 2 |
| Render quantum | Web Audio 128-frame render quantum vs 512-sample firmware buffer (no internal ring) | Phase 2 |
| Remove unit | Add-only chain | Future UX |
| Tooling | Shell build + manual serve | Phase 3 npm/CI |
| Gateway module | I/O shim only | Optional Phase 2 |

Merger cross-path timing uses decoupled per-path delay state in the chain host (Phase 2 parity). Merger unit cards show brief **Underrun** / **Overrun** badges when the compiled module triggers those recovery paths.

## Manual acceptance matrix

| Scenario | How to verify |
|----------|---------------|
| **Passthrough chain** | Add 2–3 units, all passthrough, mic on — speech should pass through with minimal latency; In/Out meters on each unit show activity. |
| **Delay with loopback** | Single delay unit, enable loopback on rightmost card, mic on — audible repeats; primary pot changes delay time as smoothed value settles. |
| **Multi-unit mix** | Chain passthrough → merger (or cutoff) → delay; confirm distinct processing per slot and downstream propagation left-to-right. |
| **Merger loopback latency** | Chain passthrough → merger → delay with loopback on the delay unit; mic on. Confirm merge latency and feedback strength feel stable as pots settle; merger card may flash Underrun while rings fill, then steady state. Secondary pot high should emphasize delayed upstream blend on merger downstream output. |
| **Module type swap mid-playback** | Start audio, change one unit from passthrough to debug tone — tone appears without stopping/restarting audio; other units unaffected. |

## Automated acceptance

| Scenario | Evidence |
|----------|----------|
| All five WASM variants | `verify-wasm.mjs`: setup + process smoke for each |
| Passthrough loopback path | `verify-wasm.mjs`: impulse on downstream → upstream after loopback wiring |
| Delay ring + feedback | `verify-wasm.mjs`: 88200-frame ring; delayed upstream energy |
| Debug tone output | `verify-wasm.mjs`: downstream tone energy after process |
| Dual-path chain delay | `verify-wasm.mjs`: two-slot passthrough chain; downstream feed delayed one period |
| Merger chain routing | `verify-wasm.mjs`: passthrough → merger with loopback; delayed blend and forward path |
| Merger stress export | `verify-wasm.mjs`: harness underrun/overrun flags from compiled merger |
| Pot EMA | `pot-simulator.js` mirrors `pots.h`; per-unit poll in `main.js` |
| Chain length cap | `MAX_PROCESSING_UNITS = 8`; Add unit disabled at cap |

**Manual browser check:** Run the acceptance matrix above on [http://localhost:8765/host/](http://localhost:8765/host/) with `./sim/build-wasm.sh` completed first.
