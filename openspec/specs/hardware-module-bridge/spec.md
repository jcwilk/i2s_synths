# hardware-module-bridge Specification

## Purpose
TBD - created by archiving change phase0-hardware-chain-offline-ab. Update Purpose after archive.
## Requirements
### Requirement: Firmware-equivalent offline buffer geometry

The hardware module bridge offline exchange SHALL use 44.1 kHz sample rate, stereo interleaved 16-bit signed PCM, and a fixed per-period sample count matching the firmware streaming buffer length (512 int16 samples per audio path per exchange).

#### Scenario: Buffer geometry matches firmware streaming

- **GIVEN** an offline exchange is configured for a supported module kind
- **WHEN** one buffer period is submitted to the device and outputs are collected
- **THEN** each audio path input and output contains exactly the firmware buffer period sample count at the firmware sample rate encoding

### Requirement: Four-path neighbor buffer contract

Each offline exchange period SHALL carry downstream input and upstream input audio for one buffer period and SHALL return downstream output and upstream output audio for the same period, representing one virtual chain neighbor's dual-path processing step.

#### Scenario: Dual inputs and dual outputs per period

- **GIVEN** a host supplies non-silent downstream input and upstream input for one period
- **WHEN** the device completes processing for that period in offline neighbor mode
- **THEN** the host receives downstream output and upstream output buffers corresponding to that same period

#### Scenario: Silent inputs produce module-consistent outputs

- **GIVEN** a host supplies silence on both input paths for one period
- **WHEN** the device completes processing in offline neighbor mode
- **THEN** outputs reflect the active module's defined behavior for silent inputs (including startup or state-fill rules for stateful modules)

### Requirement: Dual-path invocation order offline

For each offline exchange period, the device SHALL invoke upstream processing before downstream processing, supplying the same control state to both invocations, matching normal firmware module loop ordering.

#### Scenario: Path order preserved offline

- **GIVEN** offline neighbor mode is active
- **WHEN** one exchange period is processed
- **THEN** upstream processing completes before downstream processing begins for that period

### Requirement: Offline neighbor operating mode

The firmware SHALL support an explicit offline neighbor operating mode in which buffer periods are supplied by a host validation harness instead of live I2S capture, and normal I2S streaming SHALL remain the default when offline neighbor mode is not active.

#### Scenario: Enter offline mode

- **GIVEN** firmware is running in normal mode with live I2S streaming available
- **WHEN** the host issues the documented entry action for offline neighbor mode
- **THEN** the device stops consuming live I2S input for module processing and awaits host-supplied buffer periods

#### Scenario: Exit offline mode

- **GIVEN** firmware is in offline neighbor mode
- **WHEN** the host issues the documented exit action
- **THEN** the device returns to normal I2S streaming behavior without requiring a full reflash

#### Scenario: Normal boot unchanged

- **GIVEN** firmware is flashed with offline neighbor capability present
- **WHEN** the device boots and no offline entry action is performed
- **THEN** behavior matches pre-bridge normal I2S operation

### Requirement: Deterministic host-driven exchange

The offline validation harness SHALL drive buffer periods in a deterministic order chosen by the test vector, without requiring realtime audio clock alignment between host and device.

#### Scenario: Multi-period sequence

- **GIVEN** a test vector defining multiple consecutive buffer periods with specified inputs and controls
- **WHEN** the harness runs the full sequence against the device in offline neighbor mode
- **THEN** the device processes periods in submission order and returns one output pair per submitted period

#### Scenario: No realtime deadline

- **GIVEN** the host pauses between submitting periods
- **WHEN** the device eventually receives the next period
- **THEN** processing remains correct for that period and prior module state is preserved

### Requirement: Injected control state

During offline exchange, primary and secondary control values SHALL be supplied by the host in the normalized zero-to-one range for each period (or held constant across a run as defined by the test vector), and physical potentiometer ADC readings SHALL NOT be used as the control source for that exchange.

#### Scenario: Fixed controls across run

- **GIVEN** a test vector specifies constant primary and secondary values for all periods
- **WHEN** the harness runs the sequence
- **THEN** module behavior reflects those control values consistently across periods

#### Scenario: Stepped controls

- **GIVEN** a test vector changes primary or secondary values between periods
- **WHEN** the harness submits each period with its specified controls
- **THEN** audible-equivalent processing outcomes change according to the active module's control mapping on subsequent periods

### Requirement: Stateful module continuity

For module kinds that retain internal state between buffer periods, offline multi-period runs SHALL preserve that state across consecutive exchanges in the same session unless the harness explicitly resets the module.

#### Scenario: Delay state carries across periods

- **GIVEN** a delay module in offline neighbor mode and a multi-period test vector with continuous non-silent downstream input
- **WHEN** periods are processed sequentially without an explicit reset between them
- **THEN** later-period downstream output reflects delayed content from earlier periods rather than behaving as a fresh instance each period

### Requirement: Reference comparison against simulator-equivalent processing

The offline validation harness SHALL compare device downstream and upstream outputs against a reference produced by processing the identical input sequence, control state, and module kind through the same firmware module sources used by the module chain simulator (directly or via precomputed golden vectors derived from that processing).

#### Scenario: Passthrough reference match

- **GIVEN** a passthrough module build and a test vector with arbitrary deterministic inputs
- **WHEN** the harness completes an offline run and compares outputs to the reference
- **THEN** downstream and upstream outputs are bit-exact with the reference on every sample

#### Scenario: Delay reference within tolerance

- **GIVEN** a delay module build and a test vector that exercises length and speed controls after any documented warm-up periods
- **WHEN** the harness compares device outputs to the reference
- **THEN** every sample on both paths is within the documented maximum absolute deviation for the delay module kind

#### Scenario: Mismatch fails the run

- **GIVEN** a completed offline capture and reference pair for the same test vector
- **WHEN** any sample on either path exceeds the applicable comparison threshold
- **THEN** the harness reports failure and identifies the first offending period and path

### Requirement: Comparison reporting

The offline validation harness SHALL emit a structured pass or fail result for each run, including module kind, period count compared, and summary divergence metrics when failing.

#### Scenario: Successful run report

- **GIVEN** all compared samples meet the applicable thresholds
- **WHEN** the harness finishes comparison
- **THEN** the harness reports overall pass for that module kind and test vector

#### Scenario: Failed run diagnostics

- **GIVEN** at least one sample exceeds the applicable threshold
- **WHEN** the harness finishes comparison
- **THEN** the harness reports overall fail and supplies the period index, path, and maximum observed deviation

### Requirement: MVP module kind coverage

The hardware module bridge offline validation SHALL include working acceptance coverage for passthrough and delay module kinds in the initial delivery, and the buffer exchange contract SHALL not require redesign to add merger, cutoff, or debug-tone module kinds later.

#### Scenario: Passthrough acceptance

- **GIVEN** firmware built for the passthrough module kind
- **WHEN** the standard passthrough offline test vector is executed
- **THEN** the harness reports pass under bit-exact comparison rules

#### Scenario: Delay acceptance

- **GIVEN** firmware built for the delay module kind
- **WHEN** the standard delay offline test vector is executed after documented warm-up periods
- **THEN** the harness reports pass under delay tolerance rules

### Requirement: Module build parity

Offline neighbor mode SHALL execute the same compile-time-selected module variant used for normal firmware builds of that module kind, without substituting alternate or stub processing logic in offline mode.

#### Scenario: Same variant as production build

- **GIVEN** firmware compiled for a specific module kind
- **WHEN** offline neighbor mode processes a buffer period
- **THEN** the processing logic is the same variant used for I2S streaming for that build

