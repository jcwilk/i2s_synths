## Required for this change

## 1. Firmware USB neighbor mode

- [x] 1.1 Add explicit enter/exit lifecycle for USB neighbor mode distinct from offline neighbor mode and normal I2S
- [x] 1.2 Disable physical I2S neighbor capture/playback when USB neighbor mode is active; restore on exit
- [x] 1.3 Implement per-exchange handler: accept downstream and upstream inputs plus primary/secondary controls, invoke upstream then downstream module loops, return downstream and upstream outputs with status
- [x] 1.4 Use host-injected control values instead of ADC reads during USB realtime exchange
- [x] 1.5 Verify normal boot and I2S streaming behavior when USB neighbor mode is not entered

## 2. Realtime binary frame protocol

- [x] 2.1 Define host↔device realtime frame layout per `design.md` (extend Phase 0 four-path geometry with sequence, status, optional timestamp)
- [x] 2.2 Implement length-prefixed binary framing on USB CDC bulk audio interface with bounds checking
- [x] 2.3 Implement sequence echo and sequence-gap detection with status reporting
- [x] 2.4 Implement overrun/underrun status paths for backpressure reporting
- [x] 2.5 Add device-side loopback or identity self-test control frame to validate transport before module processing

## 3. Logging coexistence

- [x] 3.1 Implement dual-interface strategy (binary audio CDC separate from diagnostic logging) or compile-time exclusive binary mode per `design.md`
- [x] 3.2 Document host procedure for quiescing or redirecting logs before sustained acceptance runs
- [x] 3.3 Verify sustained frame parsing with no text corruption on the audio channel during acceptance configuration

## 4. Host reference duplex tool

- [x] 4.1 Create standalone host tool that opens the binary USB audio interface and sends enter/exit USB neighbor commands
- [x] 4.2 Implement sustained exchange loop at ~5.8 ms cadence (512 samples ÷ 44100 Hz) with deterministic test patterns
- [x] 4.3 Record per-exchange round-trip latency; compute median and ninety-ninth percentile
- [x] 4.4 Count drops (overrun, underrun, missing response within deadline, sequence gap) and fail on any drop
- [x] 4.5 Verify passthrough bit-exact identity (ds_out == ds_in, us_out == us_in) on every exchange during sustained run
- [x] 4.6 Emit structured pass/fail summary with exchange count, drop count, and latency percentiles per `hardware-module-bridge` delta spec

## 5. Build and hardware realtime acceptance

- [x] 5.1 Compile firmware with passthrough module selection and confirm clean build via project standard compile entrypoint
- [x] 5.2 When ESP32-S3-Zero board is connected: flash passthrough build, run transport loopback/identity check
- [ ] 5.3 When board is connected: run 60 s sustained realtime acceptance session (≥10,000 exchanges, zero drops, latency within `design.md` thresholds)
- [ ] 5.4 Record hardware attestation notes (host OS, metrics observed, pass/fail) in apply verification output

## Explicitly deferred

- PWA chain UI hardware slot and bridge server integration (Phase 2)
- Full-rate realtime hardening under full simulator chain load (Phase 3)
- Web Serial direct path, reconnect policy, and runtime I2S-vs-USB mode switching (Phase 4)
- Realtime USB acceptance for delay, merger, cutoff, and debug-tone module kinds
- Depth-2 or deeper exchange pipelining (optimize only if measured host scheduling requires it)
- WiFi transport, compression, downsampling, or mono fallback
- CI hardware farm automation (manual board attestation sufficient for Phase 1 apply-complete)
