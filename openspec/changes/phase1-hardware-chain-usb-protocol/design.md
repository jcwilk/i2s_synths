## Context

Phase 0 (`phase0-hardware-chain-offline-ab`) defines offline buffer geometry, four-path neighbor contract, dual-path invocation order, and passthrough bit-exact reference comparison. Phase 1 applies the same buffer semantics under realtime USB duplex with the host as clock master. The ESP32-S3-Zero exposes native USB CDC bulk; human-readable logging at 115200 baud must not share the audio binary carrier during acceptance runs.

Firmware already processes upstream then downstream per buffer period in normal I2S mode. Phase 1 replaces live I2S neighbor capture with host-supplied frames while keeping passthrough module logic unchanged.

## Goals / Non-Goals

**Goals:**

- Sustained full-duplex binary exchange at firmware buffer cadence: 44.1 kHz, stereo interleaved int16 PCM, 512 samples per path per exchange (downstream input, upstream input → downstream output, upstream output).
- Host-driven timing: host schedules exchanges at ~5.8 ms period (512 samples ÷ 44100 Hz); device processes each complete frame as it arrives.
- USB neighbor firmware mode: I2S to physical chain neighbors disabled; module loop fed from USB frames.
- Host reference tool measuring sustained rate, round-trip latency, and drop count over a fixed acceptance run.
- Passthrough acceptance: outputs match inputs on both paths under sustained realtime load (identity modulo documented transport framing).
- Logging coexistence without corrupting the audio binary pipe during acceptance.

**Non-Goals:**

- PWA chain slot, bridge server, Web Serial, WiFi (Phases 2–4).
- Delay, merger, cutoff, or debug-tone realtime modules (Phase 0 offline coverage remains; realtime deferred).
- Compression, downsampling, mono fallback, or sample-rate conversion.
- Replacing or removing Phase 0 offline harness.

## Decisions

### 1. Realtime frame layout (extends Phase 0 offline frame)

**Choice:** One logical duplex exchange per buffer period. Host→device request and device→host response share the same payload geometry as Phase 0 offline frames:

| Field | Size | Notes |
|-------|------|-------|
| Magic / version | 2–4 bytes | Protocol identification |
| Frame type | 1 byte | Data exchange vs control (enter/exit mode, metrics query) |
| Sequence number | uint32 | Monotonic per exchange; host authoritative |
| Downstream input | 512 × int16 | Stereo interleaved |
| Upstream input | 512 × int16 | Stereo interleaved |
| Primary control | float32 0..1 | Injected; default 0.5 for passthrough acceptance |
| Secondary control | float32 0..1 | Injected; default 0.5 |
| Downstream output | 512 × int16 | Device response |
| Upstream output | 512 × int16 | Device response |
| Status flags | uint16 | Success, overrun, underrun, sequence gap |
| Optional timestamp | uint32 | Device local microseconds since stream start (for latency) |

On-wire: length-prefixed binary blob per direction on USB CDC bulk endpoint dedicated to audio (separate from logging when possible). CRC16 optional on apply spike; minimum is length + payload bounds check.

**Rationale:** Reuses Phase 0 four-path contract; realtime adds sequencing, status, and timing fields only.

### 2. Sequencing and host clock mastery

**Choice:** Host increments sequence per submitted exchange. Device echoes sequence in response. Device does not self-clock production of output frames; it processes request N and returns response N before accepting N+1 (single in-flight exchange per direction pair, or small fixed-depth queue of depth 2 max for pipelining spike).

Target cadence: **5.8 ms** nominal (512 ÷ 44100). Host reference tool uses high-resolution sleep/timer to schedule submissions.

**Alternatives considered:** Device-driven cadence — rejected; Phase 2+ PWA chain needs host/master alignment.

### 3. Backpressure and drops

**Choice:**

- If device receive buffer full: device sets overrun status; host counts drop and MAY skip or retry with next sequence (documented policy: retry once then fail acceptance).
- If host does not receive response within **3× nominal period** (~17.4 ms): host counts late response; three consecutive lates fail acceptance.
- If sequence gap detected on device: set status flag; host fails acceptance immediately.

Depth-1 processing on device (one frame in flight) for MVP; pipelining deferred unless bandwidth headroom insufficient.

### 4. USB neighbor firmware mode

**Choice:** Explicit enter/exit lifecycle distinct from offline neighbor mode and normal I2S:

