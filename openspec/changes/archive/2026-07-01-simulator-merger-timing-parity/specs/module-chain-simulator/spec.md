## ADDED Requirements

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

After merger timing parity is implemented, the simulator documentation SHALL list any intentional remaining differences from on-device chain behavior so operators can interpret audition results.

#### Scenario: Post-parity delta list

- **WHEN** an operator reads simulator documentation for known limitations
- **THEN** merger cross-path timing is not listed as an intentional MVP approximation and any remaining gaps (such as startup mute or render-quantum buffering) are explicitly named

## MODIFIED Requirements

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
