## Context

Phase 0 (`phase0-hardware-chain-offline-ab`) validates module parity offline. Phase 1 (`phase1-hardware-chain-usb-protocol`) proves sustained full-rate USB duplex with host clock mastery and passthrough identity at documented latency thresholds (p50 ≤ 12 ms, p99 ≤ 20 ms, zero drops over 60 s). Phase 2 (`phase2-hardware-chain-sim-slot`) integrates one hardware slot into the module chain simulator via a local bridge server, async ring buffers, and delay-module MVP acceptance (~30 s mixed chain).

Phase 3 closes the gap between "integration works" and "operators can run realistic auditions for 10+ minutes without sustained drops or chain-halting glitches." Stress targets include maximum chain length (gateway plus eight WASM/HW units), merger with loopback and hardware in middle or end positions, microphone on, and any simulator-supported module kind flashed on the board—all at full 44.1 kHz stereo int16 PCM with no downsampling, compression, or mono fallback.

## Goals / Non-Goals

**Goals:**

- Zero sustained frame drops over extended mixed-chain runs (minimum 10 minutes documented soak).
- Documented and met added latency budget for hardware substitution in mixed chains (ring depth, USB RTT, bridge relay).
- Backpressure and recovery that substitute silence or hold last-good buffers without halting WASM neighbors or requiring session restart for transient events.
- Optional pot telemetry from device to PWA for hardware-slot display; physical pots remain authoritative for processing.
- Hardware-slot In/Out peak metering presentation consistent with WASM units (log scale, dual windows, buffer-aligned peaks).
- Realtime acceptance for all simulator-supported module kinds: passthrough, delay, merger, cutoff, debug tone.
- Stress acceptance matrix covering long chains, merger+loopback topologies with hardware placement variants, and microphone input.

**Non-Goals:**

- Web Serial direct path, reconnect polish, I2S-vs-USB runtime switch (Phase 4).
- WiFi transport, multiple simultaneous hardware boards.
- Compression, downsampling, mono, sample-rate conversion.
- Replacing Phase 1 reference tool or Phase 0 offline harness.

## Decisions

### 1. Ring buffer depth and tuning

