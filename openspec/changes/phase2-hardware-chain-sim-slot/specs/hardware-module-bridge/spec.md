## ADDED Requirements

### Requirement: Local bridge server relay

The hardware module bridge SHALL provide a local bridge server process that accepts duplex binary exchanges from the module chain simulator over a WebSocket connection and relays them to a connected ESP32 module over USB using the Phase 1 realtime four-path exchange contract, without altering audio sample geometry or introducing downsampling, compression, or mono fallback.

#### Scenario: WebSocket client connects to bridge

- **GIVEN** the bridge server is running and a device is connected over USB
- **WHEN** the module chain simulator opens a WebSocket session to the bridge server
- **THEN** the bridge accepts the session and is ready to relay complete buffer-period exchanges in both directions

#### Scenario: Relay preserves full-rate stereo paths

- **GIVEN** an active WebSocket session and USB neighbor mode on the device
- **WHEN** the simulator submits one complete exchange for a buffer period
- **THEN** the device receives downstream and upstream inputs at full 44.1 kHz stereo int16 fidelity and returns downstream and upstream outputs at the same geometry

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
