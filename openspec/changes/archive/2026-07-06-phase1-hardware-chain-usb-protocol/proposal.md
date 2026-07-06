## Why

Phase 0 proves module output parity offline, but the hardware-in-loop series still needs sustained duplex transport over USB before any PWA chain integration. Measured Phase 1 spike work shows full-rate 44.1 kHz stereo exceeds FS USB CDC throughput (~0.4× realtime). Phase 1 adopts a **universal 22.05 kHz mono** bridge contract—half sample rate, one int16 per path per sample—preserving ~5.8 ms buffer period while cutting wire bytes ~4× so sustained ≥1× realtime is achievable on target hardware.

## What Changes

- **Universal bridge geometry:** 22.05 kHz sample rate, mono int16 per audio path, 128 samples per path per exchange (~5.8 ms period)—applies to offline neighbor mode, USB realtime neighbor mode, and future PWA/bridge relay (supersedes Phase 0 stereo 44.1 kHz / 512-sample geometry on archive).
- Extend **hardware-module-bridge** with realtime USB duplex requirements using the mono contract above.
- Add firmware **USB neighbor mode** in which physical I2S neighbor paths are disabled, the host is clock master, and the device processes one dual-path period per exchange using the passthrough module.
- Add a **host reference tool** that sustains duplex exchanges, records throughput/latency/drop metrics, and validates passthrough identity under load.
- Define acceptance thresholds for sustained exchange rate (≥1× realtime), round-trip latency within one buffer period, and zero drops (details in `design.md`).
- Establish logging coexistence strategy so diagnostic text does not corrupt the audio binary pipe.
- Regenerate offline harness vectors and golden references for the new geometry; re-validate passthrough and delay offline acceptance under mono 22.05 kHz.

**Non-goals (this change):**

- PWA chain UI or hardware slot in the browser simulator (Phase 2)
- Bridge server, effect modules beyond passthrough realtime acceptance, or merger realtime coverage
- Web Serial direct path, reconnect policy, or runtime I2S-vs-USB mode switching (Phase 4)
- WiFi transport or compression
- Maintaining a parallel 44.1 kHz stereo USB contract alongside mono 22.05 kHz

## Capabilities

### New Capabilities

<!-- None — realtime USB behavior extends the existing hardware-module-bridge domain -->

### Modified Capabilities

- `hardware-module-bridge`: Replace Phase 0 buffer geometry with universal 22.05 kHz mono; add realtime USB duplex exchange, USB neighbor operating mode, host-driven clock cadence, sustained throughput and latency acceptance, and passthrough realtime validation.

## Impact

- **Specs:** Delta to `hardware-module-bridge` living spec after archive (MODIFIED offline geometry from Phase 0; ADDED realtime USB requirements).
- **Firmware:** Shared `SAMPLE_RATE` / `BUFFER_LEN` alignment to mono 22.05 kHz / 128 samples; USB neighbor mode; passthrough module for Phase 1 acceptance; I2S to physical neighbors disabled in USB neighbor mode.
- **Host tooling:** Reference duplex driver and offline harness updated for mono geometry; spike benchmark retained for regression comparison.
- **Dependencies:** Phase 0 offline concepts carry forward; geometry changes require vector regeneration before offline re-attestation.
- **Hardware:** ESP32-S3-Zero with passthrough firmware build; realtime acceptance requires connected board and exclusive binary audio pipe during test.
