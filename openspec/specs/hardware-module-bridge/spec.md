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

The bridge server and module chain simulator SHALL coordinate an explicit session lifecycle: connect, enter USB neighbor processing on the device, sustain exchanges while audio runs, detect link loss and enter degraded recovery when applicable, allow operator reconnect without full browser restart, and disconnect with orderly teardown that returns the device to normal I2S behavior when the session ends. Web Serial direct sessions SHALL follow the same lifecycle semantics without requiring the bridge server.

#### Scenario: Session start enters USB neighbor mode

- **GIVEN** the simulator requests a hardware connection using bridge or Web Serial transport and a device is available
- **WHEN** the session start action completes successfully
- **THEN** the device is in USB neighbor mode and the simulator can submit realtime exchanges through the selected transport

#### Scenario: Session end restores normal device mode

- **GIVEN** an active hardware relay session
- **WHEN** the operator disconnects or the simulator tears down the session
- **THEN** the transport session closes, relaying stops, and the device exits USB neighbor mode without requiring a reflash

#### Scenario: Disconnect while audio running

- **GIVEN** audio is running through a hardware slot
- **WHEN** the operator disconnects the hardware session
- **THEN** the simulator stops treating the slot as connected, ceases exchanges for that slot, and follows structural reconfiguration rules for the chain

#### Scenario: Reconnect without browser restart

- **GIVEN** a degraded hardware session after unexpected link loss with chain layout preserved
- **WHEN** the operator completes a successful reconnect action
- **THEN** USB neighbor mode is re-entered and exchanges resume without reloading the browser tab

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

The bridge server, Web Serial transport, and simulator SHALL expose connection status visible to the operator, including whether the selected transport is available, whether a device is reachable, whether an active relay session is in progress, and whether the session is degraded and awaiting reconnect.

#### Scenario: Disconnected bridge indicated

- **GIVEN** the simulator is open, bridge transport is selected, and no bridge server is reachable
- **WHEN** the operator views hardware connection controls
- **THEN** the operator sees that the bridge is unavailable and cannot start a hardware session on that transport

#### Scenario: Device attached without session

- **GIVEN** the selected transport is ready with a device reachable but no active session
- **WHEN** the operator views hardware connection controls
- **THEN** the operator sees the device as available and may start a session when a slot is designated hardware

#### Scenario: Degraded session indicated

- **GIVEN** an active hardware session that lost its transport link
- **WHEN** the operator views the hardware unit card
- **THEN** the operator sees a connection-lost indication and a reconnect action

### Requirement: Delay module PWA integration acceptance

The hardware module bridge PWA integration SHALL include acceptance coverage demonstrating sustained duplex relay of each simulator-supported module kind through the bridge into a running module chain simulator session with at least one neighboring WASM processing unit, using microphone input and speaker output at full fidelity for a documented minimum continuous duration without sustained drop events.

#### Scenario: Module hardware slot in mixed chain

- **GIVEN** firmware for a supported module kind, a running bridge server, and a chain with the hardware slot between WASM neighbors
- **WHEN** the operator enables microphone input and runs audio for the documented minimum soak duration
- **THEN** module-appropriate processing from the hardware slot is audible, physical pots affect processing when the module kind uses controls, and the session completes without sustained drop events

#### Scenario: Neighbor WASM units remain active

- **GIVEN** a mixed chain with WASM units on both sides of the hardware slot
- **WHEN** audio runs through the full chain for the documented soak duration
- **THEN** processing from WASM neighbors remains audible and the hardware slot participates in downstream and upstream wiring throughout

### Requirement: Documented USB round-trip latency delta

The hardware module bridge documentation SHALL state that substituting a hardware slot introduces additional end-to-end latency from USB round-trip, selected transport relay when using the bridge path, and async pipeline buffering compared with an all-WASM chain at the same buffer geometry, SHALL publish a numeric planning budget and tolerance for mixed-chain audition consistent with Phase 3 hardened guidance, and SHALL describe how operators should interpret that delta during extended runs for both bridge and Web Serial transport paths.

