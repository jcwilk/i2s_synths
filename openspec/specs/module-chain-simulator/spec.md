# module-chain-simulator Specification

## Purpose
The module chain simulator is a Phase 0 browser host for a single virtual audio processing unit. It runs module logic compiled from the same firmware sources as on-device builds (WebAssembly), processes stereo 44.1 kHz audio in real time through upstream and downstream paths, exposes simulated potentiometer controls whose smoothed values mirror firmware polling behavior, and displays scrolling peak-level metering for downstream input and upstream output so operators can observe dynamics while interactively adjusting effects.
## Requirements
### Requirement: Firmware-equivalent audio format

The module chain simulator SHALL process audio using 44.1 kHz sample rate, stereo interleaved 16-bit signed PCM, and a fixed per-call buffer size matching the firmware streaming buffer length (512 int16 samples per path invocation).

#### Scenario: Buffer contract matches firmware

- **WHEN** the simulator invokes module processing for one buffer period
- **THEN** each path receives and produces exactly the firmware buffer sample count at the firmware sample rate

### Requirement: Compiled module processing

The simulator SHALL execute processing logic compiled from the same firmware module sources used on device, not a reimplemented approximation in the host language. The simulator build SHALL include compiled variants for passthrough, delay, merger, cutoff, and debug tone module kinds.

#### Scenario: All supported module variants available

- **WHEN** the operator configures processing units across the chain
- **THEN** each supported module kind can be selected and produces behavior consistent with that module's firmware role

#### Scenario: At least one passthrough-like and one effect variant

- **WHEN** the operator selects passthrough and an effect module (for example delay) on different units
- **THEN** each produces audibly distinct behavior on its respective downstream and upstream paths

### Requirement: Dual-path invocation order

For each audio buffer period, the simulator SHALL invoke upstream processing before downstream processing on the virtual unit, passing the same simulated control state to both calls.

#### Scenario: Path order preserved

- **WHEN** one buffer period is processed
- **THEN** upstream processing completes before downstream processing begins for that period

### Requirement: Simulated potentiometer controls

The simulator SHALL expose primary and secondary controls in the normalized zero-to-one range. Control position SHALL represent the simulated raw pot reading. The simulator SHALL derive smoothed pot state using time-scaled exponential smoothing equivalent to firmware pot polling, and SHALL supply that smoothed state to module processing without requiring physical ADC input. The numeric reading shown beside each control SHALL reflect the smoothed value passed to modules, not the instantaneous control position.

#### Scenario: Controls affect downstream behavior

- **WHEN** the operator adjusts a simulated pot while an effect module that responds to controls is active
- **THEN** the audible output changes in a way consistent with that module's downstream control mapping

#### Scenario: Smoothed value eases toward control position

- **WHEN** the operator moves a control to a new position and holds it there while audio is running
- **THEN** the smoothed value supplied to modules approaches the new position over multiple poll intervals rather than jumping instantaneously

#### Scenario: Display reflects smoothed state

- **WHEN** a control has been held at a steady position long enough for smoothing to settle
- **THEN** the numeric reading beside that control matches the smoothed value used for module processing within normal display rounding

#### Scenario: Firmware-aligned poll cadence

- **WHEN** the simulator updates smoothed pot state during continuous playback
- **THEN** poll timing is aligned to approximately one firmware audio buffer period at the simulator sample rate

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

### Requirement: Speaker output from upstream path

The simulator SHALL play the virtual unit's upstream processing output to the host device speakers (or default audio output).

#### Scenario: Audible round trip

- **WHEN** audio is running and a non-silent input or self-generating module is active
- **THEN** the operator hears continuous output on the default playback device

### Requirement: Real-time interactive operation

The simulator SHALL process audio continuously in real time while running. Potentiometer control changes and microphone enable/disable SHALL take effect without restarting the audio engine. Structural reconfiguration of the processing chain SHALL follow the audio rebuild requirement instead of applying silently while audio continues.

#### Scenario: Live pot adjustment

- **WHEN** the operator moves a simulated pot during playback without structural reconfiguration
- **THEN** processing continues and output reflects the new settings within a small number of buffer periods

#### Scenario: Live microphone toggle

- **WHEN** the operator toggles microphone input during playback
- **THEN** processing continues and gateway downstream input follows the new microphone setting without a full audio teardown

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

### Requirement: Multiple meter time windows

The simulator SHALL offer at least two distinct peak-history time spans so the operator can view recent dynamics and longer-term level trends.

#### Scenario: Short and long windows available

- **WHEN** the operator views the level metering area during playback
- **THEN** at least one short window (on the order of a few seconds) and one longer window (tens of seconds) are available simultaneously

### Requirement: Log-scaled level presentation

Level metering SHALL use a logarithmic amplitude presentation with an adaptive vertical range based on peaks visible in the current window, so that quiet passages and louder peaks remain interpretable on the same view.

#### Scenario: Quiet and loud material share a view

