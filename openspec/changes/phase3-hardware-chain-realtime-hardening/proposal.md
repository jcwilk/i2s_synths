## Why

Phase 2 proves a single hardware delay slot can participate in a mixed WASM+device chain at the universal 22.05 kHz mono geometry, but short integration runs do not establish production-grade stability under realistic load. Phase 3 hardens sustained full-rate realtime operation so operators can trust extended auditions—long chains, merger loopback topologies, microphone input, and any simulator-supported module kind flashed on the board—without sustained frame drops, unpredictable latency, or chain-halting recovery behavior.

## What Changes

- Extend **hardware-module-bridge** with sustained mixed-chain acceptance thresholds, backpressure and recovery behavior that preserves chain continuity, optional pot telemetry from device to host for hardware-slot display, and realtime acceptance coverage for all simulator-supported module kinds (passthrough, delay, merger, cutoff, debug tone)—not delay alone.
- Extend **module-chain-simulator** with hardware-slot level metering parity with WASM units, documented and bounded added latency budget for mixed chains, stress acceptance scenarios (maximum chain length, merger with loopback and hardware in middle or end positions, microphone on), and extended soak duration requirements.
- Harden async hardware-slot buffering and exchange scheduling so USB and bridge round-trip variance does not cause sustained drops or audible glitches over multi-minute runs.
- Document operator-visible latency budget for hardware substitution and recovery semantics when transient underrun or overrun occurs.

**Non-goals (this change):**

- Web Serial direct browser-to-device path, reconnect polish, or runtime I2S-vs-USB mode switching (Phase 4)
- WiFi transport or alternate non-USB relay paths
- Multiple simultaneous hardware boards or hardware slots
- Compression or alternate sample-rate contracts outside the universal mono 22.05 kHz geometry
- Replacing Phase 1 host reference tool or Phase 0 offline harness (complementary)

## Capabilities

### New Capabilities

<!-- None — hardening extends existing domains -->

### Modified Capabilities

- `hardware-module-bridge`: Add sustained mixed-chain realtime acceptance (zero sustained drops over extended duration), bounded latency budget compliance, backpressure and recovery without chain halt, optional pot telemetry relay to the simulator, and per-module-kind hardware acceptance for all simulator-supported firmware module kinds.
- `module-chain-simulator`: Add hardware-slot metering presentation parity with WASM units, documented mixed-chain latency budget, stress acceptance matrix (long chains, merger loopback with hardware placement variants, microphone on), and extended soak test requirements for connected hardware slots.

## Impact

- **Specs:** Deltas to `hardware-module-bridge` and `module-chain-simulator` living specs after archive.
- **Firmware:** USB neighbor mode and PWA hardware-slot control source extended for all module kinds; optional telemetry of smoothed physical control readings alongside audio exchanges; recovery behavior under sustained load without requiring reflash.
- **Bridge server:** Relay scheduling and session stability under sustained PWA load; may pipeline exchanges while preserving full PCM geometry (details in `design.md`).
- **PWA:** Ring-buffer tuning, recovery UX, hardware-slot metering parity, pot telemetry display (optional), stress-test operator guidance, and latency budget documentation.
- **Dependencies:** Requires Phase 2 hardware slot integration (`phase2-hardware-chain-sim-slot`), Phase 1 realtime USB duplex contract (`phase1-hardware-chain-usb-protocol`), and Phase 0 offline parity baseline (`phase0-hardware-chain-offline-ab`).
- **Hardware:** ESP32-S3-Zero with per-module-kind firmware builds for acceptance matrix; extended soak attestation with connected board, bridge server, and microphone.
