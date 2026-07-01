## 1. Toolchain configuration

- [ ] 1.1 Add `arduino-cli.yaml` pinning ESP32-S3-Zero FQBN options (OPI PSRAM, USB CDC on boot, 4MB flash)
- [ ] 1.2 Add `.env.example` documenting optional `FIRMWARE_PORT` for upload/monitor (keep `.env` gitignored)

## 2. Build and upload scripts

- [ ] 2.1 Add `scripts/build.sh` accepting module name (delay, merger, cutoff, debug_tone, passthrough) and optional `all` to compile each variant
- [ ] 2.2 Add `scripts/upload.sh` resolving serial port from `.env` or sole connected device, then flashing after compile
- [ ] 2.3 Add `scripts/monitor.sh` (or equivalent) capturing serial output for a bounded duration after upload

## 3. Documentation

- [ ] 3.1 Add README "Build and flash" section: prerequisites (arduino-cli, esp32 core, dialout), build/upload/monitor usage, BOOT-button note for S3-Zero
- [ ] 3.2 Update `AGENTS.md` guide index to reference README build section for compile/upload verification

## 4. Verification

- [ ] 4.1 Run `scripts/build.sh all` (or each module) and confirm clean compile; record output in apply notes
- [ ] 4.2 With hardware connected: run upload + monitor, confirm serial boot line (e.g. setup complete); record port and output in apply notes — requires human-attested connected ESP32-S3-Zero

## Explicitly deferred

- PlatformIO support
- CI pipeline with hardware-in-the-loop
- Automatic `compile_commands.json` regeneration for clangd
- Updating `delay-module-spec-and-implementation` task 4.1 to reference new scripts (follow-up refine or done during delay apply if this change lands first)
