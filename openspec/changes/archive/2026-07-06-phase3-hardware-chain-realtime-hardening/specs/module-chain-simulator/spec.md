## ADDED Requirements

### Requirement: Hardware stress acceptance scenarios

The module chain simulator SHALL define and satisfy a documented stress acceptance matrix for connected hardware slots that includes maximum chain length with the hardware unit among neighbors, merger module configurations with loopback enabled on the rightmost unit and the hardware slot in middle or end positions, and microphone input enabled for representative scenarios.

#### Scenario: Maximum chain with hardware slot

- **GIVEN** a chain at the maximum allowed processing unit count with one hardware slot connected
- **WHEN** the documented stress soak runs with microphone input enabled
- **THEN** audio continues through the full chain for the minimum duration without sustained hardware drop events

#### Scenario: Merger loopback with hardware in middle

- **GIVEN** a chain where a WASM merger unit is rightmost with loopback enabled and the hardware slot sits between WASM neighbors
- **WHEN** the documented stress soak runs with microphone input enabled
- **THEN** merger and hardware paths both participate audibly and the session completes without sustained drops or chain halt

#### Scenario: Merger loopback with hardware at end

- **GIVEN** a chain where the hardware slot is the rightmost processing unit and an interior WASM unit runs merger with loopback enabled on the rightmost position
- **WHEN** the documented stress soak runs with microphone input enabled
- **THEN** loopback and hardware async delay interact without sustained drops and recovery behavior matches the hardware-module-bridge graceful recovery requirements

### Requirement: Extended hardware soak duration

Connected hardware slot acceptance in the module chain simulator SHALL require a documented minimum continuous run duration of at least ten minutes per stress or module-kind scenario unless a shorter warm-up window is explicitly excluded from drop counting in the acceptance procedure.

#### Scenario: Ten-minute hardware audition

- **GIVEN** a connected hardware slot in a mixed chain configured for acceptance
- **WHEN** the operator or harness runs the documented soak procedure
- **THEN** the run duration meets the published minimum and pass or fail is recorded against sustained drop criteria for the full counted interval

### Requirement: Documented mixed-chain latency budget

The module chain simulator documentation SHALL publish the expected added latency from hardware slot substitution in mixed chains, consistent with the hardware-module-bridge latency budget, so operators can compare all-WASM and mixed-chain auditions during extended runs.

#### Scenario: Simulator docs name hardware latency budget

- **GIVEN** hardened hardware slot support is documented for the simulator
- **WHEN** an operator reads simulator known-limitations guidance
- **THEN** the published mixed-chain latency budget and tolerance are listed alongside existing hardware integration deltas such as single-slot limit and USB relay path

## MODIFIED Requirements

### Requirement: Async hardware slot processing

The chain scheduler SHALL treat the hardware slot as an asynchronous processing step that does not block the audio rendering thread on USB or bridge I/O. Input and output for the hardware slot SHALL pass through bounded buffering with pipeline delay so WASM neighbors continue to receive and produce buffers on schedule. When transient underrun or overrun occurs, the scheduler SHALL apply documented recovery semantics—silence substitution on affected hardware paths, continued WASM neighbor cadence, and no chain halt—until exchange timing stabilizes.

#### Scenario: Audio worklet not blocked on USB

- **GIVEN** a running mixed chain with a connected hardware slot
- **WHEN** USB or bridge latency temporarily exceeds one buffer period
- **THEN** the audio rendering thread continues processing WASM neighbors using buffered hardware-slot data rather than stalling until USB completes

#### Scenario: Pipeline delay in chain wiring

- **GIVEN** a hardware slot between two WASM units with audio running
- **WHEN** a non-silent signal propagates through the chain
- **THEN** the hardware slot contributes additional path delay consistent with async exchange and ring-buffer depth in addition to standard inter-unit path delay rules

#### Scenario: Transient hardware underrun recovers

- **GIVEN** a running mixed chain with a connected hardware slot
- **WHEN** a transient hardware-slot underrun occurs for one or more buffer periods
- **THEN** affected hardware-path samples are silence for those periods, WASM neighbors continue on schedule, and normal hardware-slot exchange resumes without structural reconfiguration once buffers refill

### Requirement: Hardware slot level metering

While a hardware slot is connected and audio is running, the module chain simulator SHALL display peak-level history for that unit's downstream input and upstream output derived from exchanged audio buffers, advancing continuously without restarting the audio engine except on structural reconfiguration. Hardware-slot metering SHALL use the same presentation rules as WASM units, including logarithmic amplitude scaling, at least two distinct peak-history time windows, and buffer-aligned peak sampling.

#### Scenario: Hardware unit meters advance

- **GIVEN** a connected hardware slot carrying non-silent signal on at least one path
- **WHEN** audio runs for several buffer periods
- **THEN** that unit's downstream-input and upstream-output peak histories advance over time

#### Scenario: Hardware metering matches WASM presentation

- **GIVEN** a connected hardware slot and at least one WASM unit both carrying non-silent signal
- **WHEN** the operator views level metering during playback
- **THEN** the hardware unit exposes the same short and long peak-history windows and log-scaled presentation behavior as WASM units

#### Scenario: Buffer-aligned hardware peaks

- **GIVEN** a connected hardware slot processing exchanged audio
- **WHEN** one firmware buffer period of hardware-slot audio completes
- **THEN** at most one new peak sample per path is appended to that unit's level history for that period

### Requirement: Documented remaining simulator deltas

After merger timing parity is implemented, the simulator documentation SHALL list any intentional remaining differences from on-device chain behavior so operators can interpret audition results. When hardware slot integration is available, the documentation SHALL additionally list hardware-specific deltas including USB and bridge round-trip latency, the published mixed-chain latency budget and tolerance, async pipeline buffering, and the single-hardware-slot limit.

#### Scenario: Post-parity delta list

- **WHEN** an operator reads simulator documentation for known limitations
- **THEN** merger cross-path timing is not listed as an intentional MVP approximation and any remaining gaps (such as startup mute or render-quantum buffering) are explicitly named

#### Scenario: Hardware integration deltas named

- **WHEN** an operator reads simulator documentation after hardened hardware slot support is delivered
- **THEN** added USB latency, the published mixed-chain latency budget, async pipeline buffering, and the single hardware slot limit are explicitly listed among known differences from an all-hardware or all-WASM reference
