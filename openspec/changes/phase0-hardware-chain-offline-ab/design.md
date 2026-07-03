## Context

The project has a browser module chain simulator that runs firmware module logic compiled to WebAssembly at 44.1 kHz stereo with a fixed buffer period matching on-device streaming. Firmware on ESP32-S3-Zero processes dual paths (`moduleLoopUpstream` then `moduleLoopDownstream`) with physical pot ADC in normal I2S mode. The hardware-in-loop series plans to replace one simulator slot with a real device streaming over USB at full fidelity; Phase 0 proves module parity offline before any realtime duplex work.

Planning notes in `ai/planning/web-module-chain-simulator.md` list "Hardware A/B" as a future option: inject the same buffer to sim and device and compare captures. This change formalizes that as the first phase of the hardware bridge.

## Goals / Non-Goals

**Goals:**

- Define a deterministic offline exchange: host supplies one buffer period of downstream and upstream inputs plus control state; device returns downstream and upstream outputs for that period.
- Match firmware buffer geometry exactly: 44.1 kHz contract, stereo interleaved 16-bit PCM, firmware buffer period length (512 int16 samples per path).
- Compare device outputs to a reference generated from the same module sources used by the module chain simulator.
- Support multi-period runs so stateful modules (delay, merger rings) accumulate state across exchanges like continuous processing.
- MVP module kinds: passthrough and delay; harness and firmware mode structured so merger, cutoff, and debug-tone can be added without redesigning the frame contract.
- Preserve normal I2S streaming behavior when offline neighbor mode is not active.

**Non-Goals:**

- Sustained realtime USB audio duplex or bridge-server integration (Phase 1+).
- PWA chain UI slot for hardware (Phase 2).
- Web Serial reconnect or runtime I2S-vs-USB switching (Phase 4).
- Replacing simulator realtime audition; offline harness is a separate validation path.

## Decisions

### 1. Offline neighbor mode in firmware (vs. separate test sketch)

**Choice:** Add a firmware operating mode entered and exited explicitly that runs one dual-path buffer period per host exchange using injected inputs and control values, without starting the I2S pipeline.

**Rationale:** Reuses the same compiled module variant and `moduleLoop*` invocation order as production firmware, minimizing "test-only" drift.

**Alternatives considered:** Standalone test sketch with duplicated module calls — rejected because it could diverge from shipping init and control plumbing.

### 2. Host offline harness (inject → process → capture → diff)

**Choice:** A host-side tool drives the exchange loop: for each period in a test vector sequence, send inputs and controls, receive outputs, append to a capture buffer, and diff against reference after the run (or per period).

**Rationale:** Decouples validation from browser UI and realtime clocks; easy to run in CI when a board is attached and headless when using file replay.

**Alternatives considered:** Manual serial capture only — rejected as non-repeatable for agents and CI.

### 3. Binary buffer frame layout

**Choice:** One logical frame per buffer period:

| Field | Size | Notes |
|-------|------|-------|
| Downstream input | 512 × int16 | Stereo interleaved |
| Upstream input | 512 × int16 | Stereo interleaved |
| Primary control | float32 0..1 | Smoothed-equivalent value for offline run |
| Secondary control | float32 0..1 | Smoothed-equivalent value for offline run |
| Sequence index | uint32 | Monotonic period counter for logging |
| Downstream output | 512 × int16 | Returned by device |
| Upstream output | 512 × int16 | Returned by device |
| Status flags | uint16 | Success, module-specific underrun/overrun bits as needed later |

Host→device request carries inputs + controls + sequence; device→host response carries outputs + status. Exact on-wire encoding (length prefix, checksum, COBS, etc.) is an implementation detail for tasks; Phase 0 MAY use USB CDC binary or file-based replay of pre-serialized frames.

**Rationale:** Four-path contract mirrors chain neighbor wiring (ds_in, us_in → module → ds_out, us_out) at full fidelity without downsampling.

