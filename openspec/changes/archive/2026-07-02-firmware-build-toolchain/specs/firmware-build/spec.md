## ADDED Requirements

### Requirement: Headless firmware compilation

The project SHALL provide a documented, non-interactive way to compile the main sketch firmware without the Arduino IDE GUI. A successful compilation SHALL produce loadable firmware artifacts and exit with success only when the build completes without errors.

#### Scenario: Clean compile on build host

- **WHEN** an agent or developer runs the project's standard compile entrypoint on a host with required toolchain dependencies installed
- **THEN** the firmware build completes successfully and reports program and memory usage

#### Scenario: Compile failure is visible

- **WHEN** source code contains a build error
- **THEN** the compile entrypoint exits non-zero and emits compiler diagnostics to standard error or standard output

### Requirement: ESP32-S3-Zero target configuration

Firmware builds SHALL target the ESP32-S3-Zero control module: 4MB flash, OPI PSRAM enabled, and USB CDC serial suitable for boards without an external USB-UART bridge.

#### Scenario: PSRAM available to firmware

- **WHEN** firmware is built with the project's standard target configuration
- **THEN** PSRAM is enabled at build time so modules that allocate delay buffers in external RAM can run on hardware

#### Scenario: USB serial for boot logs

- **WHEN** firmware built with the standard target is flashed to an ESP32-S3-Zero
- **THEN** boot messages are available over the board's USB CDC serial interface at the configured baud rate

### Requirement: Compile-time module selection

The build system SHALL allow selecting which audio module is active at compile time without editing source files manually. Each supported module identifier SHALL be buildable using the same entrypoint with a module parameter.

#### Scenario: Build delay module variant

- **WHEN** compile is requested for the delay module selection
- **THEN** the resulting firmware uses the delay module as the active processor

#### Scenario: Build passthrough module variant

- **WHEN** compile is requested for the passthrough module selection
- **THEN** the resulting firmware uses the passthrough module as the active processor

### Requirement: Scriptable firmware upload

The project SHALL provide a documented, non-interactive upload entrypoint that flashes compiled firmware to a connected ESP32-S3-Zero when a serial port is available.

#### Scenario: Upload to connected board

- **WHEN** exactly one suitable serial device is connected and upload is invoked after a successful compile
- **THEN** firmware is written to the board and the entrypoint exits successfully

#### Scenario: Upload without a connected board

- **WHEN** no serial port is configured or discoverable
- **THEN** upload fails fast with a clear message indicating that a port must be supplied or connected

### Requirement: Serial boot verification

The project SHALL provide a documented way to read recent serial output from a flashed board so agents can capture boot evidence after upload.

#### Scenario: Capture setup banner

- **WHEN** serial monitoring is invoked after a successful upload
- **THEN** the operator can observe firmware boot messages including setup completion within a bounded monitoring window
