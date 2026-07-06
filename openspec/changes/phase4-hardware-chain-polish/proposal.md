## Why

Phases 0–3 establish mixed WASM+device chains with sustained realtime stability at the universal 22.05 kHz mono geometry, but daily development still depends on a local bridge server, manual recovery after cable or port events, and implicit knowledge of when to run normal I2S chain mode versus USB neighbor dev mode. Phase 4 reduces operator friction so Chrome and Edge users can connect directly, recover from disconnects without restarting the browser, and follow clear setup and troubleshooting guidance—without changing the mono bridge contract established in earlier phases.

## What Changes

- Add an optional **Web Serial direct transport** in the PWA so Chrome and Edge can exchange duplex audio at the universal mono 22.05 kHz geometry with the device without the Node bridge; the bridge remains the default path for browsers without Web Serial support.
- Add **session reconnect handling**: graceful disconnect detection, operator-visible status and recovery actions, and chain resumption without a full browser restart when the device or transport returns.
- Add **firmware operating-mode selection** so the operator can run the device in normal I2S chain mode or USB neighbor dev mode with documented boot and selection behavior.
- Add **operator documentation**: setup guide, troubleshooting (BOOT button, port conflicts, bridge versus Web Serial), and known deltas versus pure simulator chains.
- Extend **module-chain-simulator** with transport selection UI, reconnect UX, and documentation cross-links consistent with hardened hardware slot behavior from Phase 3.

**Non-goals (this change):**

- WiFi transport or any non-USB relay path
- Multiple simultaneous hardware boards or hardware slots
- Compression or any change to the universal 22.05 kHz mono bridge geometry
- Preset chain configurations that include a hardware slot marker (deferred unless apply reveals a natural fit)

## Capabilities

### New Capabilities

<!-- None — polish extends existing domains -->

### Modified Capabilities

- `hardware-module-bridge`: Add Web Serial direct PWA transport as an alternate path at the universal mono geometry; session reconnect and recovery semantics; firmware I2S-versus-USB-neighbor mode selection and documentation; port exclusivity guidance when upload tools and connected sessions compete for the same device.
- `module-chain-simulator`: Add transport selection UI (Web Serial versus bridge), reconnect operator feedback and recovery flow, and operator documentation for setup, troubleshooting, and known hardware-integration deltas.

## Impact

- **Specs:** Deltas to `hardware-module-bridge` and `module-chain-simulator` living specs after archive.
- **Firmware:** Documented mode selection (normal I2S chain versus USB neighbor dev); boot and operator selection behavior; unchanged mono 22.05 kHz PCM geometry when in USB neighbor mode.
- **Bridge server:** Remains default transport; no requirement to remove or replace; may gain clearer coexistence messaging when Web Serial is available.
- **PWA:** Web Serial transport adapter sharing the same exchange contract as bridge relay; reconnect state machine; transport picker and status UI; documentation pages or README sections for operators.
- **Dependencies:** Requires Phase 3 realtime hardening (`phase3-hardware-chain-realtime-hardening`), Phase 2 hardware slot integration (`phase2-hardware-chain-sim-slot`), Phase 1 USB duplex contract (`phase1-hardware-chain-usb-protocol`), and Phase 0 offline parity baseline (`phase0-hardware-chain-offline-ab`).
- **Hardware:** ESP32-S3-Zero; manual QA matrix covering Chrome Web Serial and bridge fallback on at least one non-Web-Serial browser.
