## ADDED Requirements

### Requirement: Gateway input and output card

The module chain simulator SHALL present a fixed gateway at the left of the chain layout. The gateway SHALL expose microphone enable/disable for chain input and SHALL play the gateway upstream path output to the host speakers. The gateway SHALL NOT host selectable effect processing in this phase.

#### Scenario: Gateway owns microphone toggle

- **WHEN** the operator views the chain layout
- **THEN** a gateway card at the left provides the microphone input toggle independent of any processing unit cards

#### Scenario: Gateway does not offer module selection

- **WHEN** the operator inspects the gateway card
- **THEN** no module type selector is available on the gateway

#### Scenario: Speaker output from gateway upstream path

- **WHEN** audio is running and the chain carries non-silent signal to the gateway upstream output
- **THEN** the operator hears that signal on the default playback device

### Requirement: Appendable processing units

The simulator SHALL allow the operator to add processing units to the right of the gateway. Each added unit SHALL appear as its own card with module type selection and dual potentiometer controls. The simulator SHALL enforce a configurable maximum number of processing units beyond the gateway.

#### Scenario: Add unit extends the chain

- **WHEN** the operator activates the add-unit control and the chain is below the maximum length
- **THEN** a new processing unit card appears to the right of the existing rightmost unit

#### Scenario: Maximum length enforced

- **WHEN** the chain already contains the maximum allowed number of processing units
- **THEN** the add-unit control is unavailable and no further units can be added

### Requirement: Per-unit module type selection

Each processing unit SHALL allow the operator to select among all firmware module kinds supported by the simulator build (passthrough, delay, merger, cutoff, and debug tone). Changing a unit's module type SHALL reset that unit's processing state while preserving the operator's current control positions for re-initialization.

#### Scenario: Full module catalog per unit

- **WHEN** the operator opens a processing unit's module type selector
- **THEN** each supported firmware module kind is available as a distinct option

#### Scenario: Distinct behavior per type

- **WHEN** the operator selects different module types on a unit while audio is running
- **THEN** audible output reflects the selected module's processing characteristics after re-initialization completes

#### Scenario: State reset on type change

- **WHEN** the operator changes a unit's module type
- **THEN** that unit's internal processing state is re-initialized without restarting the entire audio engine

### Requirement: Isolated compiled instance per occupied slot

For each occupied processing-unit slot, the simulator SHALL host a separate runtime instance of the compiled module variant matching that slot's selected module type, so per-module static state does not leak across units.

#### Scenario: Independent instances

- **WHEN** two processing units are configured with the same module type
- **THEN** each unit maintains independent processing state (for example, separate delay buffers)

### Requirement: Multi-unit chain audio wiring

With one gateway and one or more processing units, the simulator SHALL route audio between slots each buffer period using downstream left-to-right feeds and upstream right-to-left feeds. The gateway downstream input SHALL be live microphone audio when microphone input is enabled and silence otherwise. Each unit's downstream input SHALL be the prior slot's downstream output. Each interior unit's upstream input SHALL be the next unit's upstream output. Speaker output SHALL be the gateway upstream output.

#### Scenario: Mic feeds gateway downstream input

- **WHEN** microphone input is enabled and the chain is running
- **THEN** captured microphone audio enters at the gateway downstream input

#### Scenario: Downstream propagation

- **WHEN** a non-silent signal is present at a unit's downstream output
- **THEN** the next unit to the right receives that signal as its downstream input on the following buffer period

#### Scenario: Upstream return path

- **WHEN** a unit to the right produces upstream output
- **THEN** the unit to its left receives that signal as upstream input on the same buffer period according to the chain wiring rules

### Requirement: Rightmost loopback control

The simulator SHALL expose a loopback control on the rightmost processing unit only. When enabled, that unit's upstream input SHALL be fed from its own downstream output. When disabled, that unit's upstream input SHALL be silence.

#### Scenario: Loopback off by default

- **WHEN** a new processing unit becomes the rightmost unit
- **THEN** loopback is disabled for that unit

#### Scenario: Loopback enabled

- **WHEN** the operator enables loopback on the rightmost unit and audio is running
- **THEN** that unit's upstream path processes audio derived from its own downstream output

#### Scenario: Loopback only on rightmost unit

- **WHEN** multiple processing units are present
- **THEN** only the rightmost unit card offers a loopback control

## MODIFIED Requirements

### Requirement: Compiled module processing

The simulator SHALL execute processing logic compiled from the same firmware module sources used on device, not a reimplemented approximation in the host language. The simulator build SHALL include compiled variants for passthrough, delay, merger, cutoff, and debug tone module kinds.

#### Scenario: All supported module variants available

- **WHEN** the operator configures processing units across the chain
- **THEN** each supported module kind can be selected and produces behavior consistent with that module's firmware role

#### Scenario: At least one passthrough-like and one effect variant

- **WHEN** the operator selects passthrough and an effect module (for example delay) on different units
- **THEN** each produces audibly distinct behavior on its respective downstream and upstream paths

### Requirement: Optional microphone input

The simulator SHALL allow the operator to enable or disable feeding captured microphone audio into the gateway downstream input. When disabled, gateway downstream input SHALL be silence.

#### Scenario: Mic enabled

- **WHEN** microphone input is enabled and capture permission is granted
- **THEN** live microphone audio enters the chain at the gateway downstream input and is audible at the speakers after traversing configured units

#### Scenario: Mic disabled

- **WHEN** microphone input is disabled
- **THEN** gateway downstream input is silence

#### Scenario: Mic unavailable

- **WHEN** microphone input is enabled but capture is unavailable or denied
- **THEN** the simulator continues without crashing and gateway downstream input is treated as silence

### Requirement: Dual-path level metering

While audio is running, the module chain simulator SHALL display scrolling peak-level history for each processing unit's downstream input and upstream output. Each history SHALL advance continuously without restarting the audio engine.

#### Scenario: Meters run during playback

- **WHEN** audio is running and at least one path on a processing unit carries non-silent signal
- **THEN** peak-level history for that path on that unit advances over time and remains visible to the operator

#### Scenario: Independent in and out channels per unit

- **WHEN** audio is running with multiple processing units visible
- **THEN** each unit presents distinguishable downstream-input and upstream-output peak channels

#### Scenario: Buffer-aligned peak samples

- **WHEN** the simulator completes processing for one firmware buffer period for a unit
- **THEN** at most one new peak sample per path is appended to that unit's level history for that period

## REMOVED Requirements

### Requirement: Phase 0 single virtual unit

**Reason:** Phase 1 chain MVP supersedes the intentional single-unit topology; multi-unit layout with gateway is the supported model.

**Migration:** Use the gateway plus appendable processing units requirements. Existing single-unit spike behavior is not required after this change archives.
