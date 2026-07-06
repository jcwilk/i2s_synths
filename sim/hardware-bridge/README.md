# Local hardware bridge server (Phase 3)

Node.js process that relays binary duplex exchanges between the module chain simulator PWA and an ESP32 module over USB CDC.

## Prerequisites

- Node.js ≥ 20
- Module firmware flashed with USB neighbor support (any simulator module kind)
- `npm install --prefix sim/hardware-bridge`

## Start the bridge

```bash
npm start --prefix sim/hardware-bridge
```

Default WebSocket URL: **`ws://localhost:8765`**

Options:

| Flag / env | Purpose |
|------------|---------|
| `--port=8765` | WebSocket listen port |
| `--device=/dev/ttyACM0` | USB audio CDC port |
| `FIRMWARE_PORT` | Same as `--device` |
| `--verbose` | Log relay activity to stderr |

Example with explicit device:

```bash
FIRMWARE_PORT=/dev/ttyACM0 npm start --prefix sim/hardware-bridge
```

## Run the simulator

Serve the `sim/` directory on a **different** port (bridge uses 8765):

```bash
npx serve sim -p 8080
```

Open [http://localhost:8080/host/](http://localhost:8080/host/).

1. Mark one unit as **Hardware slot**
2. Select matching module type (e.g. delay)
3. Confirm bridge URL (`ws://localhost:8765`)
4. Click **Connect** on the hardware unit card
5. Start audio and enable microphone

## Protocol

- **JSON control** (text WebSocket frames): `ping`, `session` start/stop, `enumerate`, `status` responses (includes `relayP99Ms` when verbose status polled)
- **Binary relay** (binary WebSocket frames): length-prefixed USB wire frames — four mono int16 paths at 128 samples, 22.05 kHz
- **Telemetry** — exchange responses include `primaryTelemetry` / `secondaryTelemetry` (normalized 0..1) from physical ADC when in PWA mode

Session start sends `ENTER_USB` with PWA ADC control mode; stop sends `EXIT_USB`.

## Mixed-chain latency budget (Phase 3)

Published planning budget for hardware slot substitution (22.05 kHz mono, 128-sample buffer ≈ **5.8 ms** per period):

| Component | Budget (periods) | ~ms @ 5.8 ms/period |
|-----------|------------------|------------------------|
| Hardware slot ring target occupancy | 2 | ~12 |
| USB + bridge round-trip (p99 planning) | 3.5 | ~20 |
| Standard inter-unit path delay | 1 per neighbor | ~5.8 each |

**Example:** hardware slot between two WASM neighbors: **~2 + 3.5 = 5.5 periods (~32 ms)** plus per-neighbor delays in longer chains.

Acceptance tolerance: measured added delay within **±1 buffer period** for reference topology `[gateway, WASM passthrough, HW delay, WASM passthrough]` (see `sim/README.md` measurement procedure).

## USB pipelining and ring depth (OS guidance)

| OS | Pipelining | Default ring depth | Notes |
|----|------------|-------------------|--------|
| Linux | Auto when RTT p50 > 1.5× buffer period | 3 periods each direction | Adaptive **4** periods if sustained underrun >5% over 30 s |
| Windows | Enable pipelining (depth-2 in-flight) recommended | 3–4 periods | USB scheduling jitter often needs pipelining under mixed-chain CPU load |
| macOS | Same as Linux | 3 periods | Measure RTT in bridge status / soak harness |

Phase 3 bridge server pipelines up to **two** USB relays per client. PWA exchange worker may submit depth-2 in-flight before responses return; sequence matching is strict — sequence gap tears down session gracefully.

## Recovery semantics

| Event | Behavior |
|-------|----------|
| Transient inbound underrun | Silence on hardware paths; WASM continues; badge clears after 3 good periods |
| Transient outbound overrun | Drop oldest queued exchange (not in-flight); overrun count in soak summary |
| Sustained underrun (>5% / 30 s) | Persistent warning; soak fail |
| Sequence gap | Graceful hardware session teardown (no reflash) |

## Phase 3 soak harness

With bridge running and firmware connected:

```bash
# Single scenario (10 min default)
FIRMWARE_PORT=/dev/ttyACM0 node sim/hardware-bridge/phase3-soak.mjs --scenario S2

# Full matrix (requires reflash per module kind or accept module mismatch for relay-only rows)
node sim/hardware-bridge/phase3-soak.mjs --all --duration 600
```

Processing budget check:

```bash
FIRMWARE_PORT=/dev/ttyACM0 node sim/hardware-bridge/processing-budget.mjs
```

## Troubleshooting

| Symptom | Check |
|---------|--------|
| Bridge offline | Bridge process running? Correct URL in gateway card? |
| No device | USB cable, `FIRMWARE_PORT`, only one tool using the port |
| Module mismatch | Flash firmware for the module type selected on the hardware slot |
| Underrun / sustained badge | USB latency; ring depth 3–4; see OS table above |
| Sequence gap | Power-cycle not required — disconnect hardware slot and reconnect |
| Port conflict | Bridge on 8765; use `npx serve sim -p 8080` for static host |

## Single-client model

Only one WebSocket client may relay at a time. A second connection is rejected until the first disconnects.
