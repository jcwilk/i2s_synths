## Required for this change

## 1. Universal mono 22.05 kHz geometry

- [ ] 1.1 Align firmware `SAMPLE_RATE` and `BUFFER_LEN` to 22.05 kHz and 128 mono samples per path
- [ ] 1.2 Update I2S configuration and module loop sample counts to match mono geometry
- [ ] 1.3 Regenerate offline harness test vectors and golden references for mono 22.05 kHz
- [ ] 1.4 Re-run offline passthrough and delay acceptance at new geometry

## 2. Firmware USB neighbor mode

- [x] 2.1 Add explicit enter/exit lifecycle for USB neighbor mode distinct from offline neighbor mode and normal I2S
- [x] 2.2 Disable physical I2S neighbor capture/playback when USB neighbor mode is active; restore on exit
- [x] 2.3 Implement per-exchange handler: accept downstream and upstream inputs plus primary/secondary controls, invoke upstream then downstream module loops, return downstream and upstream outputs with status
- [x] 2.4 Use host-injected control values instead of ADC reads during USB realtime exchange
- [x] 2.5 Verify normal boot and I2S streaming behavior when USB neighbor mode is not entered

## 3. Realtime binary frame protocol

- [ ] 3.1 Consolidate spike mono exchange into primary USB realtime command using mono 22.05 kHz geometry (retire stereo 44.1 kHz wire layout)
- [x] 3.2 Implement length-prefixed binary framing on USB CDC bulk audio interface with bounds checking
- [x] 3.3 Implement sequence echo and sequence-gap detection with status reporting
- [x] 3.4 Implement overrun/underrun status paths for backpressure reporting
- [x] 3.5 Add device-side loopback or identity self-test control frame to validate transport before module processing

## 4. Logging coexistence

- [x] 4.1 Implement dual-interface strategy (binary audio CDC separate from diagnostic logging) or compile-time exclusive binary mode per `design.md`
- [x] 4.2 Document host procedure for quiescing or redirecting logs before sustained acceptance runs
- [x] 4.3 Verify sustained frame parsing with no text corruption on the audio channel during acceptance configuration

## 5. Host reference duplex tool

- [x] 5.1 Create standalone host tool that opens the binary USB audio interface and sends enter/exit USB neighbor commands
- [ ] 5.2 Implement sustained exchange loop at ~5.8 ms cadence (128 samples ÷ 22050 Hz) with deterministic test patterns
- [ ] 5.3 Record per-exchange round-trip latency, realtime ratio, median and ninety-ninth percentile
- [x] 5.4 Count drops (overrun, underrun, missing response within deadline, sequence gap) and fail on any drop
- [x] 5.5 Verify passthrough bit-exact identity (ds_out == ds_in, us_out == us_in) on every exchange during sustained run
- [ ] 5.6 Emit structured pass/fail summary with exchange count, drop count, realtime ratio, and latency percentiles per `hardware-module-bridge` delta spec
- [x] 5.7 Implement persistent framed serial reader so sustained small-frame sessions do not desync (spike finding)

## 6. Build and hardware realtime acceptance

- [ ] 6.1 Compile firmware with passthrough module selection and mono geometry; confirm clean build via project standard compile entrypoint
- [ ] 6.2 When ESP32-S3-Zero board is connected: flash passthrough build, run transport loopback/identity check at mono geometry
- [ ] 6.3 When board is connected: run 60 s sustained realtime acceptance session (≥10,000 exchanges, zero drops, realtime ratio ≥1.0, latency within `design.md` thresholds)
- [ ] 6.4 Record hardware attestation notes (host OS, metrics observed, pass/fail) in apply verification output

## Explicitly deferred

- PWA chain UI hardware slot and bridge server integration (Phase 2)
- Full-rate realtime hardening under full simulator chain load (Phase 3)
- Web Serial direct path, reconnect policy, and runtime I2S-vs-USB mode switching (Phase 4)
- Realtime USB acceptance for delay, merger, cutoff, and debug-tone module kinds
- Depth-2 or deeper exchange pipelining
- WiFi transport or compression
- Parallel 44.1 kHz stereo bridge contract
- CI hardware farm automation (manual board attestation sufficient for Phase 1 apply-complete)
