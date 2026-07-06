# hardware-module-bridge Specification

## Purpose
TBD - created by archiving change phase0-hardware-chain-offline-ab. Update Purpose after archive.
## Requirements
### Requirement: Firmware-equivalent offline buffer geometry

The hardware module bridge offline exchange SHALL use 22.05 kHz sample rate, mono 16-bit signed PCM per audio path, and a fixed per-period sample count matching the firmware streaming buffer length (128 int16 samples per audio path per exchange).

#### Scenario: Buffer geometry matches firmware streaming

- **GIVEN** an offline exchange is configured for a supported module kind
- **WHEN** one buffer period is submitted to the device and outputs are collected
- **THEN** each audio path input and output contains exactly the firmware buffer period sample count at the firmware sample rate and mono encoding

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

### Requirement: Realtime USB duplex buffer geometry

The hardware module bridge realtime exchange SHALL use 22.05 kHz sample rate, mono 16-bit signed PCM per audio path, and a fixed per-exchange sample count matching the firmware streaming buffer length (128 int16 samples per audio path per exchange), using the same geometry as offline neighbor mode.

#### Scenario: Realtime geometry matches firmware streaming

- **GIVEN** a sustained realtime USB duplex session is active
- **WHEN** one exchange completes between host and device
- **THEN** each audio path in the exchange contains exactly the firmware buffer period sample count at the mono 22.05 kHz encoding

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

The host SHALL act as clock master for realtime exchanges, scheduling submissions at the firmware buffer period cadence derived from the 22.05 kHz sample rate and per-path sample count, and the device SHALL process each complete exchange without requiring alignment to a live audio clock on the device.

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

The host reference tool SHALL run a documented sustained acceptance session at firmware buffer cadence for a minimum continuous duration, complete at least the documented minimum exchange count with zero drops, achieve a realtime ratio of at least one, and record round-trip latency percentiles that do not exceed the documented maximum thresholds.

#### Scenario: Zero-drop sustained run

- **GIVEN** a passthrough firmware build and connected device
- **WHEN** the host reference tool completes the sustained acceptance session
- **THEN** the drop count is zero and at least the documented minimum number of exchanges completed

#### Scenario: Realtime ratio at or above unity

- **GIVEN** a successful sustained acceptance session with zero drops
- **WHEN** audio-time processed is compared to wall-clock session duration
- **THEN** the realtime ratio is at least one

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

The host reference tool SHALL emit a structured summary after a sustained realtime session including exchange count, drop count, realtime ratio, latency percentiles, and overall pass or fail against the documented acceptance thresholds.

#### Scenario: Successful realtime acceptance report

- **GIVEN** all acceptance thresholds are met including zero drops, realtime ratio, and passthrough identity
- **WHEN** the sustained session ends
- **THEN** the tool reports overall pass with exchange count, realtime ratio, and latency summary

#### Scenario: Failed realtime acceptance report

- **GIVEN** any acceptance threshold is violated
- **WHEN** the sustained session ends or aborts
- **THEN** the tool reports overall fail and identifies the first violated threshold

### Requirement: Passthrough realtime MVP scope

The hardware module bridge realtime USB validation SHALL deliver acceptance coverage for the passthrough module kind in the initial Phase 1 delivery, and the realtime exchange contract SHALL not require redesign to add other module kinds in later phases.

#### Scenario: Passthrough sustained acceptance

- **GIVEN** firmware built for the passthrough module kind
- **WHEN** the documented sustained realtime acceptance session is executed against a connected device
- **THEN** the host reference tool reports pass under zero-drop, realtime ratio, latency, and identity rules

### Requirement: Local bridge server relay

The hardware module bridge SHALL provide a local bridge server process that accepts duplex binary exchanges from the module chain simulator over a WebSocket connection and relays them to a connected ESP32 module over USB using the Phase 1 realtime four-path exchange contract, without altering the universal mono 22.05 kHz audio sample geometry or introducing compression.

#### Scenario: WebSocket client connects to bridge

- **GIVEN** the bridge server is running and a device is connected over USB
- **WHEN** the module chain simulator opens a WebSocket session to the bridge server
- **THEN** the bridge accepts the session and is ready to relay complete buffer-period exchanges in both directions

#### Scenario: Relay preserves mono path geometry

- **GIVEN** an active WebSocket session and USB neighbor mode on the device
- **WHEN** the simulator submits one complete exchange for a buffer period
- **THEN** the device receives downstream and upstream inputs at 22.05 kHz mono int16 fidelity and returns downstream and upstream outputs at the same geometry

### Requirement: Single device per bridge session

The bridge server SHALL support exactly one connected ESP32 module per active relay session. Attempts to attach a second device while a session is active SHALL be rejected until the current session ends.

#### Scenario: One device bound to session

- **GIVEN** a bridge session is already relaying for a connected module
- **WHEN** another module connection is offered to the same bridge instance
- **THEN** the bridge rejects the additional connection and the existing session continues unchanged

