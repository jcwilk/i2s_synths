## Required for this change

## 1. Ring buffers and recovery policy

- [ ] 1.1 Increase default hardware-slot ring depth to three periods each direction per `design.md`; implement adaptive increase to four periods when sustained underrun threshold exceeded
- [ ] 1.2 Implement transient underrun recovery: silence substitution on affected hardware paths, WASM neighbors continue, auto-clear indicator after three consecutive good periods
- [ ] 1.3 Implement transient overrun recovery: drop oldest non-in-flight pending exchange, continue scheduling, visible overrun count for soak summary
- [ ] 1.4 Implement sustained underrun detection (>5% of periods over 30 s window) with persistent warning and soak fail semantics
- [ ] 1.5 Implement graceful teardown on sequence gap or fatal device status without requiring reflash

## 2. USB pipelining and bridge stability

- [ ] 2.1 Spike depth-2 in-flight USB exchanges on bridge path when single-flight RTT p50 exceeds 1.5× buffer period; preserve full PCM geometry and strict sequence matching
- [ ] 2.2 Harden bridge server relay loop for sustained PWA load; measure p99 relay processing time per `design.md` CPU budget
- [ ] 2.3 Document OS-specific pipelining enablement and ring depth recommendations in bridge README

## 3. Latency budget documentation and measurement

- [ ] 3.1 Publish mixed-chain latency budget (ring occupancy, USB+bridge planning allowance, inter-unit delays) in simulator and bridge documentation
- [ ] 3.2 Implement or document acceptance measurement procedure for reference topology `[gateway, WASM passthrough, HW delay, WASM passthrough]` within ±1 buffer period tolerance
- [ ] 3.3 Update simulator known-limitations section with published budget and tolerance per `module-chain-simulator` delta

## 4. Pot telemetry relay

- [ ] 4.1 Extend device exchange status to include smoothed primary and secondary physical control readings (normalized 0..1) on each exchange or documented cadence
- [ ] 4.2 Relay telemetry through bridge to PWA without additional round trip
- [ ] 4.3 Add optional read-only telemetry display on hardware unit card; keep physical pots authoritative and sim sliders disabled when connected
- [ ] 4.4 Verify telemetry display does not alter device processing control source in PWA hardware slot mode

## 5. Hardware-slot metering parity

- [ ] 5.1 Ensure hardware-slot peak histories use same log-scaled presentation and short/long windows as WASM units
- [ ] 5.2 Enforce buffer-aligned peak sampling (at most one peak per path per firmware buffer period) for hardware exchanged audio
- [ ] 5.3 Verify hardware and WASM meters advance independently and continuously during connected playback without structural rebuild

## 6. Firmware all-module-kind support

- [ ] 6.1 Extend PWA hardware slot control source (physical ADC, ignore injected controls) to passthrough, merger, cutoff, and debug tone USB neighbor builds
- [ ] 6.2 Verify module-kind reporting for all supported kinds used in acceptance matrix
- [ ] 6.3 Compile each module kind USB neighbor build and confirm clean build via project standard compile entrypoint
- [ ] 6.4 Measure ESP32 per-exchange processing time in diagnostic build; confirm within 70% buffer period budget

## 7. PWA exchange worker and CPU budget

- [ ] 7.1 Harden async exchange worker scheduling at firmware buffer cadence under max-chain mixed load
- [ ] 7.2 Profile AudioWorklet p99 render quantum usage during max-chain soak; document mitigation if >50% budget
- [ ] 7.3 Integrate sustained-drop and recovery indicators into hardware unit card UX per `design.md` audibility policy

## 8. Extended soak and stress acceptance

- [ ] 8.1 Execute S1: 10 min max chain, hardware end, passthrough, mic on — record pass/fail and drop count
- [ ] 8.2 Execute S2: 10 min GW + WASM + HW delay + WASM, mic on, pots adjusted — record pass/fail
- [ ] 8.3 Execute S3: 10 min GW + HW merger + 2 WASM, mic on — record pass/fail and merger recovery behavior
- [ ] 8.4 Execute S4: 10 min GW + WASM + HW cutoff + WASM merger (loopback), mic on — record pass/fail
- [ ] 8.5 Execute S5: 10 min GW + WASM + HW debug_tone + WASM — record pass/fail
- [ ] 8.6 Execute S6: 10 min max interleave with hardware middle, delay, mic on — record pass/fail
- [ ] 8.7 Record structured attestation per matrix row (platform, bridge URL, chain layout, duration, drop count, latency notes, pass/fail) in task completion notes or PR description

## Explicitly deferred

- Web Serial direct browser-to-device path, reconnect polish, and runtime I2S-vs-USB mode switching (Phase 4)
- WiFi transport or alternate non-USB relay paths
- Multiple simultaneous hardware boards or hardware slots
- Compression, downsampling, mono fallback, or sample-rate conversion
- Automated CI hardware farm for 10-minute soaks (manual board attestation sufficient for apply-complete)
- Merger hardware acceptance with loopback on the hardware unit itself unless S4 investigation warrants a follow-up change
