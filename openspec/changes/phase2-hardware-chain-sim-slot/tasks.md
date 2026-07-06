## Required for this change

## 1. Local bridge server

- [ ] 1.1 Create Node bridge server process with WebSocket listener on documented local port and single-client session model
- [ ] 1.2 Implement USB CDC bulk attachment to Phase 1 binary audio interface with device enter/exit USB neighbor lifecycle on session start/stop
- [ ] 1.3 Relay complete duplex exchanges transparently (four paths at 128 int16 mono per path, 22.05 kHz) between WebSocket binary frames and USB without sample-rate or channel conversion
- [ ] 1.4 Expose connection status (bridge reachable, device attached, session active) via control channel messages to the PWA
- [ ] 1.5 Document bridge start command, default URL, and troubleshooting for offline bridge in bridge README

## 2. PWA hardware slot UI and session

- [ ] 2.1 Add per-unit hardware designation toggle with at-most-one enforcement across the chain
- [ ] 2.2 Add connect/disconnect controls and status chips on hardware-designated unit cards; hide or disable sim pot sliders when hardware is connected
- [ ] 2.3 Validate module kind parity before sustained relay; block connect and show mismatch when firmware kind disagrees with slot selection
- [ ] 2.4 Wire bridge WebSocket client with session lifecycle aligned to `hardware-module-bridge` PWA duplex requirements
- [ ] 2.5 Treat hardware designation, connect, and disconnect as structural reconfiguration per `module-chain-simulator` spec

## 3. Chain scheduler async hardware adapter

- [ ] 3.1 Implement bounded ring buffers between AudioWorklet and async exchange worker so the worklet never blocks on USB or WebSocket I/O
- [ ] 3.2 Schedule exchanges at firmware buffer period cadence (~5.8 ms) from a non-worklet context; handle underrun with silence substitution and visible indicator
- [ ] 3.3 Substitute hardware slot processing for WASM instance when designated and connected; retain separate WASM instances for all other occupied slots
- [ ] 3.4 Integrate hardware slot into existing downstream left-to-right and upstream right-to-left wiring with documented additional pipeline delay from ring depth
- [ ] 3.5 Feed exchanged audio into hardware slot peak-level metering histories during connected playback

## 4. Firmware delay module in PWA hardware mode

- [ ] 4.1 Add PWA hardware slot control source to USB neighbor mode: physical ADC with firmware-aligned EMA; ignore exchange payload control fields in this mode
- [ ] 4.2 Implement firmware module-kind reporting for bridge session validation (delay build for acceptance)
- [ ] 4.3 Ensure normal I2S boot and Phase 1 reference-tool injected-control mode remain unchanged when PWA hardware mode is not active
- [ ] 4.4 Compile delay module USB neighbor build and confirm clean build via project standard compile entrypoint

## 5. Documentation and known deltas

- [ ] 5.1 Update simulator documentation to list hardware-specific deltas: USB round-trip latency, bridge relay, and single hardware slot limit
- [ ] 5.2 Document mixed-chain audition guidance for operators comparing hardware slot vs all-WASM chains

## 6. Integration acceptance

- [ ] 6.1 With bridge running and delay firmware connected: configure chain with gateway, WASM neighbor, hardware delay slot, WASM neighbor; connect hardware session
- [ ] 6.2 Enable microphone input; run audio for at least 30 seconds; confirm delay effect audible and physical pots change processing
- [ ] 6.3 Confirm WASM neighbors remain audible in mixed chain and no sustained drop or underrun indicators during acceptance run
- [ ] 6.4 Record acceptance attestation (platform, bridge URL, chain layout, duration, pass/fail) in task completion notes or PR description

## Explicitly deferred

- Web Serial direct browser-to-device path (Phase 4)
- Multiple simultaneous hardware slots or multi-board routing
- Realtime hardware acceptance for merger, cutoff, debug-tone, and passthrough beyond bridge relay smoke test
- Phase 3 full-rate realtime hardening for long mixed WASM+HW chains under worst-case load
- Automated CI hardware farm for integration acceptance (manual board attestation sufficient for Phase 2 apply-complete)
- WiFi transport or compression
