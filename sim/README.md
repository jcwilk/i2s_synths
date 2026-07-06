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

Serve the `sim/` directory (bridge uses port **8765** by default):

```bash
npx serve sim -p 8080
```

Open [http://localhost:8080/host/](http://localhost:8080/host/).

Start the hardware bridge (separate terminal):

```bash
npm start --prefix sim/hardware-bridge
# or: FIRMWARE_PORT=/dev/ttyACM0 npm start --prefix sim/hardware-bridge
```

Default bridge WebSocket URL in the gateway card: `ws://localhost:8765`. See [`hardware-bridge/README.md`](hardware-bridge/README.md) for troubleshooting.

1. **Gateway** (left card): toggle microphone, start/stop audio.
2. **Processing units**: module type dropdown (all five kinds), primary/secondary pot sliders, In/Out level graphs. Each card header has a **drag handle** (⠿) for reorder and a **delete** button (×).
3. **Add unit**: append a card to the right (disabled at eight units).
4. **Delete unit**: remove a card via × in the header. Disabled on the sole remaining unit while audio is running; when stopped, you may delete down to zero units.
5. **Reorder units**: drag a unit card by its handle to a new position among other unit cards. The gateway card stays fixed. Sliders and selectors do not initiate reorder.
6. **Loopback** (rightmost unit only): when enabled, that unit's upstream input receives its prior downstream output; default off.
7. **Hardware slot** (Phase 2): mark one unit as hardware, match module type to flashed firmware, start bridge, Connect. Physical device pots apply when connected; WASM sliders hidden.

Initial load includes gateway plus one delay unit.

## Structural reconfiguration and audio rebuild

These operator actions are **structural reconfiguration**: add unit, delete unit, reorder units, change module type, toggle loopback on the rightmost unit, **designate/clear hardware mode**, **connect/disconnect hardware session**. When audio is **running**, each structural edit briefly stops output, clears level histories and playback buffers, re-syncs the chain scheduler (including path delay state), and **auto-restarts** audio while preserving the microphone toggle and pot slider positions. The status line shows “Restarting audio…” during rebuild; failures surface there with a prompt to tap Start again.

**Pot slider** moves on WASM units are **not** structural. **Physical pot** movement on a connected hardware device is **not** structural. Disconnect hardware before reordering units.

When audio is **stopped**, structural edits update layout and configuration only; audio does not start automatically.

## Chain wiring

- **Sample rate:** 22050 Hz mono (`SAMPLE_RATE` in firmware `constants.h`).
- **Buffer size:** 128 mono int16 samples per path call (`BUFFER_LEN`, ~5.8 ms period).
- **Gateway (index 0):** I/O shim only — no WASM, transparent passthrough on both paths.
- **Processing units (slots 1..N):** one WASM instance per slot (or hardware bridge slot when designated and connected); upstream then downstream per unit per buffer, right-to-left sweep order.

| Signal | Source |
|--------|--------|
| `ds_in[0]` | Microphone when enabled; silence otherwise |
| `ds_in[i+1]` | Prior period `ds_out[i]` |
| `us_in[i]` (interior) | Prior period `us_out[i+1]` |
| `us_in[last]` | Prior period `ds_out[last]` when loopback on; silence when off |
| Speakers | Prior period `us_out[0]` (gateway upstream output) |

Maximum **eight** processing units beyond the gateway (`MAX_PROCESSING_UNITS` in `chain-scheduler.js`).

## Module catalog

Each unit's dropdown offers: passthrough, delay, merger, cutoff, debug tone. Changing a unit's type re-instantiates that slot's WASM and calls `sim_setup()`. When audio is running, module type change triggers a full audio rebuild (see Structural reconfiguration above). Pot smoothed values reset to current slider positions on type change.

## Pot controls (firmware-equivalent EMA)

Pot sliders represent the simulated **raw** ADC reading in `[0, 1]`. Smoothed state written to each slot's WASM `DualPotsState` uses the same time-scaled exponential moving average as firmware `potsUpdate` in `src/input/pots.h`:

- `EMA_ALPHA = 0.008` per millisecond
- Poll interval ≈ one I2S buffer period at 22.05 kHz (`128 / 22050` ≈ 5.8 ms)

Each unit card owns an independent pot poll loop writing that slot's WASM struct.

## Level metering

Each processing unit card shows three scrolling peak-history graphs (4 s / 16 s / 64 s) for downstream **In** (blue) and upstream **Out** (orange). One peak sample per path per firmware buffer period (~172 Hz at 22.05 kHz / 128).

## Hardware slot (Phase 2–3)

| Topic | Behavior |
|-------|----------|
| **Limit** | At most **one** hardware-designated unit per chain |
| **Bridge** | Local Node process relays WebSocket ↔ USB (`sim/hardware-bridge/`) |
| **Module parity** | Slot module type must match flashed firmware before Connect |
| **Controls** | Physical pots on device when connected; WASM sliders hidden; telemetry read-only on card |
| **Async pipeline** | Ring buffers (**3** periods default, adaptive **4** on sustained underrun) |
| **Recovery** | Transient underrun → silence; overrun → drop oldest queued exchange; sustained >5%/30s → warning |
| **Extra latency** | See mixed-chain latency budget below |

### Mixed-chain latency budget (Phase 3)

| Component | Budget (periods @ 5.8 ms) | Notes |
|-----------|---------------------------|-------|
| Ring target occupancy | 2 | Nominal pipeline delay |
| USB + bridge RTT (p99 planning) | 3.5 | Aligns with Phase 1 p99 ≤ 20 ms |
| Inter-unit path delay | 1 each | Standard simulator wiring |
| **HW between 2 WASM neighbors** | **~5.5 periods (~32 ms)** | Plus neighbor delays in longer chains |

Tolerance: **±1 buffer period** for reference topology `[gateway, WASM passthrough, HW delay, WASM passthrough]`.

### Latency measurement procedure

1. Build reference chain: gateway → passthrough → **hardware delay** (connected) → passthrough; mic on, loopback on rightmost if needed.
2. Flash delay firmware; connect bridge; start audio.
3. Add a debug-tone or impulse neighbor **only on WASM** side for correlation, or use merger loopback with known delay pot setting.
4. Compare audible/visible delay against all-WASM chain with same topology (swap hardware slot to WASM delay).
5. Measured added delay should fall within published budget ± one buffer period (~5.8 ms).
6. Record platform, bridge URL, and soak drop counters (`hardwareAdapter.soakSummary()` in browser console during connected session).

### Mixed-chain audition guidance

Compare hardware vs all-WASM chains with the **same module type** on the hardware slot:

1. Build an all-WASM reference: e.g. gateway → passthrough → delay → passthrough (loopback on rightmost if needed).
2. Swap the delay unit to **hardware slot**, flash matching firmware, connect bridge.
3. Expect **additional end-to-end latency** per budget table above.
4. Merger or timing-sensitive audition through hardware may differ from all-WASM — use passthrough neighbors for A/B of device DSP only.

### Phase 3 soak matrix

Minimum **10 minutes** per scenario with connected board. Headless bridge relay: `node sim/hardware-bridge/phase3-soak.mjs --scenario S2`. Full PWA mixed chain: configure topology in host UI per scenario ID (S1–S6 in `design.md`), run audio 10+ min, verify zero sustained drops.

| ID | Topology summary |
|----|------------------|
| S1 | Max chain, HW end, passthrough |
| S2 | GW + WASM + HW delay + WASM |
| S3 | GW + HW merger + 2 WASM |
| S4 | GW + WASM + HW cutoff + WASM merger (loopback) |
| S5 | GW + WASM + HW debug_tone + WASM |
| S6 | Max interleave, HW middle, delay |

## Limitations

| Area | Current behavior | Notes |
|------|------------------|-------|
| Startup mute | Not modeled (~1 s silence on device boot) | — |
| Render quantum | Web Audio 128-frame quantum vs 128-sample firmware buffer | See `host/AUDIOWORKLET_CPU.md` |
| Hardware slots | Single slot only | Phase 4+ |
| USB latency | Published budget + tolerance; recovery badges on hardware card | Phase 3 hardening |
| Web Serial | Not available | Phase 4 |
| Undo/redo | Delete and reorder are immediate | Future UX |
| Gateway module | I/O shim only | Optional |

**Hardware integration deltas (known vs all-WASM):** added USB and bridge round-trip latency, async ring pipeline (3–4 periods), published mixed-chain latency budget and ±1 period tolerance, single hardware slot limit, physical-pot authoritative control with optional telemetry display.

Merger cross-path timing uses decoupled per-path delay state in the chain host (Phase 2 parity). Merger and hardware unit cards show **Underrun** / **Overrun** / **Sustained drop risk** badges per recovery policy.

## Manual acceptance matrix

| Scenario | How to verify |
|----------|---------------|
| **Passthrough chain** | Add 2–3 units, all passthrough, mic on — speech should pass through with minimal latency; In/Out meters on each unit show activity. |
| **Delay with loopback** | Single delay unit, enable loopback on rightmost card, mic on — audible repeats; primary pot changes delay time as smoothed value settles. |
| **Multi-unit mix** | Chain passthrough → merger (or cutoff) → delay; confirm distinct processing per slot and downstream propagation left-to-right. |
| **Merger loopback latency** | Chain passthrough → merger → delay with loopback on the delay unit; mic on. Confirm merge latency and feedback strength feel stable as pots settle; merger card may flash Underrun while rings fill, then steady state. Secondary pot high should emphasize delayed upstream blend on merger downstream output. |
| **Module type swap mid-playback** | Start audio, change one unit from passthrough to debug tone — audio briefly stops and resumes with the new module active; level histories clear during rebuild. |
| **Delete unit** | Add 2–3 units, start audio, delete the middle unit — card disappears, labels renumber, audio auto-restarts on remaining units. Delete on sole unit is disabled while running. |
| **Reorder units** | Add passthrough then debug tone, start audio, drag debug tone left of passthrough via handle — audible order follows new layout after rebuild. |
| **Pot live during playback** | Start audio, move primary pot — output changes without stop/rebuild. |
| **Touch reorder QA** | On iOS Safari and Android Chrome: drag handle reorders units without scrolling the page; sliders and module selector do not reorder. |
| **Hardware delay slot** | Bridge running, delay firmware flashed: passthrough → HW delay (connected) → passthrough, mic on, loopback optional — delay audible; twist device pots; WASM neighbors still process; no sustained Underrun badge. |

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
| Slot order swap | `verify-wasm.mjs`: scheduler slot reorder changes first-hop impulse routing |
| Delete/reorder DOM | Manual browser check (see acceptance matrix); handle uses `touch-action: none` |
| Bridge relay smoke | `hardware-bridge/integration-smoke.mjs`: 30 s sustained duplex |
| Phase 3 soak | `hardware-bridge/phase3-soak.mjs`: 10 min per scenario attestation |
| Processing budget | `hardware-bridge/processing-budget.mjs`: ESP32 <70% buffer period |

**Manual browser check:** Run the acceptance matrix above on [http://localhost:8080/host/](http://localhost:8080/host/) with `./sim/build-wasm.sh` completed and bridge running.
