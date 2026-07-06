## Context

Phases 0–3 deliver offline parity, mono 22.05 kHz USB duplex, PWA hardware-slot integration via a local bridge server, and sustained mixed-chain realtime hardening at the universal mono geometry with no compression. Daily development still requires starting and maintaining a Node bridge process, recovering manually from cable unplug or port contention, and knowing implicitly when to flash for normal I2S chain operation versus USB neighbor dev mode.

Phase 4 adds developer and operator polish: an optional Web Serial direct path for Chromium browsers, reconnect without full browser restart, explicit firmware mode selection, and consolidated operator documentation—while preserving Phase 3 fidelity, recovery, and latency-budget semantics.

## Goals / Non-Goals

**Goals:**

- Web Serial direct transport in the PWA for Chrome and Edge, sharing the same full-rate four-path exchange contract as the bridge relay path.
- Bridge server remains the default and required path for browsers without Web Serial (Safari, Firefox).
- Graceful disconnect detection, operator-visible reconnect flow, and chain recovery without full browser restart when transport or device returns.
- Documented firmware mode selection: normal I2S chain mode versus USB neighbor dev mode, with clear boot and operator selection behavior.
- Operator documentation: setup guide, troubleshooting (BOOT button, port conflicts, bridge versus Web Serial), known deltas versus pure simulator chains.
- Transport selection UI and reconnect UX in the module chain simulator.
- Manual QA matrix covering Chrome Web Serial and at least one bridge-only browser.

**Non-Goals:**

- WiFi transport or any non-USB relay.
- Multiple simultaneous hardware boards or hardware slots.
- Compression or any change to the universal mono 22.05 kHz bridge geometry.
- Preset chain configurations with hardware slot markers (deferred unless apply reveals a natural fit).
- Replacing Phase 1 host reference tool or Phase 0 offline harness.

## Decisions

### 1. Web Serial versus bridge feature parity matrix

**Choice:** Both transports SHALL relay the same Phase 1 four-path exchange geometry (22.05 kHz mono int16, fixed per-path sample count, monotonic sequence identifiers) with PCM fidelity preserved at the universal contract. Feature parity is transport-level only; browser capability determines availability.

| Capability | Bridge (default) | Web Serial direct |
|------------|------------------|-------------------|
| Full-rate four-path duplex | Yes | Yes |
| All simulator-supported module kinds | Yes | Yes |
| Physical ADC control source in PWA hardware slot | Yes | Yes |
| Pot telemetry display relay | Yes | Yes |
| Phase 3 recovery semantics (underrun/overrun, sustained-drop policy) | Yes | Yes |
| Safari / Firefox | Yes | No (Web Serial unavailable) |
| Requires separate host process | Yes | No |
| USB port opened by | Bridge server (OS) | Browser tab (user grant) |
| Device enumeration | Bridge reports attached device | Browser port picker |
| Session enter/exit USB neighbor | Coordinated via bridge control channel | PWA sends documented entry/exit actions directly |
| Recommended for daily Chrome dev | Optional | Preferred when no upload conflict |
| Recommended for cross-browser QA | Required | N/A |

**Rationale:** One exchange adapter interface in the PWA with two backends avoids divergent audio behavior; browser matrix drives operator defaults, not capability gaps.

**Alternatives considered:** Web Serial only — rejects Safari/Firefox operators; bridge only — leaves Phase 4 goal unmet.

### 2. PWA transport adapter architecture

**Choice:** Shared **hardware transport adapter** in the PWA:

```
[async exchange worker] ←→ [transport interface] ←→ { bridge WebSocket | Web Serial port }
```

- Same ring-buffer depth, cadence, sequencing, and Phase 3 recovery policy regardless of backend.
- Operator selects transport before connect; selection persists for the browser session (not across reloads unless documented).
- Switching transport while connected requires explicit disconnect first.

**Rationale:** Prevents duplicate scheduling logic and drift between paths.

### 3. Reconnect state machine

**Choice:** Hardware session states and transitions:

