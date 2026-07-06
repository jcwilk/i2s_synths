## ADDED Requirements

### Requirement: Single hardware processing unit designation

The module chain simulator SHALL allow the operator to designate at most one processing unit slot as a hardware unit. When designated, that slot SHALL be processed by a connected physical module through the hardware module bridge instead of a local compiled module instance. All other occupied slots SHALL remain WASM processing units.

#### Scenario: Mark slot as hardware

- **GIVEN** a chain with at least one processing unit and no hardware slot is currently designated
- **WHEN** the operator enables hardware mode on a unit card
- **THEN** that unit is marked as the hardware slot and other units remain WASM units

#### Scenario: Only one hardware slot allowed

- **GIVEN** one unit is already designated as hardware
- **WHEN** the operator attempts to designate a second unit as hardware
- **THEN** the second designation is unavailable until the first is cleared or converted back to WASM

### Requirement: Hardware unit operator controls

Each processing unit card SHALL expose hardware connection controls when hardware mode is enabled for that unit, including connect and disconnect actions and visible connection status for the bridge session serving that slot.

#### Scenario: Connect starts hardware session

- **GIVEN** a unit is designated hardware with a module kind matching connected firmware and the bridge is available
- **WHEN** the operator activates connect
- **THEN** a bridge relay session starts and the slot begins participating in chain audio once structural reconfiguration completes

#### Scenario: Disconnect stops hardware participation

- **GIVEN** a connected hardware slot with audio running
- **WHEN** the operator activates disconnect
- **THEN** the slot stops exchanging audio through the bridge and follows structural reconfiguration rules

#### Scenario: WASM controls hidden for hardware slot

- **GIVEN** a unit is designated and connected as hardware
- **WHEN** the operator views that unit card during playback
- **THEN** simulated potentiometer sliders for that unit are not the active control path and the card indicates that physical device controls apply

### Requirement: Async hardware slot processing

The chain scheduler SHALL treat the hardware slot as an asynchronous processing step that does not block the audio rendering thread on USB or bridge I/O. Input and output for the hardware slot SHALL pass through bounded buffering with pipeline delay so WASM neighbors continue to receive and produce buffers on schedule.

#### Scenario: Audio worklet not blocked on USB

- **GIVEN** a running mixed chain with a connected hardware slot
- **WHEN** USB or bridge latency temporarily exceeds one buffer period
- **THEN** the audio rendering thread continues processing WASM neighbors using buffered hardware-slot data rather than stalling until USB completes

#### Scenario: Pipeline delay in chain wiring

- **GIVEN** a hardware slot between two WASM units with audio running
- **WHEN** a non-silent signal propagates through the chain
- **THEN** the hardware slot contributes additional path delay consistent with async exchange and ring-buffer depth in addition to standard inter-unit path delay rules

### Requirement: Mixed WASM and hardware chain wiring

With one hardware slot and one or more WASM processing units, the simulator SHALL route downstream left-to-right and upstream right-to-left feeds using the same adjacency rules as an all-WASM chain, substituting bridge-mediated exchanges for the hardware slot's dual-path processing step while preserving decoupled upstream and downstream timing between neighbors.

#### Scenario: Downstream through hardware slot

- **GIVEN** a chain ordered gateway, WASM unit, hardware delay slot, WASM unit with microphone input enabled
- **WHEN** audio runs continuously
- **THEN** downstream audio reaches the hardware slot from the left neighbor and continues to the right neighbor after hardware processing

#### Scenario: Upstream through hardware slot

- **GIVEN** a chain where upstream energy reaches the hardware slot from the right neighbor
- **WHEN** audio runs continuously
- **THEN** upstream audio from the hardware slot reaches the left neighbor according to chain return-path rules

### Requirement: Hardware slot level metering

While a hardware slot is connected and audio is running, the module chain simulator SHALL display peak-level history for that unit's downstream input and upstream output derived from exchanged audio buffers, advancing continuously without restarting the audio engine except on structural reconfiguration.

#### Scenario: Hardware unit meters advance

- **GIVEN** a connected hardware slot carrying non-silent signal on at least one path
- **WHEN** audio runs for several buffer periods
- **THEN** that unit's downstream-input and upstream-output peak histories advance over time

## MODIFIED Requirements

### Requirement: Per-unit module type selection

Each processing unit SHALL allow the operator to select among all firmware module kinds supported by the simulator build (passthrough, delay, merger, cutoff, and debug tone). Changing a unit's module type SHALL reset that unit's processing state while preserving the operator's current control positions for re-initialization. When audio is running, changing module type SHALL trigger structural reconfiguration and audio rebuild rather than hot-swapping module state while playback continues. For a unit designated as hardware, module type selection SHALL remain available before connection but SHALL require agreement with the connected device firmware module kind before sustained hardware relay begins; changing module type on a connected hardware unit SHALL require disconnecting the hardware session first.

