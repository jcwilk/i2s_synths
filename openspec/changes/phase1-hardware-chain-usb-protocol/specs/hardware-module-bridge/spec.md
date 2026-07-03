## ADDED Requirements

### Requirement: Realtime USB duplex buffer geometry

The hardware module bridge realtime exchange SHALL use 44.1 kHz sample rate, stereo interleaved 16-bit signed PCM, and a fixed per-exchange sample count matching the firmware streaming buffer length (512 int16 samples per audio path per exchange), with no downsampling, compression, or mono fallback.

#### Scenario: Realtime geometry matches firmware streaming

- **GIVEN** a sustained realtime USB duplex session is active
- **WHEN** one exchange completes between host and device
- **THEN** each audio path in the exchange contains exactly the firmware buffer period sample count at full-rate stereo encoding

### Requirement: Realtime four-path duplex contract

Each realtime exchange SHALL carry downstream input and upstream input audio for one buffer period and SHALL return downstream output and upstream output audio for the same period, representing one virtual chain neighbor dual-path processing step under host-driven timing.

#### Scenario: Duplex inputs and outputs per exchange

- **GIVEN** the host submits non-silent downstream and upstream inputs for one exchange
- **WHEN** the device completes processing for that exchange in USB neighbor mode
- **THEN** the host receives downstream and upstream output buffers for the same exchange period

### Requirement: Realtime dual-path invocation order

For each realtime exchange, the device SHALL invoke upstream processing before downstream processing with the same injected control state for both invocations, matching normal firmware module loop ordering.

#### Scenario: Path order preserved in realtime

- **GIVEN** USB neighbor mode is active
- **WHEN** one realtime exchange is processed
- **THEN** upstream processing completes before downstream processing begins for that exchange

### Requirement: USB neighbor operating mode

The firmware SHALL support an explicit USB neighbor operating mode in which buffer periods are supplied by a host over a binary USB duplex channel, physical I2S exchange with chain neighbors is disabled, and normal I2S streaming SHALL remain the default when USB neighbor mode is not active.

#### Scenario: Enter USB neighbor mode

- **GIVEN** firmware is running in normal mode with live I2S streaming available
- **WHEN** the host issues the documented entry action for USB neighbor mode
- **THEN** the device stops using live I2S neighbor input for module processing and accepts host-supplied realtime buffer exchanges

#### Scenario: Exit USB neighbor mode

- **GIVEN** firmware is in USB neighbor mode
- **WHEN** the host issues the documented exit action
- **THEN** the device returns to normal I2S neighbor streaming without requiring a full reflash

#### Scenario: Normal boot unchanged with USB capability present

- **GIVEN** firmware includes USB neighbor capability
- **WHEN** the device boots and no USB neighbor entry action is performed
- **THEN** behavior matches pre-bridge normal I2S operation

### Requirement: Host-driven exchange cadence

The host SHALL act as clock master for realtime exchanges, scheduling submissions at the firmware buffer period cadence derived from the 44.1 kHz sample rate and per-path sample count, and the device SHALL process each complete exchange without requiring alignment to a live audio clock on the device.

#### Scenario: Sustained cadence over acceptance duration

- **GIVEN** a host reference tool runs the documented sustained acceptance session
- **WHEN** exchanges are scheduled at firmware buffer period cadence for the full session duration
- **THEN** the device processes each submitted exchange and returns a corresponding response without requiring device-side self-clocking of input production

#### Scenario: Incomplete exchange not applied

- **GIVEN** USB neighbor mode is active
- **WHEN** the host submits a partial or malformed exchange payload
- **THEN** the device does not advance module state as if a full period completed and reports failure status for that exchange

### Requirement: Exchange sequencing

Each realtime data exchange SHALL carry a monotonic sequence identifier assigned by the host, and the device response SHALL echo the same identifier for that exchange.

#### Scenario: Matching sequence on response

- **GIVEN** the host submits exchange sequence number N with a complete payload
- **WHEN** the device returns the processing result
- **THEN** the response sequence identifier equals N

#### Scenario: Sequence gap detected

- **GIVEN** the device receives exchange sequence number N while the prior processed exchange was not N minus one
- **WHEN** the device evaluates the incoming exchange
- **THEN** the device reports a sequence gap in status and the host reference tool treats the session as failed

### Requirement: Realtime backpressure and drop reporting

