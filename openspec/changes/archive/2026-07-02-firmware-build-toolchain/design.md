## Context

The repo uses Arduino IDE layout (`i2s_synths.ino` + `src/`). Developers historically compiled via IDE or manual `arduino-cli`. A stale `compile_commands.json` exists for clangd from a prior path. ESP32-S3-Zero boards use native USB CDC (no USB-UART chip) and 2MB OPI PSRAM — default `esp32s3` FQBN options disable PSRAM and CDC-on-boot, which breaks delay allocation and serial logging.

`arduino-cli` 1.3.1 and esp32 core 3.3.0 are already installed on the primary dev host; compile succeeds with corrected FQBN options.

## Goals / Non-Goals

**Goals:**

- Pin ESP32-S3-Zero-oriented build flags in repo config.
- Provide `scripts/build.sh` and `scripts/upload.sh` (and optionally `scripts/monitor.sh`) as stable agent entrypoints.
- Support `MODULE` argument mapping to `-DACTIVE_MODULE=...` compile defines.
- Document prerequisites (arduino-cli, esp32 core, dialout group) in README.
- Prove compile on apply; upload/monitor when human has hardware connected.

**Non-Goals:**

- PlatformIO migration.
- CI runners with hardware-in-the-loop (local agent + user board is enough for PoC).
- Gateway-module or non-S3-Zero targets in this change.
- Regenerating `compile_commands.json` automatically (optional follow-up).

## Decisions

### 1. arduino-cli over PlatformIO

**Choice:** Standardize on `arduino-cli` with `arduino-cli.yaml` in repo root.

**Rationale:** Matches existing layout; compile already works; no `platformio.ini` migration.

**Alternatives:** PlatformIO env matrix — better for CI later, higher migration cost now.

### 2. Pinned FQBN options for S3-Zero

**Choice:**

```
esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc,USBMode=hwcdc,FlashSize=4M
```

**Rationale:** Waveshare S3-Zero has 4MB flash + 2MB OPI PSRAM; USB CDC required per board wiki.

### 3. Module selection via build.extra_flags

**Choice:** `scripts/build.sh [delay|merger|cutoff|debug_tone|passthrough]` passes `-DACTIVE_MODULE=MODULE_*` without editing `constants.h`.

**Rationale:** Agents can build variants reproducibly; keeps `constants.h` default for IDE users.

### 4. Serial port resolution

**Choice:** `FIRMWARE_PORT` in gitignored `.env`; if unset, auto-select sole `ttyACM*` / `ttyUSB*` when exactly one match; otherwise fail with instructions.

**Rationale:** Safe default for single-board desk; explicit override for multi-device setups.

### 5. Upload evidence for apply-complete

**Choice:** Apply tasks require compile proof always; upload + serial capture when human attests hardware is connected (same-turn or named environment in finish notes).

**Rationale:** Agents cannot upload without hardware; compile-only is still valuable evidence.

## Risks / Trade-offs

- **[Risk] BOOT button needed on some flashes** → Document in README; upload script prints reminder.
- **[Risk] Linux dialout permissions** → README prerequisite; upload failure message mentions group membership.
- **[Risk] Multiple serial devices** → Require `FIRMWARE_PORT` when auto-detect is ambiguous.
- **[Risk] Delay change task 4.1 still says manual compile** → Note to update delay `tasks.md` during apply or a tiny refine after this lands.

## Migration Plan

1. Apply adds `arduino-cli.yaml`, scripts, README section, `.env.example`.
2. Run `scripts/build.sh delay` — capture success output in apply notes.
3. When hardware available: `scripts/upload.sh` + brief monitor — capture "Setup complete." line.
4. Rollback: delete new scripts/config; no firmware behavior change.

## Open Questions

- Whether to add `scripts/build.sh all` that compiles every module in one pass for CI-style checks — lean yes, low cost.
- Whether `AGENTS.md` preflight should mandate compile after module edits — defer to separate doc tweak.
