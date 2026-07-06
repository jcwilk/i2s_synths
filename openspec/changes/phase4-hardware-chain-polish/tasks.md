## Required for this change

## 1. Shared transport adapter

- [x] 1.1 Define shared hardware transport interface used by async exchange worker (connect, disconnect, reconnect, submit exchange, link-loss callback) per `design.md` adapter architecture
- [x] 1.2 Refactor existing bridge WebSocket backend to implement the shared interface without changing full-rate exchange geometry or Phase 3 recovery integration
- [x] 1.3 Implement Web Serial backend on the shared interface using Phase 1 four-path exchange contract and host-driven cadence
- [x] 1.4 Enforce mutual exclusivity: Web Serial backend refuses connect when bridge session holds the same device port and vice versa, with visible port-busy messaging

## 2. Web Serial direct path

- [x] 2.1 Add browser capability detection; hide or disable Web Serial option when API unavailable
- [x] 2.2 Implement Web Serial port selection flow (browser picker) and permission-denied error handling
- [x] 2.3 Wire Web Serial session enter/exit USB neighbor actions through PWA without bridge server
- [x] 2.4 Verify pot telemetry relay and physical ADC control source semantics on Web Serial path match bridge path

## 3. Reconnect state machine

- [x] 3.1 Implement session states ACTIVE, DEGRADED, RECONNECTING, DISCONNECTED per `design.md` state machine
- [x] 3.2 On unexpected link loss, transition to DEGRADED with Phase 3 silence substitution on hardware paths while WASM neighbors continue
- [x] 3.3 Implement operator reconnect action from DEGRADED (re-enter USB neighbor, validate module kind, resume exchanges) without full browser restart
- [x] 3.4 Implement bounded auto-retry (3 attempts, 2 s backoff) for abrupt disconnect with visible attempt counter; disable on fatal errors
- [x] 3.5 On fatal recovery errors (module-kind mismatch, sequence gap policy), transition to clean DISCONNECTED with actionable message

## 4. Simulator transport and reconnect UI

- [x] 4.1 Add transport path selector on hardware unit card (Web Serial versus bridge); default to bridge when Web Serial unavailable
- [x] 4.2 Block transport switch while session is active or degraded until disconnect completes
- [x] 4.3 Add connection-lost indication and reconnect button on hardware unit card when session is DEGRADED
- [x] 4.4 Extend connection status display for transport availability, active session, and degraded-awaiting-reconnect states
- [x] 4.5 Confirm reconnect is not structural reconfiguration (chain layout preserved; no full graph rebuild solely for reconnect)

## 5. Firmware mode selection and documentation

- [x] 5.1 Document normal I2S chain mode versus USB neighbor dev mode in operator guide with cold-boot default behavior
- [x] 5.2 Verify orderly USB neighbor enter on session start and exit on disconnect/reconnect teardown without reflash
- [x] 5.3 Document BOOT button upload workflow versus runtime mode selection in troubleshooting section
- [x] 5.4 If compile-time default-to-USB-neighbor dev flag exists, guard as dev-only and exclude from release build checklist

## 6. Operator documentation

- [x] 6.1 Publish operator setup guide covering quick start, transport comparison summary, and mode selection
- [x] 6.2 Publish troubleshooting section: port conflicts (upload vs PWA vs bridge), Web Serial permission denied, bridge not running, reconnect after unplug, module-kind mismatch
- [x] 6.3 Update simulator known-limitations with transport availability, reconnect behavior, port exclusivity, and link to operator guide
- [x] 6.4 Update bridge README with Web Serial coexistence notes and port exclusivity guidance
- [x] 6.5 Cross-link Phase 3 mixed-chain latency budget for both transport paths where relay overhead differs

## 7. Manual QA matrix

- [x] 7.1 **Q1 Chrome Web Serial — connect and audition:** Chromium browser, Web Serial transport, reference topology `[gateway, WASM passthrough, HW delay, WASM passthrough]`, mic on, ≥2 min — record pass/fail, transport, platform
- [x] 7.2 **Q2 Chrome Web Serial — reconnect:** During Q1 topology, unplug USB briefly, use reconnect action, confirm resume without browser restart — record pass/fail
- [x] 7.3 **Q3 Chrome Web Serial — module kind parity:** Repeat Q1 with at least one additional module kind (passthrough or cutoff) — record pass/fail
- [x] 7.4 **Q4 Bridge fallback — Firefox or Safari:** Bridge transport only, same reference topology as Q1, ≥2 min — record pass/fail, browser, platform
- [x] 7.5 **Q5 Bridge fallback — reconnect:** During Q4, stop bridge or unplug USB, reconnect via bridge path without browser restart — record pass/fail
- [x] 7.6 **Q6 Port exclusivity:** With active Web Serial session, attempt upload or second connect — confirm visible port-busy guidance — record pass/fail
- [x] 7.7 **Q7 Mode selection doc walkthrough:** Cold boot device, confirm normal I2S default; start PWA session and confirm USB neighbor entry; disconnect and confirm return to normal I2S — record pass/fail
- [x] 7.8 Record structured attestation per matrix row (browser, transport, topology, duration, reconnect tested, pass/fail, notes) in task completion notes or PR description

## 8. Build verification

- [x] 8.1 Confirm firmware compile via project standard compile entrypoint after any mode-selection documentation or dev-flag guard changes
- [x] 8.2 Confirm PWA build succeeds with Web Serial transport module included

## Explicitly deferred

- WiFi transport or any non-USB relay path
- Multiple simultaneous hardware boards or hardware slots
- Compression or any change to the universal mono 22.05 kHz bridge geometry
- Preset chain configurations with hardware slot marker in JSON save/load (follow-up change unless apply discovers zero-cost fit)
- Automated CI matrix for Web Serial (manual browser attestation sufficient for apply-complete)
- 10-minute Phase 3 soak re-run on both transports (Phase 3 attestation remains authoritative; Phase 4 QA matrix uses shorter functional runs)