When the device cannot accept a new exchange or cannot complete processing in time for the realtime session, it SHALL report overrun or underrun status, and the host reference tool SHALL count such events as drops.

#### Scenario: Receive overrun

- **GIVEN** a sustained realtime session is active
- **WHEN** the host submits a new exchange while the device receive path is saturated
- **THEN** the device reports overrun status and the host counts one drop

#### Scenario: Missing response within deadline

- **GIVEN** the host submitted a complete exchange
- **WHEN** no valid response arrives within the documented response deadline for the acceptance session
- **THEN** the host counts one drop and the sustained acceptance run fails if drops exceed zero

### Requirement: Injected control state in realtime mode

During realtime USB exchange, primary and secondary control values SHALL be supplied by the host in the normalized zero-to-one range for each exchange, and physical potentiometer readings SHALL NOT be used as the control source for that exchange.

#### Scenario: Controls from host per exchange

- **GIVEN** USB neighbor mode is active
- **WHEN** the host submits an exchange with specified primary and secondary values
- **THEN** module processing for that exchange reflects those values and not live ADC readings

### Requirement: Binary audio transport isolation

During sustained realtime acceptance, diagnostic human-readable logging SHALL NOT share the same transport channel as the binary audio exchange unless the device and host use a documented isolation strategy that prevents text output from corrupting audio frame boundaries.

#### Scenario: Acceptance run uses isolated audio channel

- **GIVEN** a sustained realtime acceptance session is starting
- **WHEN** the host opens the binary audio duplex channel
- **THEN** frame parsing completes without interleaved diagnostic text corrupting exchange boundaries for the session duration

### Requirement: Sustained realtime acceptance thresholds

The host reference tool SHALL run a documented sustained acceptance session at firmware buffer cadence for a minimum continuous duration, complete at least the documented minimum exchange count with zero drops, and record round-trip latency percentiles that do not exceed the documented maximum thresholds.

#### Scenario: Zero-drop sustained run

- **GIVEN** a passthrough firmware build and connected device
- **WHEN** the host reference tool completes the sustained acceptance session
- **THEN** the drop count is zero and at least the documented minimum number of exchanges completed

#### Scenario: Latency within bounds

- **GIVEN** a successful sustained acceptance session with zero drops
- **WHEN** round-trip latency is summarized from send to response receive timestamps
- **THEN** the median latency does not exceed the documented median threshold and the ninety-ninth percentile does not exceed the documented tail threshold

### Requirement: Passthrough identity under realtime load

For passthrough module firmware builds, each realtime exchange during the sustained acceptance session SHALL produce downstream output identical to downstream input and upstream output identical to upstream input on every sample.

#### Scenario: Bit-exact passthrough per exchange

- **GIVEN** a passthrough module build in USB neighbor mode during the sustained acceptance session
- **WHEN** the host compares inputs and outputs for any completed exchange
- **THEN** every sample on both paths matches between input and output

#### Scenario: Identity failure fails acceptance

- **GIVEN** any completed exchange during the sustained session
- **WHEN** any sample on either path differs between input and output
- **THEN** the host reference tool reports overall failure for the acceptance run

### Requirement: Realtime metrics reporting

The host reference tool SHALL emit a structured summary after a sustained realtime session including exchange count, drop count, latency percentiles, and overall pass or fail against the documented acceptance thresholds.

#### Scenario: Successful realtime acceptance report

- **GIVEN** all acceptance thresholds are met including zero drops and passthrough identity
- **WHEN** the sustained session ends
- **THEN** the tool reports overall pass with exchange count and latency summary

#### Scenario: Failed realtime acceptance report

- **GIVEN** any acceptance threshold is violated
- **WHEN** the sustained session ends or aborts
- **THEN** the tool reports overall fail and identifies the first violated threshold

### Requirement: Passthrough realtime MVP scope

The hardware module bridge realtime USB validation SHALL deliver acceptance coverage for the passthrough module kind in the initial Phase 1 delivery, and the realtime exchange contract SHALL not require redesign to add other module kinds in later phases.

#### Scenario: Passthrough sustained acceptance

- **GIVEN** firmware built for the passthrough module kind
- **WHEN** the documented sustained realtime acceptance session is executed against a connected device
- **THEN** the host reference tool reports pass under zero-drop, latency, and identity rules
