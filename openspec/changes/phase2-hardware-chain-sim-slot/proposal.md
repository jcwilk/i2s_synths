## Why

Phase 0 offline A/B and Phase 1 sustained USB duplex prove module parity and transport at the universal 22.05 kHz mono bridge geometry, but operators still cannot hear real hardware pots and DSP in the context of a mixed WASM+device chain. Phase 2 closes that gap by substituting one browser simulator slot with a physical ESP32 module so audition reflects true ADC control and on-device processing alongside neighboring WASM units at the same mono contract.

## What Changes

- Add a **local bridge server** that relays binary duplex exchanges between the PWA chain scheduler and a connected ESP32 over USB (default path for browsers without Web Serial).
- Extend **hardware-module-bridge** with bridge-server role, PWA session integration, and hardware-slot semantics (one physical unit, real pots, module-type match to flashed firmware).
- Extend **module-chain-simulator** so the operator can mark one processing unit as hardware, connect/disconnect the bridge, and run a mixed chain where that slot is async (pipeline delay / ring buffers) while WASM neighbors keep existing wiring rules.
- MVP hardware module kind: **delay** (stateful effect representative of chain integration challenges).
- Document added USB round-trip latency as a known operator-visible delta versus all-WASM chains.

**Non-goals (this change):**

- Multiple simultaneous hardware slots
- Web Serial direct browser-to-device path (Phase 4)
- Full-rate realtime hardening across long mixed chains (Phase 3)
- Merger module hardware acceptance or exhaustive all-module-type hardware coverage
- Compression or alternate sample-rate contracts outside the universal mono 22.05 kHz geometry
- Replacing Phase 0 offline harness or Phase 1 host reference tool (complementary)

## Capabilities

### New Capabilities

<!-- None — bridge server and hardware slot extend existing domains -->

### Modified Capabilities

- `hardware-module-bridge`: Add local bridge server (WebSocket ↔ USB relay), PWA duplex session lifecycle, hardware-slot control semantics (real ADC pots, module-type parity with firmware), and delay-module realtime acceptance in the simulator integration context — building on Phase 0 buffer geometry and Phase 1 realtime USB duplex contract.
- `module-chain-simulator`: Add hardware unit substitution (one slot among WASM neighbors), operator connect/disconnect and hardware marking controls, async slot pipeline behavior in chain wiring, and documented latency delta versus pure simulator chains.

## Impact

- **Specs:** Deltas to `hardware-module-bridge` and `module-chain-simulator` living specs after archive.
- **Firmware:** Delay module build in USB neighbor mode with physical ADC for controls (not host-injected sliders for the hardware slot); I2S neighbor paths remain disabled in USB neighbor mode per Phase 1.
- **Bridge server:** New local Node process relaying WebSocket binary frames to USB CDC bulk per `design.md` / `tasks.md`.
- **PWA:** Chain scheduler and unit cards gain hardware mode, bridge connection UI, and async slot buffering without blocking the audio worklet on USB I/O.
- **Dependencies:** Requires Phase 1 realtime USB duplex protocol and acceptance thresholds (`phase1-hardware-chain-usb-protocol`). Phase 0 offline parity (`phase0-hardware-chain-offline-ab`) provides confidence for delay module behavior before live integration.
- **Hardware:** ESP32-S3-Zero flashed with delay module firmware; integration acceptance uses microphone-fed chain audition with at least one WASM neighbor.
