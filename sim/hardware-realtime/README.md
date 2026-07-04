# Hardware realtime USB acceptance (Phase 1)

Standalone host tool for sustained duplex exchange with the ESP32-S3 passthrough firmware over the binary USB audio CDC interface.

## Prerequisites

- Node.js ≥ 20
- Passthrough firmware flashed (`./scripts/upload.sh passthrough`)
- `npm install --prefix sim/hardware-realtime`

## Dual CDC vs single CDC

**Preferred:** firmware exposes two USB CDC interfaces — binary audio on the secondary port (`BridgeAudio`), diagnostic `Serial` on the primary. On Linux this is usually `/dev/ttyACM0` (logs) and `/dev/ttyACM1` (audio).

**Fallback:** when only one CDC port appears, use that port for `--audio-port` and rely on exclusive binary mode (`BRIDGE_AUDIO_EXCLUSIVE=1` in `build_opt.h`) so `Serial` logging is suppressed during USB neighbor mode.

## Quiescing logs before acceptance

1. Identify ports: `ls /dev/ttyACM*`
2. Bind the host tool to the **audio** port only (`--audio-port`).
3. Do not run `scripts/monitor.sh` or other tools on the audio port during sustained runs.
4. Optional: redirect firmware logs to the primary CDC while testing on the secondary.

## Commands

Transport loopback (identity before module processing):

```bash
node sim/hardware-realtime/acceptance.mjs loopback --audio-port /dev/ttyACM0
```

60 s sustained acceptance (≥10k exchanges, zero drops, latency thresholds from `design.md`):

```bash
node sim/hardware-realtime/acceptance.mjs sustained --audio-port /dev/ttyACM0
```

Environment: `FIRMWARE_PORT` is used when `--audio-port` is omitted.

## Acceptance thresholds

| Metric | Threshold |
|--------|-----------|
| Duration | ≥ 60 s |
| Exchanges | ≥ 10,000 |
| Drops | 0 |
| Round-trip p50 | ≤ 12 ms |
| Round-trip p99 | ≤ 20 ms |
| Passthrough identity | bit-exact every exchange |

Output is JSON plus `OVERALL: PASS` or `OVERALL: FAIL`.