```
                    ┌─────────────┐
         connect    │  CONNECTING │
        ──────────► │  (entering  │
                    │   USB nbr)  │
                    └──────┬──────┘
                           │ success
                           ▼
                    ┌─────────────┐     transport loss / device reset
         audio     │   ACTIVE    │──────────────────────────────┐
         running   │  (exchanging)│                              │
                    └──────┬──────┘                              │
                           │ operator disconnect                 │
                           ▼                                     ▼
                    ┌─────────────┐                    ┌─────────────┐
                    │ DISCONNECTED│                    │  DEGRADED   │
                    │  (clean)    │                    │ (link lost, │
                    └─────────────┘                    │ chain alive)│
                                                       └──────┬──────┘
                                                              │
                              operator reconnect / auto-retry │
                              (bounded attempts)              │
                                                              ▼
                                                       ┌─────────────┐
                                                       │ RECONNECTING│
                                                       │ (re-enter   │
                                                       │  USB nbr)   │
                                                       └──────┬──────┘
                                                              │
                                         success ────────────► ACTIVE
                                         failure ────────────► DEGRADED (with error)
                                         fatal (kind mismatch,
                                         sequence gap policy) ─► DISCONNECTED
```

**Behavior by state:**

| State | Chain audio | Hardware slot output | Operator UI |
|-------|-------------|----------------------|-------------|
| ACTIVE | Full mixed chain | Normal exchange | Connected indicator |
| DEGRADED | WASM neighbors continue; hardware paths silence per Phase 3 underrun policy | No exchanges | "Connection lost" banner; Reconnect action |
| RECONNECTING | Same as DEGRADED | Attempting enter + first exchanges | Spinner + attempt count |
| DISCONNECTED | Structural rules for disconnected slot | None | Connect enabled |

**Reconnect policy:**

- Operator-initiated reconnect from DEGRADED: re-open transport, re-enter USB neighbor, validate module kind, resume exchanges without full page reload.
- Optional bounded auto-retry (3 attempts, 2 s backoff) when link loss was abrupt (USB unplug); disable auto-retry on module-kind mismatch or fatal device status.
- Reconnect is **not** structural reconfiguration: chain layout, hardware designation, and WASM neighbors preserved; level histories may clear on successful reconnect (same as connect) per existing structural rules.
- Fatal errors (sequence gap, kind mismatch after reconnect) → clean DISCONNECTED with actionable message.

**Rationale:** Operators should recover from cable bumps without losing chain topology or restarting the tab; WASM cadence must not halt during degraded periods.

**Alternatives considered:** Full page reload on disconnect — poor daily-dev UX; auto-reconnect infinite loop — hides port conflicts.

### 4. Firmware mode switch mechanism

**Choice:** Three-layer mode selection with documented precedence:

| Layer | Mechanism | Purpose |
|-------|-----------|---------|
| **Boot default** | Normal I2S chain mode on cold boot | Standalone hardware chain operation unchanged |
| **Runtime override** | Documented serial command from host (bridge or Web Serial) to enter/exit USB neighbor | PWA dev sessions |
| **Development aid** | Optional compile-time default-to-USB-neighbor flag for lab boards only; never default in release builds | Faster iteration on dedicated dev boards |

**Operator selection flow:**

1. **Normal I2S chain mode:** Power on, no USB neighbor entry — device participates in physical I2S neighbor chain per pre-bridge behavior. USB port may be idle or used for logging only; PWA cannot process audio until operator switches to dev mode.
2. **USB neighbor dev mode:** Operator connects via bridge or Web Serial and issues session start (enter USB neighbor). I2S neighbor exchange disabled for module processing; host supplies buffer periods. On session end or disconnect, device exits USB neighbor and returns to normal I2S mode without reflash.

**BOOT button role (documented, not spec-mandated GPIO map):**

- Short press during boot: documented recovery / safe mode if implemented; does not silently force USB neighbor.
- Upload / flash workflow: hold BOOT for serial bootloader — documented in troubleshooting guide so operators do not confuse flash mode with runtime mode selection.