### Requirement: PWA duplex session lifecycle

The bridge server and module chain simulator SHALL coordinate an explicit session lifecycle: connect, enter USB neighbor processing on the device, sustain exchanges while audio runs, and disconnect with orderly teardown that returns the device to normal I2S behavior when the session ends.

#### Scenario: Session start enters USB neighbor mode

- **GIVEN** the simulator requests a hardware connection and the bridge has a device attached
- **WHEN** the session start action completes successfully
- **THEN** the device is in USB neighbor mode and the simulator can submit realtime exchanges through the bridge

#### Scenario: Session end restores normal device mode

- **GIVEN** an active hardware relay session
- **WHEN** the operator disconnects or the simulator tears down the session
- **THEN** the bridge closes the WebSocket session, stops relaying, and the device exits USB neighbor mode without requiring a reflash

#### Scenario: Disconnect while audio running

- **GIVEN** audio is running through a hardware slot
- **WHEN** the operator disconnects the hardware session
- **THEN** the simulator stops treating the slot as connected, ceases USB exchanges for that slot, and follows structural reconfiguration rules for the chain

### Requirement: Physical controls for PWA hardware slot

When the device serves the module chain simulator as an occupied hardware slot, primary and secondary control values for module processing SHALL be derived from on-device physical potentiometer readings with firmware-aligned smoothing, and values supplied in the exchange payload SHALL NOT override those readings for that session.

#### Scenario: Hardware slot ignores host-injected controls

- **GIVEN** a delay module device in an active PWA hardware slot session
- **WHEN** the simulator submits exchanges while the operator turns a physical potentiometer
- **THEN** audible processing reflects the physical control position rather than any control fields in the exchange payload

#### Scenario: Smoothed ADC matches firmware polling behavior

- **GIVEN** a hardware slot session with audio running
- **WHEN** the operator holds a physical control at a steady position
- **THEN** the smoothed control state used for module processing approaches that position over multiple buffer periods consistent with normal firmware pot polling

### Requirement: Module kind parity with flashed firmware

The bridge session and simulator hardware slot SHALL require that the operator-selected module kind for the hardware slot matches the module kind compiled into the connected device firmware. The system SHALL refuse or clearly block sustained audio relay when kinds disagree.

#### Scenario: Matching delay firmware and slot selection

- **GIVEN** firmware built for the delay module kind is connected
- **WHEN** the operator marks a slot as hardware with delay selected and starts a session
- **THEN** relay proceeds and delay processing is audible in the chain

#### Scenario: Mismatched module kind blocked

- **GIVEN** firmware built for the delay module kind is connected
- **WHEN** the operator marks a hardware slot with a different module kind selected
- **THEN** the system does not start sustained relay and presents a visible module-kind mismatch indication

### Requirement: Bridge connection status visibility

The bridge server and simulator SHALL expose connection status visible to the operator, including whether the bridge process is reachable, whether a USB device is attached, and whether an active relay session is in progress.

#### Scenario: Disconnected bridge indicated

- **GIVEN** the simulator is open and no bridge server is reachable
- **WHEN** the operator views hardware connection controls
- **THEN** the operator sees that the bridge is unavailable and cannot start a hardware session

#### Scenario: Device attached without session

- **GIVEN** the bridge server is running with a USB device attached but no active WebSocket session
- **WHEN** the operator views hardware connection controls
- **THEN** the operator sees the device as attached and may start a session when a slot is designated hardware

### Requirement: Delay module PWA integration acceptance

The hardware module bridge PWA integration SHALL include acceptance coverage demonstrating sustained duplex relay of the delay module kind through the bridge into a running module chain simulator session with at least one neighboring WASM processing unit, using microphone input and speaker output at full fidelity.

#### Scenario: Delay hardware slot in mixed chain

- **GIVEN** a delay firmware device, running bridge server, and a chain with the hardware slot between WASM neighbors
- **WHEN** the operator enables microphone input and runs audio for a documented minimum duration
- **THEN** delayed effect from the hardware slot is audible, physical pots change the effect, and the session completes without sustained drop events

#### Scenario: Neighbor WASM units remain active

- **GIVEN** a mixed chain with WASM units on both sides of the hardware slot
- **WHEN** audio runs through the full chain
- **THEN** processing from WASM neighbors remains audible and the hardware slot participates in downstream and upstream wiring

### Requirement: Documented USB round-trip latency delta

The hardware module bridge documentation SHALL state that substituting a hardware slot introduces additional end-to-end latency from USB round-trip and bridge relay compared with an all-WASM chain at the same buffer geometry, and SHALL describe how operators should interpret that delta during audition.

#### Scenario: Operator reads latency guidance

- **GIVEN** documentation for PWA hardware slot integration
- **WHEN** an operator reviews known limitations before hardware audition
- **THEN** added USB and bridge latency is explicitly named as an expected difference from pure simulator chains

