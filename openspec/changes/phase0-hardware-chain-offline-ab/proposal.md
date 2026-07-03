## Why

Real-time USB duplex between a browser chain simulator and an ESP32-S3 module slot is high risk until firmware module outputs are proven equivalent to simulator reference outputs for identical inputs. Phase 0 de-risks later realtime work by validating module parity in a deterministic offline loop at full 44.1 kHz stereo fidelity before any sustained streaming or PWA integration.

## What Changes

- Introduce a **hardware-module-bridge** capability spec for offline buffer exchange between a host harness and device firmware operating as a single chain neighbor (not a realtime stream).
- Add an offline validation harness that feeds fixed buffer frames (downstream input and upstream input per period), collects downstream and upstream outputs, and compares them against a simulator or golden-vector reference.
- Add firmware **offline neighbor mode** that processes one buffer period per host exchange using the same dual-path invocation order as normal operation, without requiring live I2S audio.
- MVP module coverage: **passthrough** and **delay** first; spec SHALL allow extending to all firmware module kinds without changing the buffer contract.
- Document tolerance policy (bit-exact vs small sample deviation) and reference-generation strategy in `design.md`.

**Non-goals (this change):**

- Real-time USB audio streaming or sustained duplex throughput
- PWA chain UI integration or a hardware slot in the browser simulator
- Bridge server, Web Serial reconnect, or I2S-vs-USB runtime mode switching (Phases 1–4 of the hardware-in-loop series)

## Capabilities

### New Capabilities

- `hardware-module-bridge`: Offline, deterministic exchange of firmware-format audio buffer frames between a host validation harness and one ESP32-S3 module running in offline neighbor mode, with reference comparison proving module output parity against the module chain simulator for identical inputs and control state.

### Modified Capabilities

<!-- None — firmware module processing contracts and simulator behavior are referenced but not altered -->

## Impact

- **Specs:** New `hardware-module-bridge` living spec after archive.
- **Firmware:** New offline neighbor operating mode and entry/exit lifecycle alongside existing I2S streaming; no change to normal on-device audio behavior when offline mode is not active.
- **Host tooling:** New offline harness for buffer injection, capture, diff, and pass/fail reporting (implementation in `design.md` / `tasks.md`).
- **Dependencies:** Builds on existing firmware-build toolchain and module-chain-simulator equivalence assumptions (same module sources, same buffer period geometry). **No dependency on later hardware-in-loop phases.** Phases 1–4 (USB protocol spike, bridge server, realtime hardening, Web Serial) depend on successful offline parity from this change.
- **Hardware:** ESP32-S3-Zero with compile-time module selection; offline test requires a connected board when hardware verification tasks run.
