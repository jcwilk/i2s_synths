## Required for this change

## 1. Firmware offline neighbor mode

- [x] 1.1 Add explicit enter/exit lifecycle for offline neighbor mode that bypasses live I2S input and processes host-supplied buffer periods
- [x] 1.2 Implement one exchange handler: accept downstream and upstream inputs plus primary/secondary controls, invoke upstream then downstream module loops, return downstream and upstream outputs and status
- [x] 1.3 Ensure injected control values are used instead of ADC reads during offline exchange; preserve module state across consecutive periods in the same session
- [x] 1.4 Verify normal boot and I2S streaming behavior when offline mode is not entered

## 2. Buffer frame contract and transport

- [x] 2.1 Define host↔device binary frame layout per `design.md` (four audio paths at 512 int16 per path, controls, sequence index, status)
- [x] 2.2 Implement USB CDC binary transport (or file-replay fallback) for submitting frames and receiving responses
- [x] 2.3 Add device-side loopback self-test mode (echo inputs) to validate transport before module diff

## 3. Host offline validation harness

- [x] 3.1 Create host tool that loads test vectors, drives multi-period exchange, captures outputs, and diffs against reference
- [x] 3.2 Integrate reference generation from simulator WASM headless run and/or precomputed golden vector files
- [x] 3.3 Implement comparison reporting: pass/fail, period index, path, max deviation per `hardware-module-bridge` spec
- [x] 3.4 Apply tolerance policy: bit-exact for passthrough; ≤2 LSB max absolute deviation for delay

## 4. Test vectors and acceptance

- [x] 4.1 Add standard passthrough offline test vector (deterministic non-silent inputs, constant controls) with golden reference
- [x] 4.2 Add standard delay offline test vector with documented warm-up periods, length/speed control coverage, and golden reference
- [x] 4.3 Document warm-up period exclusion rules for stateful modules in harness README or inline manifest metadata

## 5. Build and hardware verification

- [x] 5.1 Compile firmware with passthrough module selection and confirm clean build via project standard compile entrypoint
- [x] 5.2 Compile firmware with delay module selection and confirm clean build via project standard compile entrypoint
- [x] 5.3 When ESP32-S3-Zero board is connected: flash passthrough build, run offline harness, confirm bit-exact pass — evidence: `node sim/hardware-offline/harness.mjs run-usb sim/hardware-offline/vectors/passthrough.json --port /dev/ttyACM0` → `PASS passthrough: 6 periods from index 0` (2026-07-03)
- [x] 5.4 When board is connected: flash delay build, run offline harness, confirm pass within delay tolerance — evidence: `node sim/hardware-offline/harness.mjs run-usb sim/hardware-offline/vectors/delay.json --port /dev/ttyACM0` → `PASS delay: 18 periods from index 22` (2026-07-03)

## Explicitly deferred

- Real-time USB duplex audio streaming (Phase 1 hardware-in-loop)
- Bridge server and hardware slot in PWA chain simulator (Phase 2)
- Sustained full-rate realtime hardening, compression, or downsampling (Phase 3)
- Web Serial reconnect and I2S-vs-USB runtime mode switch (Phase 4)
- Offline acceptance vectors for merger, cutoff, and debug-tone module kinds (add when those kinds are prioritized)
- PWA or browser UI integration for hardware A/B
- CI hardware farm automation (manual board attestation sufficient for Phase 0 apply-complete)
