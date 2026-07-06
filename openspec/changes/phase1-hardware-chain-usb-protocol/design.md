## Context

Phase 0 (`phase0-hardware-chain-offline-ab`) established offline buffer geometry at 44.1 kHz stereo; Phase 1 spike measurements show that geometry cannot sustain realtime USB duplex on FS CDC (~14 ms RTT vs ~5.8 ms period). A **universal shift to 22.05 kHz mono** (128 int16 samples per path, same ~5.8 ms period) cuts wire bytes ~4× and measured RTT ~4 ms, yielding ~1.5× realtime headroom on the reference host.

Firmware processes upstream then downstream per buffer period in normal I2S mode. Phase 1 applies the mono contract under realtime USB duplex with the host as clock master. Offline neighbor mode uses the same geometry so simulator, WASM, firmware I2S, and bridge transports stay aligned.

## Goals / Non-Goals

**Goals:**

- **Universal mono 22.05 kHz contract:** 22.05 kHz, mono int16 per path, 128 samples per path per exchange (~5.8 ms period) for offline neighbor, USB realtime, and all downstream phases.
- Host-driven timing: host schedules exchanges at firmware buffer period cadence; device processes each complete frame as it arrives.
- USB neighbor firmware mode: I2S to physical chain neighbors disabled; module loop fed from USB frames.
- Host reference tool measuring sustained rate, round-trip latency, drop count, and realtime ratio over a fixed acceptance run.
- Passthrough acceptance: outputs match inputs on both paths under sustained realtime load.
- Offline harness vectors regenerated; passthrough bit-exact and delay tolerance re-validated at new geometry.
- Logging coexistence without corrupting the audio binary pipe during acceptance.

**Non-goals:**

- PWA chain slot, bridge server, Web Serial, WiFi (Phases 2–4).
- Delay realtime modules in USB acceptance (offline delay coverage remains; realtime deferred).
- Compression or dual-contract support (44.1 kHz stereo retired for bridge/I2S chain).
- Pack4 or other wire packing experiments (spike showed RTT scales with bytes; smaller payloads win).

## Decisions

### 1. Universal buffer geometry

**Choice:** One geometry everywhere the firmware chain and bridge exchange audio:

| Parameter | Value | Notes |
|-----------|-------|-------|
| Sample rate | 22.05 kHz | Standard half-rate of 44.1 kHz |
| Encoding | Mono int16 per path | One sample per path per time index; not stereo interleaved |
| Samples per path per exchange | 128 | Matches ~5.8 ms at 22.05 kHz |
| Paths per exchange | 4 | Downstream in/out, upstream in/out (unchanged semantics) |
| Period | ~5.8 ms | 128 ÷ 22050 |

Firmware `SAMPLE_RATE` and `BUFFER_LEN` constants align to this contract. I2S DMA and module loops use the same buffer length.

**Rationale:** Spike proved ~3.9× wire reduction vs stereo 44.1 kHz with identical period; simplifies simulator, WASM, offline harness, and USB to one contract.

### 2. Realtime frame layout

**Choice:** One logical duplex exchange per buffer period. Request/response carry mono path payloads plus controls, sequence, status, optional timestamp—same field semantics as Phase 0 four-path contract but mono sample encoding.

On-wire: length-prefixed binary blob per direction on USB CDC bulk. Inner request ~530 B vs ~2066 B stereo (spike measurement).

### 3. Sequencing and host clock mastery

**Choice:** Host increments sequence per submitted exchange. Device echoes sequence in response. Single in-flight exchange (depth 1) for MVP.

Target cadence: **5.8 ms** nominal (128 ÷ 22050).

### 4. Backpressure and drops

**Choice:** Same as prior Phase 1 design—overrun/underrun status, sequence gap fails session, missing response within **3× nominal period** counts as drop. Host uses a persistent framed reader so partial USB chunks do not desync sessions (spike finding).

### 5. USB neighbor firmware mode

Unchanged lifecycle from prior design: explicit enter/exit; injected controls during USB mode; passthrough compile target for acceptance.

### 6. Host reference tool

Standalone host process: enter USB neighbor, sustained **60 s** acceptance at 5.8 ms cadence, passthrough identity check, metrics summary including **realtime ratio** (audio seconds ÷ wall seconds).

### 7. Logging coexistence

Dual CDC preferred; single-CDC fallback with binary-only audio during acceptance (spike validated on single CDC).

### 8. Measured acceptance thresholds (mono 22.05 kHz, spike-informed)

| Metric | Threshold | Notes |
|--------|-----------|-------|
| Sustained duration | ≥ 60 s continuous | No intentional pauses |
| Exchange count | ≥ 10,000 completed | ~60 s ÷ 5.8 ms ≈ 10,345 |
| Drop count | 0 | Overrun, underrun, missing response, sequence gap |
| Realtime ratio | ≥ 1.0 | Audio-time processed ÷ wall-clock duration |
| Round-trip latency (p50) | ≤ 6 ms | Must stay below buffer period |
| Round-trip latency (p99) | ≤ 10 ms | USB jitter allowance |
| Passthrough identity | Bit-exact per exchange | ds_out == ds_in, us_out == us_in |
| Sequence gaps | 0 | Any gap fails run |

Spike reference (30 s, Linux, ESP32-S3-Zero): ~7489 periods, p50 ~3.9 ms, realtime ratio ~1.45×, zero drops.

### 9. Relationship to Phase 0

**Choice:** Phase 1 **replaces** Phase 0 geometry in living spec on archive (MODIFIED requirement). Offline harness vectors and golden files regenerated; delay tolerance rules unchanged in principle, re-run required.

## Risks / Trade-offs

- **[Risk] Lower audio bandwidth (22.05 kHz mono)** → Acceptable for dev/harness path; document as intentional trade for USB realtime; physical chain uses same geometry for parity.
- **[Risk] Vector regeneration misses edge cases** → Re-run full Phase 0 offline acceptance matrix before Phase 1 HW attestation.
- **[Risk] Simulator/WASM not yet aligned** → Phase 1 tasks include constant alignment; Phase 2 assumes mono contract throughout chain.

## Migration Plan

1. Approve updated Phase 1 artifacts.
2. Apply: align firmware constants; merge spike transport into primary USB exchange command; regenerate offline vectors; update host tools.
3. Re-run offline passthrough + delay acceptance.
4. Flash passthrough; 60 s USB realtime acceptance.
5. Archive Phase 1; living spec reflects mono geometry.

## Open Questions

- Whether I2S physical chain modules already deployed need a one-time reflash note in operator docs (Phase 4).
- Exact dual-CDC enumeration on each host OS—unchanged from prior Phase 1.