### 4. Reference source: simulator-equivalent headless run

**Choice:** Generate reference outputs by running the same module variant and buffer sequence through the existing simulator WASM build in a headless host process, **or** load precomputed golden vector files produced by that same pipeline.

**Rationale:** Aligns reference with `module-chain-simulator` compiled-module requirement; golden files allow diff-only reruns without rebuilding WASM each time.

**Alternatives considered:** Reimplementing reference in Python — rejected; violates parity intent.

### 5. Tolerance policy

**Choice:**

| Module kind | Policy |
|-------------|--------|
| Passthrough | Bit-exact: zero sample differences on both paths |
| Delay | Maximum absolute per-sample deviation ≤ 2 LSB on int16 samples after identical control state; report max diff and RMS |
| Merger / cutoff / debug-tone (later) | Defined when added; default to documented LSB threshold pending FP audit |

Comparison SHALL fail if any sample exceeds the module-kind threshold or if path geometry/order mismatches.

**Rationale:** Integer copy modules should match exactly; delay uses fractional read pointers and averaging that may differ by rounding between WASM and native builds.

### 6. Control state: held vs. stepped

**Choice:** Each frame carries explicit primary/secondary values; harness MAY hold constants across all periods or supply a per-period schedule (e.g., stepped pot sweeps). Offline mode SHALL NOT read physical ADC during exchange; optional "use live pots" is out of scope for Phase 0.

**Rationale:** Determinism requires injected controls; matches simulator's ability to set smoothed pot state directly in verify harnesses.

### 7. Transport for Phase 0

**Choice:** Prefer USB CDC binary framing on ESP32-S3 native USB for integrated inject/capture; allow file-based injection (device reads vectors from flash or host sends file batches) when USB tooling is inconvenient.

**Rationale:** USB is the eventual Phase 1 transport; offline framing can evolve into realtime protocol header. File replay de-risks bring-up before binary USB is stable.

**Note:** 115200 baud logging remains human-readable diagnostics only, not the audio carrier.

### 8. Test vector format

**Choice:** JSON or binary manifest listing ordered periods: optional metadata (module kind, control values), input blobs, expected output blobs (when precomputed). Harness can generate manifest from WASM reference run.

## Risks / Trade-offs

- **[Risk] WASM vs native rounding divergence on delay** → Document LSB tolerance; tighten after first hardware run; consider fixed-point reference if threshold too loose.
- **[Risk] USB binary framing bugs obscure module failures** → File-replay path and loopback self-test (device echoes inputs) before module diff.
- **[Risk] Offline mode accidentally ships in production path** → Gated by compile flag or explicit enter command; normal boot remains I2S; tasks include verify normal mode unchanged.
- **[Risk] Stateful modules need warm-up periods** → Test vectors include leading periods excluded from diff window (startup silence/fill) consistent with module specs.
- **[Risk] PSRAM allocation timing differs offline vs I2S** → Run allocation at offline mode entry using same init path as setup().

## Migration Plan

1. Approve change artifacts.
2. Apply on working branch: firmware offline mode, host harness, golden vectors for passthrough + delay.
3. Verify compile for passthrough and delay module selections.
4. When board available: run offline A/B for passthrough (expect bit-exact) then delay (expect within tolerance).
5. Rollback: disable offline mode entry; remove harness scripts; no change to living simulator or firmware I2S behavior.

## Open Questions

- Exact USB request/response framing and checksum strategy — resolve during apply (spike in tasks).
- Whether offline mode is compile-time-only (`OFFLINE_NEIGHBOR_TEST`) or runtime serial command — prefer runtime enter/exit for agent convenience if flash budget allows.
- Golden vector storage location (repo test fixtures vs generated-only) — prefer checked-in small vectors for passthrough; larger delay sequences generated in CI.
- Merger underrun/overrun status flags in response — defer to merger module addition; reserve status bits in frame layout.