#### Scenario: Operator reads latency guidance

- **GIVEN** documentation for PWA hardware slot integration
- **WHEN** an operator reviews known limitations before an extended hardware audition
- **THEN** added USB, transport relay when applicable, and pipeline latency is explicitly named with a published budget and tolerance, including notes for Web Serial versus bridge paths where they differ

### Requirement: Sustained mixed-chain realtime stability

The hardware module bridge PWA integration SHALL sustain full-rate duplex exchanges through the local bridge server for mixed WASM and hardware chains without sustained frame drops for a documented minimum continuous duration while audio is running.

#### Scenario: Ten-minute soak without sustained drops

- **GIVEN** a connected hardware module, running bridge server, and a mixed chain with the hardware slot active and microphone input enabled
- **WHEN** audio runs continuously for at least ten minutes at 22.05 kHz mono fidelity
- **THEN** the session completes with zero sustained drop events as defined in the documented acceptance policy

#### Scenario: Maximum-length mixed chain stability

- **GIVEN** a chain at the simulator maximum unit count with the hardware slot occupying one position among WASM neighbors
- **WHEN** audio runs continuously for the documented minimum soak duration with microphone input enabled
- **THEN** the hardware slot continues exchanging audio without sustained drops and WASM neighbors remain active

### Requirement: Bounded mixed-chain latency budget

The hardware module bridge documentation SHALL publish a documented added end-to-end latency budget for hardware slot substitution in mixed chains, expressed in buffer periods and approximate milliseconds, and mixed-chain acceptance SHALL demonstrate measured added delay within the documented tolerance for a reference topology.

#### Scenario: Operator reads latency budget

- **GIVEN** documentation for hardened hardware slot operation
- **WHEN** an operator reviews expected latency before an extended audition
- **THEN** the documented budget names ring pipeline occupancy, USB and bridge round-trip planning allowance, and standard inter-unit path delay contributions

#### Scenario: Reference topology within budget

- **GIVEN** a reference mixed chain with the hardware slot between two WASM neighbors
- **WHEN** added delay is measured during the documented acceptance procedure
- **THEN** the result falls within the documented tolerance of the published budget

### Requirement: Graceful backpressure recovery in PWA sessions

When transient underrun or overrun occurs during an active PWA hardware relay session, the bridge and simulator integration SHALL recover without halting the overall chain, without requiring a full device reflash, and without sustained audible corruption beyond brief silence substitution on affected hardware-slot paths.

#### Scenario: Transient underrun does not halt chain

- **GIVEN** a running mixed chain with a connected hardware slot
- **WHEN** a transient inbound underrun occurs because a hardware-slot output buffer is not yet available for one period
- **THEN** affected hardware-slot path samples for that period are treated as silence, WASM neighbors continue processing, and the session remains active

#### Scenario: Transient overrun does not halt chain

- **GIVEN** a running mixed chain with a connected hardware slot
- **WHEN** the exchange path experiences a transient outbound overrun
- **THEN** the oldest non-in-flight pending exchange is discarded according to documented policy, scheduling continues, and the chain does not stop

#### Scenario: Recovery after transient events

- **GIVEN** a transient underrun or overrun occurred during playback
- **WHEN** exchange timing returns to nominal for several consecutive buffer periods
- **THEN** hardware-slot audio resumes normal exchange without operator intervention and transient indicators clear

### Requirement: Physical control telemetry relay

During an active PWA hardware slot session, the device and bridge relay SHALL supply the simulator with smoothed primary and secondary control readings derived from on-device physical potentiometers so the hardware unit card MAY display telemetry, while module processing on the device SHALL continue to use physical ADC readings as the authoritative control source.

#### Scenario: Telemetry reflects physical pot movement

- **GIVEN** a connected hardware slot with audio running
- **WHEN** the operator turns a physical potentiometer
- **THEN** telemetry values available to the simulator trend toward the corresponding normalized position within several buffer periods

#### Scenario: Telemetry does not override device processing

