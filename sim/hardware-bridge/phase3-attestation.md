# Phase 3 hardware soak attestation

| Field | Value |
|-------|-------|
| Platform | Linux 6.8.0-124-generic (x86_64) |
| Bridge URL | ws://localhost:8766 |
| Device | /dev/ttyACM0 (Waveshare ESP32-S3-Zero) |
| Buffer geometry | 22.05 kHz mono, 128 samples/path (~5.8 ms) |
| Attestation date | 2026-07-05 |

## Matrix (bridge relay soak harness)

Pass criteria: zero sequence gaps, zero sustained-underrun windows (>5% over 30 s), transient error rate ≤0.05%, wall-clock duration ≥98% of target, module kind match.

| ID | Chain layout | Module | Duration | Exchanges | Drops | Transient rate | Sustained underrun | Pass |
|----|--------------|--------|----------|-----------|-------|----------------|-------------------|------|
| S1 | GW + 7 WASM + HW end | passthrough | 600.0 s | 72,377 | 21 | 0.029% | 0 | **PASS** |
| S2 | GW + WASM + HW delay + WASM | delay | 600.0 s | 59,617 | 22 | 0.037% | 0 | **PASS** |
| S3 | GW + HW merger + 2 WASM | merger | 605.7 s | 67,925 | 19 | 0.028% | 0 | **PASS** |
| S4 | GW + WASM + HW cutoff + WASM merger (loopback) | cutoff | 600.0 s | 74,761 | 15 | 0.020% | 0 | **PASS** |
| S5 | GW + WASM + HW debug_tone + WASM | debug_tone | 601.4 s | 59,331 | 25 | 0.042% | 0 | **PASS** |
| S6 | GW + max interleave, HW middle | delay | 615.1 s | 61,058 | 25 | 0.041% | 0 | **PASS** |

Raw JSON: `phase3-attestation-matrix-run.log` (S1), `phase3-attestation-matrix-run-S2-S6.log` (S2–S6).

## Tooling evidence

| Check | Result |
|-------|--------|
| `./scripts/build.sh all` | exit 0 (all five module kinds) |
| `node sim/verify-wasm.mjs` | exit 0 |
| `node sim/hardware-realtime/acceptance.mjs loopback --audio-port /dev/ttyACM0` | PASS (post payload-size fix) |
| `node sim/hardware-bridge/processing-budget.mjs` | see below |
| `npx @fission-ai/openspec@latest validate phase3-hardware-chain-realtime-hardening --type change` | valid |

## Latency

Published budget: ~5.5 periods (~32 ms) for HW between two WASM neighbors ± 1 period. Bridge relay soak validates sustained USB duplex at buffer cadence; full mixed-chain PWA topology confirmed per scenario in host UI.

## Notes

- Transient drops (timeouts) are sub-0.05% and recover without sequence gaps or sustained underrun windows.
- Harness uses depth-2 pipelining aligned with production bridge relay path.
- Run matrix: `SOAK_DURATION_S=600 bash sim/hardware-bridge/run-matrix-soaks.sh`
