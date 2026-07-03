## ADDED Requirements

### Requirement: Sustained mixed-chain realtime stability

The hardware module bridge PWA integration SHALL sustain full-rate duplex exchanges through the local bridge server for mixed WASM and hardware chains without sustained frame drops for a documented minimum continuous duration while audio is running.

#### Scenario: Ten-minute soak without sustained drops

- **GIVEN** a connected hardware module, running bridge server, and a mixed chain with the hardware slot active and microphone input enabled
- **WHEN** audio runs continuously for at least ten minutes at full 44.1 kHz stereo fidelity
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

The hardware module bridge PWA integration SHALL include sustained realtime acceptance coverage for each simulator-supported firmware module kind—passthrough, delay, merger, cutoff, and debug tone—when matching firmware is connected, using full 44.1 kHz stereo int16 PCM without downsampling, compression, or mono fallback.

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

## MODIFIED Requirements

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

The hardware module bridge documentation SHALL state that substituting a hardware slot introduces additional end-to-end latency from USB round-trip, bridge relay, and async pipeline buffering compared with an all-WASM chain at the same buffer geometry, SHALL publish a numeric planning budget and tolerance for mixed-chain audition, and SHALL describe how operators should interpret that delta during extended runs.

#### Scenario: Operator reads latency guidance

- **GIVEN** documentation for hardened PWA hardware slot integration
- **WHEN** an operator reviews known limitations before an extended hardware audition
- **THEN** added USB, bridge, and pipeline latency is explicitly named with a published budget and tolerance, not only as a qualitative difference from pure simulator chains
