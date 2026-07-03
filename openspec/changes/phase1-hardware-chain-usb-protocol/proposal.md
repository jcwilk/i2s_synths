## Why

Phase 0 proves module output parity offline, but the hardware-in-loop series still needs sustained full-fidelity duplex transport over USB before any PWA chain integration. Phase 1 de-risks realtime work by proving the host can drive buffer cadence at 44.1 kHz stereo with raw int16 PCM round-trip through a passthrough module build, measuring latency and drop behavior under host clock mastery.

## What Changes

- Extend **hardware-module-bridge** with realtime USB duplex requirements: framed binary exchange of four audio paths per buffer period at firmware cadence (~5.8 ms per path set, 512 int16 stereo samples per path).
- Add firmware **USB neighbor mode** in which physical I2S neighbor paths are disabled, the host is clock master, and the device processes one dual-path period per exchange using the passthrough module.
- Add a **host reference tool** (not PWA) that sustains duplex exchanges, records throughput/latency/drop metrics, and validates passthrough identity behavior under load.
- Define acceptance thresholds for sustained exchange rate, maximum round-trip latency, and drop count (details in `design.md`).
- Establish logging coexistence strategy so diagnostic text does not corrupt the audio binary pipe.

**Non-goals (this change):**

- PWA chain UI or hardware slot in the browser simulator (Phase 2)
- Bridge server, effect modules beyond passthrough acceptance, or delay/merger realtime coverage
- Web Serial direct path, reconnect policy, or runtime I2S-vs-USB mode switching (Phase 4)
- WiFi transport, compression, downsampling, or mono fallback
- Replacing Phase 0 offline A/B harness (complementary; Phase 1 can proceed in parallel once Phase 0 frame concepts exist)

## Capabilities

### New Capabilities

<!-- None — realtime USB behavior extends the existing hardware-module-bridge domain -->

### Modified Capabilities

- `hardware-module-bridge`: Add realtime USB duplex exchange, USB neighbor operating mode, host-driven clock cadence, sustained throughput and latency acceptance, and passthrough realtime validation — building on Phase 0 offline buffer geometry and four-path contract.

## Impact

- **Specs:** Delta to `hardware-module-bridge` living spec after archive (extends Phase 0 requirements).
- **Firmware:** New USB neighbor operating mode alongside offline neighbor and normal I2S; passthrough module only for Phase 1 acceptance; I2S to physical neighbors disabled in USB neighbor mode.
- **Host tooling:** New reference duplex driver for sustained binary exchange and metrics (implementation in `design.md` / `tasks.md`).
- **Dependencies:** Conceptually builds on Phase 0 buffer geometry and dual-path order (`phase0-hardware-chain-offline-ab`); may be applied in parallel after Phase 0 protocol design is stable. Requires ESP32-S3 native USB CDC bulk capacity at ~5.6 Mbps sustained for the four-path frame payload.
- **Hardware:** ESP32-S3-Zero with passthrough firmware build; realtime acceptance requires connected board and exclusive binary audio pipe during test.
