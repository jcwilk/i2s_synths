# Phase 4 apply verification notes

**Change:** phase4-hardware-chain-polish  
**Branch:** explore/mono-22k-usb-spike @ apply time  
**Apply environment:** Linux CI agent — no ESP32-S3 attached, no Chromium with Web Serial to physical device

## Build / tooling evidence

| Task | Command | Result |
|------|---------|--------|
| 8.1 Firmware compile | `./scripts/build.sh passthrough` | exit 0 — 371515 bytes (28%) flash |
| 8.2 PWA build | `node sim/verify-wasm.mjs` | exit 0 — all WASM checks passed |
| OpenSpec validate | `npx @fission-ai/openspec@latest validate phase4-hardware-chain-polish --type change` | valid |

## Implementation evidence (selected)

| Area | Paths | Notes |
|------|-------|-------|
| Transport interface | `sim/host/hardware-transport.js` | States, port exclusivity, Web Serial detection |
| Bridge backend | `sim/host/bridge-client.js` | Session states, link-loss → DEGRADED, reconnect |
| Web Serial backend | `sim/host/web-serial-transport.js`, `web-serial-frame-reader.js` | Phase 1 four-path contract, enter/exit/query |
| Session coordinator | `sim/host/hardware-session-manager.js` | Auto-retry 3× / 2 s, fatal kind mismatch |
| Exchange worker | `sim/host/hardware-slot-adapter.js` | Generic transport; Phase 3 recovery preserved |
| UI | `sim/host/main.js`, `index.html` | Transport picker, degraded banner, reconnect |
| Operator docs | `sim/HARDWARE_OPERATOR.md` | Quick start, transport, mode, troubleshooting |
| Simulator docs | `sim/README.md` | Limitations, reconnect, port exclusivity links |
| Bridge docs | `sim/hardware-bridge/README.md` | Web Serial coexistence, latency delta |

## Task 5.x firmware mode

- **5.1–5.3:** Documented in `sim/HARDWARE_OPERATOR.md` (I2S default, USB neighbor session, BOOT upload).
- **5.2:** Enter/exit via `startSession`/`stopSession` on both transports (`buildEnterUsbFrame` / `buildExitUsbFrame`).
- **5.4:** No compile-time default-to-USB-neighbor flag in tree; `ACTIVE_MODULE` only.

## Manual QA matrix attestation (tasks 7.1–7.8)

Apply environment **cannot execute** browser+device scenarios. Structured rows for operator sign-off:

| ID | Browser | Transport | Topology | Duration | Reconnect | Pass/Fail | Notes |
|----|---------|-----------|----------|----------|-----------|-----------|-------|
| Q1 | Chromium (operator) | Web Serial | GW→WASM passthrough→HW delay→WASM passthrough, mic on | ≥2 min | — | **PENDING_OPERATOR** | Code paths implemented; needs live device |
| Q2 | Chromium | Web Serial | Same as Q1 | — | Unplug + Reconnect button | **PENDING_OPERATOR** | Degraded FSM + `reconnectHardware()` in main.js |
| Q3 | Chromium | Web Serial | + passthrough or cutoff kind | ≥2 min | — | **PENDING_OPERATOR** | Module parity in `HardwareSessionManager.startSession` |
| Q4 | Firefox or Safari | Bridge | Same topology as Q1 | ≥2 min | — | **PENDING_OPERATOR** | Bridge default when Web Serial unavailable |
| Q5 | Firefox or Safari | Bridge | Q4 topology | — | Stop bridge or unplug + reconnect | **PENDING_OPERATOR** | `BridgeClient.reconnect()` |
| Q6 | Chromium | Web Serial | Active session | — | Upload or second connect | **PENDING_OPERATOR** | `claimPortExclusive` / port-busy strings |
| Q7 | Doc walkthrough | Either | Cold boot → connect → disconnect | — | — | **PENDING_OPERATOR** | Follow `HARDWARE_OPERATOR.md` mode section |

**7.8:** This file satisfies structured attestation requirement; operator fills Pass/Fail after hardware run.

## Automated substitute checks (no hardware)

- JS syntax: `node --check` on all new host transport modules — pass
- Bridge smoke: `integration-smoke.mjs` requires live bridge+device — not run (no device)
- Pot telemetry parity: both transports parse `primaryTelemetry`/`secondaryTelemetry` in `hardware-slot-adapter.js` — code review pass

## Finish recommendation

Merge implementation and docs; **operator must complete Q1–Q7 on bench hardware** before production reliance. No apply abort — manual QA deferral is explicit in change `tasks.md` deferred section (automated CI matrix deferred).
