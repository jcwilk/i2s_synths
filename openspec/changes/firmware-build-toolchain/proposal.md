## Why

Firmware changes in this repo currently lack a headless, repeatable compile-and-upload path that agents can run for verification. The delay change and future module work need apply-complete evidence (build success, optional hardware upload) without relying on the Arduino IDE GUI or ad hoc command recall.

Exploration confirmed `arduino-cli` already compiles this sketch layout successfully; the gap is pinned target configuration for ESP32-S3-Zero, thin automation wrappers, and a behavioral contract agents can follow.

## What Changes

- Introduce a **firmware-build** capability spec describing headless compile, module selection at build time, and scriptable upload to connected hardware.
- Add repository tooling: pinned board target for ESP32-S3-Zero (OPI PSRAM, USB CDC), build/upload wrapper scripts, optional serial-port configuration, and human-facing README guidance.
- Document agent workflow in apply tasks: compile all module variants or a selected module, upload when hardware is available, capture serial boot evidence.
- Cross-reference from the in-flight delay change's verification tasks (note in design; optional follow-up edit to delay `tasks.md` is deferred to apply or a small refine).

## Capabilities

### New Capabilities

- `firmware-build`: Headless firmware compilation and upload for the ESP32-S3-Zero control-module target, with compile-time module selection.

### Modified Capabilities

<!-- None — greenfield spec domain -->

## Impact

- **Specs:** New `firmware-build` living spec after archive.
- **Repo:** New scripts and CLI config under repo root; README build/flash section; optional `.env.example` for serial port (`.env` stays gitignored).
- **Agents:** `AGENTS.md` guide index may gain a pointer to build docs (task during apply, not in this propose turn).
- **Dependencies:** Requires `arduino-cli` and esp32 Arduino core on the build host; USB permissions for upload on Linux.
- **Unrelated:** Does not change DSP/module behavior; does not block the delay-module change but unblocks its verification story.
