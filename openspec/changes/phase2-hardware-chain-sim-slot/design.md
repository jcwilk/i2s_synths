## Context

Phase 0 (`phase0-hardware-chain-offline-ab`) validates module parity offline. Phase 1 (`phase1-hardware-chain-usb-protocol`) proves sustained full-rate USB duplex with host clock mastery and passthrough identity. The module chain simulator (`module-chain-simulator`) runs a gateway plus up to eight WASM units with decoupled dual-path timing, structural reconfiguration on chain edits, and simulated pot EMA on WASM slots.

Phase 2 integrates one physical ESP32-S3-Zero as a substitute processing slot in that browser chain. Browsers cannot reliably open native USB CDC bulk from the PWA main thread or AudioWorklet, so a **local bridge server** relays WebSocket binary frames to USB. The hardware slot is **async**: the AudioWorklet must never block on USB round-trip; ring buffers absorb pipeline delay while WASM neighbors keep cadence.

MVP module kind: **delay** — stateful, control-responsive, and representative of integration risk. Physical pots on device replace sim sliders for the hardware slot.

## Goals / Non-Goals

**Goals:**

- Local Node bridge server: WebSocket ↔ USB CDC bulk relay using Phase 1 exchange geometry (512 int16 stereo per path, 44.1 kHz, no compression).
- PWA operator flow: designate one slot as hardware, select delay, connect/disconnect bridge, visible status.
- Chain scheduler: substitute bridge-mediated exchanges for one slot; bounded async buffers; preserve left-to-right / right-to-left wiring and path-decoupling semantics.
- Firmware: delay module in USB neighbor mode with **physical ADC** controls when serving PWA hardware slot (distinct from Phase 1 host-injected controls for reference tool).
- Integration acceptance: mic-fed mixed chain (WASM neighbors + hardware delay), pots audibly affect output, full fidelity maintained.
- Document USB + bridge latency as operator-visible delta.

**Non-Goals:**

- Web Serial direct path (Phase 4).
- Multiple hardware slots or multi-board routing.
- All module types on hardware (merger HW testing deferred).
- Phase 3 full-chain realtime hardening under worst-case mixed topology.
- WiFi transport, downsampling, mono, compression.

## Decisions

### 1. Bridge server architecture (Node process)

**Choice:** Standalone Node.js process exposing a WebSocket server on a documented local port (default `localhost:8765`). One WebSocket client (the PWA) per session. Bridge opens USB via platform library (`node-serialport`, `usb`, or equivalent) on the Phase 1 binary audio interface.

**Relay loop:** For each message from PWA → serialize if needed → write USB request frame → await USB response → forward to PWA. Bridge does not process audio; it is a transparent relay with session lifecycle (enter/exit USB neighbor on device).

**Rationale:** Default path works in browsers without Web Serial; keeps USB permissions and enumeration on the host OS outside the sandboxed tab.

**Alternatives considered:** Browser Web Serial only — deferred to Phase 4; requires flag day for operators without bridge.

### 2. WebSocket message contract (implementation layer)

**Choice:** Binary WebSocket frames mirroring Phase 1 duplex payload: sequence, four audio paths, status echo. Separate JSON control messages for session start/stop, device enumeration, and error reporting. Exact field layout documented in bridge README during apply; spec remains behavior-level.

**Rationale:** Binary audio on WebSocket avoids base64 overhead; JSON only for low-rate control.

### 3. Async hardware slot in chain scheduler

**Choice:** Dedicated **hardware adapter** in the audio graph:

```
[WASM neighbors] ←→ [ring buffers] ←→ [async exchange worker] ←→ WebSocket ←→ bridge ←→ USB
```

- AudioWorklet pushes downstream input / upstream input for the hardware period into outbound rings and reads latest available outputs from inbound rings (may be one or more periods delayed).
- Main thread or `AudioWorkletProcessor` companion port schedules exchanges at ~5.8 ms cadence aligned with Phase 1 host clock mastery; worklet never awaits USB.
- Ring depth: minimum 2 periods each direction (configurable spike to 3 if underruns observed); underrun → output silence for that path and visible indicator on hardware card (consistent with merger underrun UX).

