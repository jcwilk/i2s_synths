## ADDED Requirements

### Requirement: Removable processing units

The simulator SHALL allow the operator to remove individual processing units from the chain. After removal, remaining units SHALL keep their module types, control positions, and processing state associated with each unit. The chain SHALL rewire downstream and upstream paths according to the new unit order. The rightmost loopback control SHALL apply only to the new rightmost unit after removal.

#### Scenario: Delete middle unit

- **GIVEN** a chain with at least three processing units and audio is stopped
- **WHEN** the operator removes a unit that is neither the gateway nor the only remaining unit
- **THEN** that unit disappears from the layout and audio routes through the remaining units in their prior relative order

#### Scenario: Cannot delete sole unit while audio runs

- **GIVEN** exactly one processing unit and audio is running
- **WHEN** the operator attempts to remove that unit
- **THEN** removal is unavailable and audio continues on the existing chain

#### Scenario: Delete restores add capacity

- **GIVEN** the chain already contains the maximum allowed number of processing units
- **WHEN** the operator removes one unit
- **THEN** the add-unit control becomes available again

### Requirement: Reorderable processing units

The simulator SHALL allow the operator to change processing unit order by dragging one unit card to a new position among other unit cards. Reordering SHALL NOT move the gateway card. Each unit card SHALL expose a dedicated drag handle control that is the only part of the card used to initiate a reorder gesture.

#### Scenario: Reorder changes chain wiring

- **GIVEN** at least two processing units with audibly distinct module types on adjacent units
- **WHEN** the operator moves the rightmost unit to the left of its neighbor using the drag handle
- **THEN** downstream and upstream routing follows the new left-to-right order on subsequent buffer periods

#### Scenario: Handle initiates drag only

- **GIVEN** a processing unit card is visible on a touch-capable device
- **WHEN** the operator drags starting from the dedicated drag handle
- **THEN** the unit moves for reordering rather than the gesture being treated solely as page scrolling

#### Scenario: Non-handle interaction does not reorder

- **GIVEN** a processing unit card is visible
- **WHEN** the operator drags starting from a pot slider or module type selector
- **THEN** the unit order does not change

### Requirement: Audio rebuild on structural reconfiguration

The simulator SHALL treat the following operator actions as structural reconfiguration: adding a processing unit, removing a processing unit, reordering processing units, changing a unit's module type, and toggling loopback on the rightmost unit. When audio is running and the operator performs structural reconfiguration, the simulator SHALL stop audio output, clear buffered playback and per-unit level histories, re-establish the audio processing graph for the updated chain, and resume continuous playback without requiring a manual start action. Potentiometer slider adjustments SHALL NOT be structural reconfiguration.

#### Scenario: Reorder restarts audio automatically

- **GIVEN** audio is running with at least two processing units
- **WHEN** the operator completes a reorder using the drag handle
- **THEN** audio briefly stops, level histories clear, and playback resumes on the updated chain order

#### Scenario: Module type change restarts audio

- **GIVEN** audio is running on a processing unit
- **WHEN** the operator selects a different module type for that unit
- **THEN** audio briefly stops, that unit's level histories clear, and playback resumes with the new module type active

#### Scenario: Delete restarts audio

- **GIVEN** audio is running with at least two processing units
- **WHEN** the operator removes a unit
- **THEN** audio briefly stops, removed unit metering disappears, and playback resumes on the remaining units

#### Scenario: Pot adjustment stays live

- **GIVEN** audio is running on a processing unit
- **WHEN** the operator moves a potentiometer slider without any structural reconfiguration
- **THEN** audio continues without stop or rebuild and output reflects the new control position within a small number of buffer periods

#### Scenario: Structural edit while stopped

- **GIVEN** audio is not running
- **WHEN** the operator adds, removes, reorders units, changes module type, or toggles loopback
- **THEN** the chain layout and configuration update without starting audio automatically

## MODIFIED Requirements

### Requirement: Real-time interactive operation

The simulator SHALL process audio continuously in real time while running. Potentiometer control changes and microphone enable/disable SHALL take effect without restarting the audio engine. Structural reconfiguration of the processing chain SHALL follow the audio rebuild requirement instead of applying silently while audio continues.

#### Scenario: Live pot adjustment

- **WHEN** the operator moves a simulated pot during playback without structural reconfiguration
- **THEN** processing continues and output reflects the new settings within a small number of buffer periods

#### Scenario: Live microphone toggle

- **WHEN** the operator toggles microphone input during playback
- **THEN** processing continues and gateway downstream input follows the new microphone setting without a full audio teardown

### Requirement: Per-unit module type selection

Each processing unit SHALL allow the operator to select among all firmware module kinds supported by the simulator build (passthrough, delay, merger, cutoff, and debug tone). Changing a unit's module type SHALL reset that unit's processing state while preserving the operator's current control positions for re-initialization. When audio is running, changing module type SHALL trigger structural reconfiguration and audio rebuild rather than hot-swapping module state while playback continues.

#### Scenario: Full module catalog per unit

- **WHEN** the operator opens a processing unit's module type selector
- **THEN** each supported firmware module kind is available as a distinct option

#### Scenario: Distinct behavior per type

- **WHEN** the operator selects different module types on a unit while audio is running
- **THEN** audible output reflects the selected module's processing characteristics after re-initialization and audio rebuild complete

#### Scenario: State reset on type change

- **WHEN** the operator changes a unit's module type
- **THEN** that unit's internal processing state is re-initialized and level histories for that unit are cleared as part of structural reconfiguration