**Choice:** Default ring depth **3 periods** each direction (up from Phase 2's 2–3 spike) for hardware slot inbound/outbound paths. Target occupancy: maintain 1–2 periods of headroom under nominal USB RTT. Adaptive depth increase to **4 periods** only when sustained underrun rate exceeds documented threshold during soak (logged, not silent).

**Rationale:** Phase 1 p99 ≤ 20 ms ≈ 3.4 buffer periods; 2-period depth leaves insufficient margin for bridge + scheduler jitter.

**Alternatives considered:** Fixed depth 2 — rejected after Phase 2 risk note; depth 5+ — adds latency beyond documented budget.

### 2. Underrun/overrun recovery audibility policy

**Choice:**

| Event | Audio output | Chain behavior | Operator visibility |
|-------|--------------|----------------|---------------------|
| Transient inbound underrun (hardware slot output not ready) | Silence for affected path samples in that period | WASM neighbors continue; no structural rebuild | Brief underrun indicator on hardware card; auto-clear after 3 consecutive good periods |
| Transient outbound overrun (exchange worker backlog) | Drop oldest pending exchange (not in-flight) | Continue scheduling; never block worklet | Overrun badge; count visible in soak summary |
| Sustained underrun (>5% of periods over 30 s window) | Same silence policy | Session remains active; optional operator reconnect prompt | Persistent warning; soak fails acceptance |
| Sequence gap or device-reported fatal status | Silence on hardware paths | Tear down hardware session gracefully; structural rebuild | Clear error; device exits USB neighbor mode |

**Rationale:** Audible glitches from stale buffer replay are worse than brief silence; chain halt is unacceptable for transient USB jitter.

**Alternatives considered:** Repeat last buffer — risks audible stutter loops; full chain stop on any underrun — rejected.

### 3. USB pipelining (full PCM preserved)

**Choice:** Allow **depth-2 in-flight exchanges** on bridge↔USB path when measured single-flight RTT p50 > 1.5× buffer period on target host. Host-side exchange worker may submit exchange N+1 before N completes; device processes strictly in sequence order; responses matched by sequence identifier. **No** payload compression or geometry change.

**Rationale:** Windows USB scheduling may need pipelining to sustain 5.8 ms cadence under mixed-chain CPU load without drops.

**Alternatives considered:** Depth-1 only — Phase 1 MVP; may fail Phase 3 soak on some hosts.

### 4. Documented mixed-chain latency budget

**Choice:** Publish operator-facing budget as sum of:

| Component | Budget (periods @ 5.8 ms) | Notes |
|-----------|---------------------------|-------|
| Hardware slot ring target occupancy | 2 | Nominal pipeline delay |
| USB + bridge round-trip (p99 planning) | 3.5 | Aligns with Phase 1 p99 ≤ 20 ms |
| Standard inter-unit path delay per neighbor | 1 each | Existing simulator wiring |
| **Example: HW slot between 2 WASM neighbors** | **~2 + 3.5 = 5.5 periods (~32 ms)** | Plus per-neighbor delays in longer chains |

Acceptance: measured end-to-end added delay (impulse or correlated tone method in soak harness) within **±1 buffer period** of documented budget for reference topology `[gateway, WASM passthrough, HW delay, WASM passthrough]`.

### 5. Merger + hardware timing interaction

**Choice:** When hardware slot neighbors a merger unit (either side), scheduler SHALL NOT collapse merger ring-buffer decoupling with hardware async delay. Merger underrun/overrun indicators remain merger-local; hardware underrun indicators remain hardware-local. Loopback on rightmost unit with hardware in chain: upstream return path includes full hardware async delay before merger sees loopback feed.

**Stress topologies for acceptance:**

1. `[gateway, HW merger, WASM…]` — hardware merger, loopback off, mic on.
2. `[gateway, WASM, HW delay, WASM merger(loopback)]` — hardware middle, loopback on end.
3. `[gateway, WASM×(max−2), HW passthrough]` — max chain, hardware end, mic on.

**Rationale:** Merger ring fill timing is the highest-risk interaction with async hardware delay (Phase 2 deferred merger HW).

### 6. Pot telemetry relay

**Choice:** Device includes smoothed primary and secondary control readings (normalized 0..1) in exchange status or ancillary response fields on every exchange (or every Nth exchange with hold-last semantics). PWA hardware card MAY display telemetry values as read-only indicators; sliders remain disabled; **processing on device always uses physical ADC**, not echoed telemetry round-trip.

**Rationale:** Operators want visual confirmation of pot position without sim sliders; telemetry is display-only to avoid control-loop latency issues.

### 7. Multi-module-kind acceptance matrix

**Choice:** Per-kind minimum soak (connected hardware, matching firmware):

| Module kind | Minimum soak | Pass criteria |
|-------------|--------------|---------------|
| Passthrough | 10 min | Zero sustained drops; identity audible (transparent) |
| Delay | 10 min | Effect audible; pots affect output; zero sustained drops |
| Merger | 10 min | Merge dynamics audible; underrun/overrun recover without halt |
| Cutoff | 10 min | Filter response to pots; zero sustained drops |
| Debug tone | 10 min | Tone present when configured; zero sustained drops |

Each kind: at least one topology from stress matrix with mic enabled for ≥2 min within soak.

### 8. CPU budget

**Choice:**

| Layer | Budget | Measurement |
|-------|--------|-------------|
| ESP32-S3 module loop + USB ISR | < 70% of buffer period worst-case | `esp_timer` delta per exchange in diagnostic build |
| Bridge server relay | < 2 ms p99 processing per exchange excluding USB wait | Node `performance.now()` around relay |
| PWA AudioWorklet (mixed max chain) | < 50% of render quantum p99 | Chrome performance panel during soak |

Soak fails if any layer exceeds budget sustained over 60 s window.

### 9. Acceptance test matrix (summary)

| ID | Duration | Topology | Mic | Module |
|----|----------|----------|-----|--------|
| S1 | 10 min | GW + 7 WASM + HW end | On | passthrough |
| S2 | 10 min | GW + WASM + HW delay + WASM | On | delay |
| S3 | 10 min | GW + HW merger + 2 WASM | On | merger |
| S4 | 10 min | GW + WASM + HW cutoff + WASM merger (loopback) | On | cutoff |
| S5 | 10 min | GW + WASM + HW debug_tone + WASM | Off | debug_tone |
| S6 | 10 min | GW + max WASM/HW interleave, HW middle | On | delay |

Automated headless soak optional; manual attestation with structured checklist sufficient for apply-complete.

## Risks / Trade-offs

- **[Risk] 10 min soak too heavy for CI** → Manual board attestation; compile-only CI; document matrix row completed per module kind.
- **[Risk] Pipelining depth-2 exposes ordering bugs** → Strict sequence matching; fail closed on gap; Phase 1 sequence rules reused.
- **[Risk] Merger+HW+loopback unstable** → Dedicated S3/S4 scenarios; merger underrun policy unchanged from simulator spec.
- **[Risk] Pot telemetry bandwidth** → Piggyback on existing status fields; no extra round trip.
- **[Risk] Latency budget mismatch on Windows** → Record platform in attestation; OS-specific ring depth table in README.
- **[Risk] Debug tone soak annoying** → S5 allows mic off; headphones attenuation noted in checklist.

## Migration Plan

1. Approve Phase 3 artifacts.
2. Apply: ring depth tuning, recovery policy, pipelining spike, telemetry fields, metering parity, firmware builds per module kind.
3. Run acceptance matrix rows with connected board; record attestation per row.
4. Update simulator and bridge documentation with latency budget and recovery semantics.
5. Rollback: revert to Phase 2 ring depth and recovery behavior; telemetry display optional and can be hidden.

## Open Questions

- Whether depth-2 USB pipelining is enabled by default or opt-in per host OS — measure during apply on Linux and Windows.
- Exact sustained-drop definition: zero drops entire 10 min vs zero drops after 60 s warm-up — prefer entire run, allow 30 s warm-up excluded from drop count if documented.
- Merger hardware acceptance with loopback on hardware unit itself — defer unless S4 passes; not required for apply-complete.
- Whether pot telemetry updates every exchange or every 4th exchange — prefer every exchange if status field space allows.