#### Scenario: Full module catalog per unit

- **WHEN** the operator opens a processing unit's module type selector
- **THEN** each supported firmware module kind is available as a distinct option

#### Scenario: Distinct behavior per type

- **WHEN** the operator selects different module types on a WASM unit while audio is running
- **THEN** audible output reflects the selected module's processing characteristics after re-initialization and audio rebuild complete

#### Scenario: State reset on type change

- **WHEN** the operator changes a WASM unit's module type
- **THEN** that unit's internal processing state is re-initialized and level histories for that unit are cleared as part of structural reconfiguration

#### Scenario: Hardware module type locked while connected

- **GIVEN** a hardware slot with an active bridge session
- **WHEN** the operator attempts to change that unit's module type
- **THEN** the change is unavailable until the hardware session is disconnected

### Requirement: Isolated compiled instance per occupied slot

For each occupied processing-unit slot that is not designated as hardware, the simulator SHALL host a separate runtime instance of the compiled module variant matching that slot's selected module type, so per-module static state does not leak across units. The hardware slot SHALL NOT host a local compiled instance for audio processing while hardware mode is active and connected.

#### Scenario: Independent WASM instances

- **WHEN** two WASM processing units are configured with the same module type
- **THEN** each unit maintains independent processing state (for example, separate delay buffers)

#### Scenario: Hardware slot uses device processing

- **GIVEN** a unit designated and connected as hardware
- **WHEN** audio runs through the chain
- **THEN** that slot's audio is processed on the connected device rather than by a local compiled instance

### Requirement: Audio rebuild on structural reconfiguration

The simulator SHALL treat the following operator actions as structural reconfiguration: adding a processing unit, removing a processing unit, reordering processing units, changing a unit's module type, toggling loopback on the rightmost unit, designating or clearing hardware mode on a unit, and connecting or disconnecting a hardware slot bridge session. When audio is running and the operator performs structural reconfiguration, the simulator SHALL stop audio output, clear buffered playback and per-unit level histories, re-establish the audio processing graph for the updated chain, and resume continuous playback without requiring a manual start action. Potentiometer slider adjustments on WASM units SHALL NOT be structural reconfiguration. Physical potentiometer movement on a connected hardware device SHALL NOT be structural reconfiguration.

#### Scenario: Reorder restarts audio automatically

- **GIVEN** audio is running with at least two processing units
- **WHEN** the operator completes a reorder using the drag handle
- **THEN** audio briefly stops, level histories clear, and playback resumes on the updated chain order

#### Scenario: Module type change restarts audio

- **GIVEN** audio is running on a WASM processing unit
- **WHEN** the operator selects a different module type for that unit
- **THEN** audio briefly stops, that unit's level histories clear, and playback resumes with the new module type active

#### Scenario: Delete restarts audio

- **GIVEN** audio is running with at least two processing units
- **WHEN** the operator removes a unit
- **THEN** audio briefly stops, removed unit metering disappears, and playback resumes on the remaining units

#### Scenario: Pot adjustment stays live on WASM units

- **GIVEN** audio is running on a WASM processing unit
- **WHEN** the operator moves a potentiometer slider without any structural reconfiguration
- **THEN** audio continues without stop or rebuild and output reflects the new control position within a small number of buffer periods

#### Scenario: Hardware connect restarts audio

- **GIVEN** audio is running on a chain with a hardware-designated unit that is not yet connected
- **WHEN** the operator completes a successful hardware connect
- **THEN** audio briefly stops, level histories clear, and playback resumes with the hardware slot active in the chain

#### Scenario: Structural edit while stopped

- **GIVEN** audio is not running
- **WHEN** the operator adds, removes, reorders units, changes module type, toggles loopback, or changes hardware designation or connection state
- **THEN** the chain layout and configuration update without starting audio automatically

### Requirement: Documented remaining simulator deltas

After merger timing parity is implemented, the simulator documentation SHALL list any intentional remaining differences from on-device chain behavior so operators can interpret audition results. When hardware slot integration is available, the documentation SHALL additionally list hardware-specific deltas including USB and bridge round-trip latency and the single-hardware-slot limit.

#### Scenario: Post-parity delta list

- **WHEN** an operator reads simulator documentation for known limitations
- **THEN** merger cross-path timing is not listed as an intentional MVP approximation and any remaining gaps (such as startup mute or render-quantum buffering) are explicitly named

#### Scenario: Hardware integration deltas named

- **WHEN** an operator reads simulator documentation after hardware slot support is delivered
- **THEN** added USB latency and the single hardware slot limit are explicitly listed among known differences from an all-hardware or all-WASM reference