- **Enter:** Stop I2S DMA to/from physical neighbors; allocate same module buffers; await USB frames; run `moduleLoopUpstream` then `moduleLoopDownstream` per exchange with injected controls (not ADC).
- **Exit:** Tear down USB stream handler; restore normal I2S neighbor streaming.
- **Boot default:** Normal I2S unchanged when USB neighbor not entered.

Compile-time passthrough module for Phase 1 acceptance builds.

**Rationale:** Mirrors Phase 0 offline mode separation without conflating deterministic offline with realtime clock.

### 5. Host reference tool (not PWA)

**Choice:** Standalone host process (language TBD in apply) that:

1. Opens native USB CDC bulk interface for binary audio (platform-specific: `pyserial`/`libusb`/etc.).
2. Sends enter-USB-neighbor control frame.
3. Runs **acceptance loop** for **60 seconds** at 5.8 ms cadence: generate deterministic test pattern (e.g., ramp or LFSR on int16 samples), submit frame, await response, verify passthrough identity, record timestamps.
4. Emits summary: mean/percentile round-trip latency, exchanges completed, drop count, pass/fail.

Not integrated with module chain simulator UI.

### 6. Logging coexistence

**Choice (preferred):** Dual-interface — USB CDC #1 binary audio only; USB CDC #2 or UART at 115200 for `Serial.println` diagnostics. Acceptance runs bind exclusively to audio interface with logging quiesced or redirected to secondary interface.

**Fallback:** Compile flag `USB_AUDIO_EXCLUSIVE` disables all `Serial` output on the audio interface during USB neighbor mode; control/status via binary frame types only.

**Rationale:** ~5.6 Mbps payload needs clean bulk pipe; interleaved text corrupts framing.

### 7. Measured acceptance thresholds

| Metric | Threshold | Notes |
|--------|-----------|-------|
| Sustained duration | ≥ 60 s continuous | No intentional pauses |
| Exchange count | ≥ 10,000 completed | ~60 s ÷ 5.8 ms ≈ 10,345 |
| Drop count | 0 | Overrun, underrun, or missing response |
| Round-trip latency (p50) | ≤ 12 ms | Host send → response received |
| Round-trip latency (p99) | ≤ 20 ms | Allows USB jitter |
| Passthrough identity | Bit-exact per exchange | ds_out == ds_in, us_out == us_in |
| Sequence gaps | 0 | Any gap fails run |

Hardware attestation required for apply-complete on realtime tasks; CI may compile-only.

### 8. Relationship to Phase 0

**Choice:** Reuse Phase 0 frame field sizes and path order. Phase 0 offline mode remains for A/B parity; USB realtime mode is separate entry command. Transport encoding developed in Phase 0 MAY be shared; realtime adds timing discipline only.

## Risks / Trade-offs

- **[Risk] USB CDC bandwidth or OS scheduling misses 5.8 ms cadence** → Measure on target host OS; allow ±0.5 ms jitter in scheduler; fail closed on drops.
- **[Risk] Logging on shared CDC corrupts frames** → Dual-interface or exclusive binary mode; loopback identity test before passthrough module test.
- **[Risk] Confusion between offline and USB realtime modes** → Distinct enter/exit commands and status reporting; document in harness help.
- **[Risk] Latency dominated by host USB stack** → Record p50/p99; Phase 3 addresses full-chain hardening; Phase 1 establishes baseline.
- **[Risk] Single in-flight frame limits throughput headroom** → Acceptable for passthrough MVP; pipelining noted as future optimization.

## Migration Plan

1. Approve Phase 1 artifacts.
2. Apply: firmware USB neighbor mode, binary framing (extend Phase 0 spike), host reference tool.
3. Compile passthrough build; run loopback/identity on bench.
4. When board connected: 60 s acceptance run; capture metrics; attestation in tasks.
5. Rollback: disable USB neighbor entry; normal I2S unaffected.

## Open Questions

- Exact USB interface enumeration on Linux/macOS/Windows for dual-CDC vs single-CDC fallback — resolve in apply spike.
- Whether depth-2 pipelining is needed on Windows hosts — measure before adding complexity.
- Runtime vs compile-time USB neighbor entry — prefer runtime command matching Phase 0 offline pattern.
- Whether device timestamp field uses `esp_timer` or frame counter only — either acceptable if latency math documented in harness.