- **WHEN** level history contains both very quiet and substantially louder peaks within the visible window
- **THEN** the display remains readable without clipping louder peaks to the top edge solely because quieter material is present

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

### Requirement: Multi-unit chain audio wiring

With one gateway and one or more processing units, the simulator SHALL route audio between slots each buffer period using downstream left-to-right feeds and upstream right-to-left feeds with path-appropriate delay between adjacent units. The gateway downstream input SHALL be live microphone audio when microphone input is enabled and silence otherwise. Each unit's downstream input SHALL be the prior slot's downstream output delayed by at least one firmware buffer period. Each interior unit's upstream input SHALL be the next unit's upstream output delayed by at least one firmware buffer period. Speaker output SHALL be the gateway upstream output.

#### Scenario: Mic feeds gateway downstream input

- **WHEN** microphone input is enabled and the chain is running
- **THEN** captured microphone audio enters at the gateway downstream input

#### Scenario: Downstream propagation

- **WHEN** a non-silent signal is present at a unit's downstream output
- **THEN** the next unit to the right receives that signal as its downstream input after the configured path delay

#### Scenario: Upstream return path

- **WHEN** a unit to the right produces upstream output
- **THEN** the unit to its left receives that signal as upstream input after the configured path delay according to the chain wiring rules

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

### Requirement: Decoupled dual-path input timing

For each processing unit, the simulator SHALL supply upstream-path and downstream-path inputs on independent timing schedules that match the physical module chain model: downstream inputs propagate left-to-right with at least one firmware buffer period of delay between adjacent units, and upstream inputs propagate right-to-left with equivalent path delay, rather than resolving both paths from the same instantaneous buffer-period snapshot.

#### Scenario: Adjacent units do not share same-period upstream and downstream feeds

- **GIVEN** at least two processing units in a running chain
- **WHEN** a non-silent signal appears at the gateway downstream input
- **THEN** the downstream input seen by a unit to the right is delayed relative to the upstream input that unit receives from its neighbor on the return path according to the chain timing rules

#### Scenario: Merger ring fill reflects path decoupling

- **GIVEN** a chain where one unit is configured as merger and upstream audio arrives on the return path while downstream audio arrives from the left
- **WHEN** audio runs continuously for several buffer periods
- **THEN** the merger unit's delayed upstream contribution on the downstream path reflects ring-buffer latency rather than immediate same-period upstream capture

### Requirement: Merger chain behavior parity

When a processing unit runs the merger module kind, the simulator SHALL produce merge dynamics consistent with the merger module behavior contract: upstream capture with passthrough, downstream mix of current downstream input with delayed upstream samples, gentle peak limiting on merged output, underrun silence substitution with stabilization, overrun drop-oldest handling, and optional downstream-to-upstream forwarding when enabled in the compiled build.

#### Scenario: Delayed upstream blend in chain

- **GIVEN** a running chain with merger configured and non-silent upstream energy reaching that unit
- **WHEN** the operator sets primary contribution low and secondary contribution high
- **THEN** downstream output is audibly dominated by delayed upstream content after the capture buffer fills

#### Scenario: Forward path in chain

- **GIVEN** a running chain with merger configured and downstream-to-upstream forwarding enabled in the build
- **WHEN** non-silent downstream input reaches the merger unit
- **THEN** scaled downstream energy reappears on that unit's upstream output after buffering and limiting consistent with downstream processing

#### Scenario: Underrun does not halt chain

- **GIVEN** a merger unit whose capture buffer lacks sufficient delayed upstream samples for a downstream buffer period
- **WHEN** downstream processing runs
- **THEN** missing samples are treated as silence, processing continues, and the operator receives a visible underrun indication on that unit

#### Scenario: Overrun does not halt chain

- **GIVEN** a merger unit whose capture buffer would exceed capacity
- **WHEN** upstream processing enqueues additional samples
- **THEN** oldest samples are discarded according to ring policy, processing continues, and the operator receives a visible overrun indication on that unit

### Requirement: Documented remaining simulator deltas

After merger timing parity is implemented, the simulator documentation SHALL list any intentional remaining differences from on-device chain behavior so operators can interpret audition results. When hardware slot integration is available, the documentation SHALL additionally list hardware-specific deltas including USB and bridge round-trip latency, the published mixed-chain latency budget and tolerance, async pipeline buffering, and the single-hardware-slot limit.

#### Scenario: Post-parity delta list

- **WHEN** an operator reads simulator documentation for known limitations
- **THEN** merger cross-path timing is not listed as an intentional MVP approximation and any remaining gaps (such as startup mute or render-quantum buffering) are explicitly named

#### Scenario: Hardware integration deltas named

- **WHEN** an operator reads simulator documentation after hardened hardware slot support is delivered
- **THEN** added USB latency, the published mixed-chain latency budget, async pipeline buffering, and the single hardware slot limit are explicitly listed among known differences from an all-hardware or all-WASM reference

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