**Rationale:** USB RTT (Phase 1 p50 ≤ 12 ms, p99 ≤ 20 ms) exceeds one buffer period; blocking worklet would glitch entire chain.

**Alternatives considered:** Synchronous exchange in worklet — rejected; violates real-time guarantee for WASM neighbors.

### 4. Pipeline delay accounting in mixed wiring

**Choice:** Hardware slot adds **documented extra path delay** equal to ring depth × buffer period in addition to standard inter-unit delay already modeled. Scheduler wires neighbors to hardware slot through delay lines matching ring occupancy target, not instantaneous same-period feeds.

**Rationale:** Aligns with existing decoupled dual-path timing requirement; operators hear predictable extra latency vs all-WASM.

### 5. Hardware designation and WASM bypass

**Choice:** Unit card toggle `Hardware` sets slot role. When connected:
- No WASM instance allocated for that slot.
- Sim pot sliders hidden/disabled; label shows "Device pots".
- Module dropdown remains for **intent** but connect validates against firmware-reported kind (binary query frame on session start).

**Rationale:** Single mental model: one slot, one device; prevents silent wrong-module relay.

### 6. Firmware: PWA hardware slot control source

**Choice:** USB neighbor mode gains a **control source** distinction:
- **Reference tool mode** (Phase 1): injected controls from exchange payload.
- **PWA hardware slot mode** (Phase 2): read physical ADC with normal EMA; ignore control fields in payload (or treat as no-ops).

Enter command from bridge on session start selects PWA hardware slot mode. Delay module build for Phase 2 acceptance.

**Rationale:** Operator expectation is real pots; Phase 1 harness remains unchanged.

### 7. Structural reconfiguration scope

**Choice:** Hardware connect/disconnect and hardware designation toggle trigger same audio rebuild as module type change (stop, clear meters/buffers, rebuild graph, auto-resume). Reorder/add/remove while hardware connected: allowed but forces disconnect first (UI guard) to avoid stale device state mid-drag.

**Rationale:** Simplifies graph rebind; avoids half-connected topology during drag.

### 8. Bridge discovery and operator UX

**Choice:** PWA settings panel: bridge URL (default `ws://localhost:8765`), Connect Hardware button on designated card, status chips: Bridge offline / Device attached / Session active / Module mismatch.

**Rationale:** Minimal UI surface for MVP; URL field supports non-default port.

### 9. Integration test (acceptance)

**Choice:** Manual attestation checklist:
1. Start bridge; flash delay firmware; connect session on slot 2 of chain `[gateway, WASM passthrough, HW delay, WASM passthrough]`.
2. Enable mic; run ≥ 30 s; verify delay audible; twist length/speed pots; verify change.
3. Confirm no sustained drop indicators; document subjective latency note.

Automated headless test optional; not required for apply-complete if hardware attestation recorded.

## Risks / Trade-offs

- **[Risk] Ring underrun causes silence glitches in hardware slot** → Depth 2–3 periods; visible underrun badge; Phase 3 hardening.
- **[Risk] Bridge process not running confuses operators** → Clear offline state; link to start command in README.
- **[Risk] Module kind mismatch silent failure** → Block connect; firmware kind query on session start.
- **[Risk] Added latency makes merger audition misleading** → Document delta; defer merger HW to later phase.
- **[Risk] Windows USB scheduling worse than Linux** → Record platform in attestation; tune ring depth per OS if needed.
- **[Risk] Control source mode confusion (injected vs ADC)** → Distinct enter subcommand; bridge only uses PWA mode.

## Migration Plan

1. Approve Phase 2 artifacts.
2. Apply: bridge server package, PWA hardware adapter + UI, firmware delay + ADC control source.
3. Verify compile (delay USB neighbor build).
4. Run integration acceptance with board + mic.
5. Rollback: disable hardware toggle in UI; stop shipping bridge script; firmware USB neighbor unchanged for Phase 1 tool.

## Open Questions

- Exact ring depth default (2 vs 3 periods) — measure on target OS during apply.
- Whether firmware reports module kind via existing status frame or new query — prefer reuse of Phase 1 control frame types.
- Reorder-while-connected: disconnect-first guard vs hot rebind — prefer disconnect-first for MVP.
- Bridge packaging: npm script in repo root vs `sim/` subpackage — resolve in apply for DX consistency.