**Rationale:** Preserves standalone I2S chain use case; dev mode is explicit and reversible. Compile flag is opt-in for boards that never leave the bench.

**Alternatives considered:** USB neighbor default on boot — breaks standalone chain; GPIO-only mode switch — hardware-dependent and harder to document uniformly.

### 5. Port exclusivity (upload versus connected PWA)

**Choice:** Document and enforce at UX level:

| Situation | Expected behavior | Operator guidance |
|-----------|-------------------|-------------------|
| PWA Web Serial session open | Upload tool cannot open same port | Disconnect PWA first; close other serial monitors |
| Bridge session open | Web Serial and upload compete for OS port lock | Stop bridge session before flash |
| Flash in progress | PWA connect fails with clear "port busy" message | Complete upload, reset device, retry connect |
| Device reset during ACTIVE session | Transition to DEGRADED; offer reconnect | Wait for reboot, reconnect |

Bridge and Web Serial are **mutually exclusive** on the same physical port; PWA SHALL NOT attempt Web Serial while bridge holds the device.

**Rationale:** OS-level serial port exclusivity is the root cause of most "it worked yesterday" support issues; surfacing it beats silent failure.

### 6. Operator documentation structure

**Choice:** Single operator guide (README section or docs page) with:

1. **Quick start** — designate hardware slot, pick transport, connect, audition.
2. **Transport comparison** — parity matrix summary and browser recommendations.
3. **Mode selection** — when to use I2S chain vs USB neighbor dev mode.
4. **Troubleshooting** — BOOT/upload, port busy, bridge not running, Web Serial permission denied, reconnect after unplug, module-kind mismatch.
5. **Known deltas vs pure sim** — cross-link Phase 3 latency budget, async pipeline, single-slot limit, browser-specific transport notes.

Simulator known-limitations section updated to link to the guide.

### 7. Preset chains with hardware slot marker

**Choice:** **Defer** to a follow-up change. JSON save/load of chain topology is not required for Phase 4 apply-complete; if apply discovers existing preset infrastructure with minimal extension, document in open questions rather than blocking polish delivery.

**Rationale:** Phase 4 goal is transport and reconnect polish; preset schema design risks scope creep.

## Risks / Trade-offs

- **[Risk] Web Serial permission UX varies by OS** → Document Chrome flag and permission flow; bridge fallback always available.
- **[Risk] Reconnect mid-chain audible glitch** → DEGRADED uses Phase 3 silence policy; WASM neighbors uninterrupted.
- **[Risk] Port exclusivity confuses operators** → Explicit error strings and troubleshooting section; disable connect when port busy detected early.
- **[Risk] Transport parity drift over time** → Single adapter interface; acceptance matrix runs both paths for reference topology.
- **[Risk] Compile-time USB-default flag shipped accidentally** → Release build checklist; flag guarded and documented as dev-only.
- **[Risk] Auto-reconnect loops on bad firmware** → Cap attempts; fall back to manual reconnect with error detail.

## Migration Plan

1. Approve Phase 4 artifacts.
2. Apply: Web Serial transport backend, reconnect state machine, transport UI, firmware mode documentation, operator guide.
3. Run manual QA matrix (Chrome Web Serial + Firefox or Safari bridge).
4. Update bridge README with coexistence and port exclusivity notes.
5. Rollback: disable Web Serial option in UI (bridge-only); reconnect UI inert; documentation revert — no firmware breaking change if mode selection remains host-driven enter/exit.

## Open Questions

- Whether bounded auto-retry on disconnect is enabled by default or opt-in per operator preference — prefer default on with visible attempt counter.
- Whether successful reconnect clears hardware level histories (align with connect structural rule) or preserves them — prefer clear on reconnect for meter consistency.
- Whether Web Serial transport selection persists in browser local storage across reloads — prefer session-only initially.
- Whether preset chain JSON with hardware slot marker fits naturally during apply — if not, confirm deferral in finish debrief.
