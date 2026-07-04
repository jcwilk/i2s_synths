# Hardware offline A/B harness (Phase 0)

Deterministic host tool for validating ESP32-S3 firmware module outputs against simulator WASM golden references over USB CDC binary framing.

## Prerequisites

- Node.js ≥ 20
- WASM artifacts: `./sim/build-wasm.sh`
- For USB runs: connected ESP32-S3-Zero flashed with matching module build

Install harness dependencies once:

```bash
npm install --prefix sim/hardware-offline
```

## Warm-up period exclusion

Stateful modules (delay, merger, etc.) need leading periods before outputs are comparable. Each test vector JSON declares:

| Field | Meaning |
|-------|---------|
| `warmupPeriods` | Leading periods run on device but excluded from diff |
| `compareFromPeriod` | First period index included in pass/fail comparison (usually equals `warmupPeriods`) |

Passthrough vectors use `0` for both (stateless). Delay vectors use ~22 warm-up periods to fill the minimum tape span before impulses and control steps are compared.

## Tolerance policy

| Module kind | Rule |
|-------------|------|
| passthrough | Bit-exact (0 LSB deviation) |
| delay | ≤2 LSB max absolute per-sample deviation |

## Commands

Generate vectors and golden references from WASM:

```bash
node sim/hardware-offline/harness.mjs generate-vectors
```

Compare a saved device capture JSON against golden:

```bash
node sim/hardware-offline/harness.mjs compare-capture \
  sim/hardware-offline/vectors/passthrough.json \
  sim/hardware-offline/vectors/passthrough.capture.json
```

Run full USB offline A/B (set `FIRMWARE_PORT` or pass `--port`):

```bash
node sim/hardware-offline/harness.mjs run-usb \
  sim/hardware-offline/vectors/passthrough.json \
  --port /dev/ttyACM0
```

Validate transport loopback before module diff:

```bash
node sim/hardware-offline/harness.mjs loopback-usb --port /dev/ttyACM0
```

## Frame contract

One exchange carries four audio paths at 512 int16 samples each (stereo interleaved), plus primary/secondary controls (float32 0..1), sequence index, and status flags. See `design.md` in the OpenSpec change and `src/hardware_bridge/bridge_frame.h`.

## File replay

`run-usb` writes `<vector>.capture.json` after each hardware run. Use `compare-capture` to re-diff captures without reconnecting the board.