- **GIVEN** a connected hardware slot session
- **WHEN** telemetry values are displayed on the hardware unit card
- **THEN** audible processing reflects physical pot positions rather than any values shown only for display in the simulator

#### Scenario: Display is optional

- **GIVEN** a connected hardware slot session
- **WHEN** the simulator chooses not to present telemetry on the unit card
- **THEN** relay and audio behavior remain correct and physical pots remain authoritative

### Requirement: All module kinds hardware acceptance

The hardware module bridge PWA integration SHALL include sustained realtime acceptance coverage for each simulator-supported firmware module kind—passthrough, delay, merger, cutoff, and debug tone—when matching firmware is connected, using 22.05 kHz mono int16 PCM at the universal bridge geometry without compression.

#### Scenario: Passthrough hardware soak

- **GIVEN** passthrough firmware connected and a hardware slot designated for passthrough
- **WHEN** the documented sustained acceptance soak runs in a mixed chain with microphone input
- **THEN** the session completes without sustained drops and passthrough behavior is audibly transparent in the chain

#### Scenario: Delay hardware soak

- **GIVEN** delay firmware connected and a hardware slot designated for delay
- **WHEN** the documented sustained acceptance soak runs with physical pots adjusted during playback
- **THEN** delay effect remains audible, pot changes affect processing, and the session completes without sustained drops

#### Scenario: Merger hardware soak

- **GIVEN** merger firmware connected and a hardware slot designated for merger in a mixed chain that exercises merger dynamics
- **WHEN** the documented sustained acceptance soak runs with microphone input
- **THEN** merge behavior is audible, underrun or overrun events recover without chain halt, and the session completes without sustained drops

#### Scenario: Cutoff hardware soak

- **GIVEN** cutoff firmware connected and a hardware slot designated for cutoff
- **WHEN** the documented sustained acceptance soak runs with physical pots adjusted during playback
- **THEN** filtering response is audible, pot changes affect processing, and the session completes without sustained drops

#### Scenario: Debug tone hardware soak

- **GIVEN** debug tone firmware connected and a hardware slot designated for debug tone
- **WHEN** the documented sustained acceptance soak runs
- **THEN** the expected tone is present in the chain and the session completes without sustained drops

### Requirement: Web Serial direct PWA transport

The hardware module bridge PWA integration SHALL support an optional Web Serial direct transport path that exchanges four-path duplex audio with a connected device without requiring the local bridge server, using the same exchange contract and mono 22.05 kHz sample geometry as the bridge relay path, with no compression.

#### Scenario: Web Serial connect without bridge

- **GIVEN** a Chromium-based browser with Web Serial available and a compatible device attached
- **WHEN** the operator selects Web Serial transport and completes a successful hardware connect for a designated slot
- **THEN** the simulator sustains duplex exchanges at the universal mono geometry directly with the device without a running bridge server

#### Scenario: Web Serial preserves exchange geometry

- **GIVEN** an active Web Serial hardware session
- **WHEN** one complete buffer-period exchange completes
- **THEN** each audio path uses the same 22.05 kHz mono int16 geometry as the bridge relay path

#### Scenario: Web Serial unavailable falls back to bridge guidance

- **GIVEN** a browser without Web Serial support
- **WHEN** the operator views hardware connection controls
- **THEN** Web Serial transport is unavailable and the operator is directed to use the bridge transport path

### Requirement: Transport feature parity

The Web Serial direct transport and the bridge relay transport SHALL provide equivalent hardware-slot audio behavior for all simulator-supported module kinds, including physical control source semantics, pot telemetry relay when enabled, and Phase 3 graceful recovery policies, differing only by how the host opens the device connection.

#### Scenario: Equivalent module-kind relay on Web Serial

- **GIVEN** matching firmware for a supported module kind and an active Web Serial session
- **WHEN** the operator runs a mixed chain with microphone input
- **THEN** module-appropriate processing, physical pot authority, and recovery behavior match what the bridge path provides for the same topology

#### Scenario: Bridge remains required for unsupported browsers

