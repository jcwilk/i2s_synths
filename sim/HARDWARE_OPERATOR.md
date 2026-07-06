# Hardware operator guide (Phase 4)

Setup, transport selection, firmware mode, and troubleshooting for mixed WASM+device chains in the module chain simulator.

See also: [`README.md`](README.md) (simulator overview), [`hardware-bridge/README.md`](hardware-bridge/README.md) (bridge server), [Phase 3 mixed-chain latency budget](README.md#mixed-chain-latency-budget-phase-3).

---

## Quick start

1. Flash firmware for the module kind you want on the hardware slot (e.g. delay).
2. Serve the simulator: `npx serve sim -p 8080` → open [http://localhost:8080/host/](http://localhost:8080/host/).
3. Mark **one** processing unit as **Hardware slot** and set its module type to match firmware.
4. Choose **transport** on the unit card (see [Transport comparison](#transport-comparison)).
5. Click **Connect**, then **Start audio** and enable the microphone.
6. Audition the chain; physical device pots control the hardware slot when connected.

---

## Transport comparison

| | Bridge (WebSocket) | Web Serial direct |
|---|------------------|-------------------|
| Browsers | All (Firefox, Safari, Chrome, Edge) | Chrome, Edge (Chromium with Web Serial) |
| Host process | Node bridge on port 8765 | None — browser opens USB |
| Device enumeration | Bridge reports attached port | Browser port picker |
| Recommended for | Cross-browser QA, Safari/Firefox | Daily Chrome dev when not flashing |
| Latency | USB + bridge relay overhead | USB only (no bridge hop) |

**Default:** Bridge when Web Serial is unavailable; otherwise operator choice per session (not persisted across reload).

**Switching transport:** Disconnect the hardware session first. Active or degraded sessions block transport changes.

---

## Firmware mode selection

### Normal I2S chain mode (cold boot default)

On power-on with no host session, the device runs the physical I2S neighbor chain. USB may be idle or used for logging. The PWA cannot process audio through the device until a dev session starts.

### USB neighbor dev mode (PWA session)

When you **Connect** via bridge or Web Serial, the host sends **ENTER_USB** (PWA ADC mode — physical pots authoritative). I2S neighbor processing for the module is suspended; the host supplies buffer-period exchanges at 22.05 kHz mono.

On **Disconnect** or orderly session end, the host sends **EXIT_USB** and the device returns to normal I2S chain mode **without reflash**.

### BOOT button and upload

| Action | Purpose |
|--------|---------|
| Hold **BOOT** during USB connect / reset | Serial bootloader for **upload/flash** — not runtime mode selection |
| Cold boot, no host session | Normal I2S chain mode |
| PWA Connect | Enter USB neighbor dev mode |
| PWA Disconnect | Exit USB neighbor dev mode |

Do not confuse flash/bootloader mode with USB neighbor dev mode.

**Compile-time default-to-USB-neighbor:** Not present in release builds. `ACTIVE_MODULE` selects module kind only; mode is always host-driven at runtime.

---

## Troubleshooting

### Port busy / conflicts

Only **one** process may hold the USB serial port.

| Situation | What to do |
|-----------|------------|
| PWA Web Serial session active | Disconnect PWA before Arduino upload or serial monitor |
| Bridge session active | Stop bridge (`Ctrl+C`) or disconnect PWA before flash |
| Upload in progress | Wait for upload to finish, reset device, retry Connect |
| “Port busy” in PWA | Close other serial tools; disconnect the other transport |

Bridge and Web Serial are **mutually exclusive** on the same device.

### Web Serial permission denied

- Use Chrome or Edge on desktop (not Firefox/Safari).
- Grant serial permission when the port picker appears.
- On Linux, ensure your user is in the `dialout` group for `/dev/ttyACM*`.

### Bridge not running

- Start: `npm start --prefix sim/hardware-bridge`
- Confirm gateway **Bridge URL** matches (`ws://localhost:8765` default).
- Status chips should show “Bridge online” and “Device attached”.

### Reconnect after unplug

1. Session enters **Connection lost** (degraded); WASM neighbors keep running; hardware paths output silence.
2. Replug USB, wait for device boot (~1 s).
3. Click **Reconnect** on the hardware unit card (or wait for up to 3 auto-retries, 2 s apart).
4. No browser restart required; chain layout is preserved.

### Module kind mismatch

Slot module type must match flashed firmware. Mismatch after reconnect moves to clean **Disconnected** with an actionable error — fix firmware or slot type, then Connect again.

### Sequence gap

Disconnect and Connect again; power-cycle usually not required.

---

## Mixed-chain latency (both transports)

Published budget (22.05 kHz mono, 128 samples ≈ 5.8 ms/period):

| Component | Budget (periods) | ~ms |
|-----------|------------------|-----|
| Ring target occupancy | 2 | ~12 |
| USB round-trip (p99 planning) | 3.5 | ~20 |
| Bridge relay (bridge path only) | +0.5–1 | +3–6 |
| Inter-unit path delay | 1 per neighbor | ~5.8 each |

**Reference topology** `[gateway, WASM passthrough, HW delay, WASM passthrough]`: ~5.5 periods (~32 ms) added vs all-WASM, **±1 buffer period** tolerance.

Web Serial removes the bridge relay hop; expect slightly lower RTT than bridge on the same machine.

---

## Known deltas vs pure simulator

- USB and transport round-trip latency (budget above)
- Async ring pipeline (3–4 periods)
- Single hardware slot only
- Reconnect preserves layout; initial connect/disconnect triggers structural audio rebuild
- Web Serial: Chromium only; bridge required on Firefox/Safari
- Port exclusivity with upload tools and between transports