- **GIVEN** a browser that does not expose Web Serial
- **WHEN** the operator connects a hardware slot
- **THEN** sustained relay is available through the bridge transport path at the universal mono geometry

### Requirement: Session reconnect and recovery

The hardware module bridge PWA integration SHALL detect transport or device link loss during an active hardware session, keep WASM neighbors running under documented degraded recovery semantics, and allow the operator to reconnect and resume hardware-slot exchanges without a full browser restart when the device and transport become available again.

#### Scenario: Link loss enters degraded state

- **GIVEN** an active hardware session with audio running through a mixed chain
- **WHEN** the transport link is lost unexpectedly
- **THEN** the session enters a degraded state, hardware-slot paths follow documented silence substitution, WASM neighbors continue processing, and the operator sees that the connection was lost

#### Scenario: Operator reconnect restores active session

- **GIVEN** a hardware session in degraded state with chain layout and hardware designation unchanged
- **WHEN** the operator initiates reconnect and the device responds successfully to USB neighbor entry
- **THEN** duplex exchanges at the universal mono geometry resume, the session returns to active state, and a full browser restart is not required

#### Scenario: Fatal error prevents silent reconnect loop

- **GIVEN** a reconnect attempt encounters module-kind mismatch or a documented fatal device status
- **WHEN** recovery cannot proceed safely
- **THEN** the session moves to a clean disconnected state with a visible actionable error rather than retrying indefinitely

#### Scenario: Clean disconnect from degraded state

- **GIVEN** a hardware session in degraded state
- **WHEN** the operator chooses disconnect
- **THEN** the session tears down orderly, the device exits USB neighbor mode when reachable, and the slot follows disconnected structural rules

### Requirement: Firmware operating mode selection

The hardware module bridge documentation and firmware behavior SHALL distinguish normal I2S chain operating mode from USB neighbor dev operating mode, with normal I2S chain mode as the default on cold boot, USB neighbor mode entered only through documented host session actions, and return to normal I2S mode on orderly session end without requiring a reflash.

#### Scenario: Cold boot defaults to normal I2S chain mode

- **GIVEN** firmware with USB neighbor capability installed
- **WHEN** the device completes a cold boot without a host session start action
- **THEN** the device operates in normal I2S chain mode with live I2S neighbor streaming available

#### Scenario: PWA session enters USB neighbor dev mode

- **GIVEN** the device is in normal I2S chain mode and a host connects via bridge or Web Serial
- **WHEN** the operator starts a hardware session successfully
- **THEN** the device enters USB neighbor dev mode, accepts host-supplied buffer exchanges, and disables live I2S neighbor input for module processing

#### Scenario: Session end returns to normal I2S chain mode

- **GIVEN** the device is in USB neighbor dev mode through an active or degraded PWA session
- **WHEN** the operator disconnects or the session completes orderly teardown
- **THEN** the device exits USB neighbor mode and resumes normal I2S chain behavior without reflash

### Requirement: Operator setup and troubleshooting documentation

The hardware module bridge documentation SHALL provide an operator-facing setup guide and troubleshooting section covering transport selection, firmware mode selection, BOOT and upload workflow, serial port conflicts between upload tools and connected sessions, bridge versus Web Serial choice, reconnect procedure after cable unplug, and known behavioral deltas versus pure simulator chains.

#### Scenario: Operator finds transport guidance

- **GIVEN** an operator preparing a hardware audition
- **WHEN** they read the setup guide
- **THEN** the guide explains when to use Web Serial direct connection versus the bridge server and which browsers support each path

#### Scenario: Operator resolves port conflict

- **GIVEN** a connect attempt fails because the serial port is held by an upload tool or another process
- **WHEN** the operator reads troubleshooting guidance
- **THEN** the documentation explains port exclusivity and the steps to release the port before reconnecting

#### Scenario: Operator understands mode selection

- **GIVEN** an operator unsure whether to run standalone I2S chain or PWA dev mode
- **WHEN** they read mode selection documentation
- **THEN** the guide states when normal I2S chain mode applies versus USB neighbor dev mode and how to switch through session actions

