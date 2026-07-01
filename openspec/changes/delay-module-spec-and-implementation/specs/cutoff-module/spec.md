## ADDED Requirements

### Requirement: Upstream identity

The cutoff module SHALL pass the upstream path through unchanged.

#### Scenario: Upstream unchanged

- **WHEN** upstream processing runs with valid buffers
- **THEN** output equals input for that path

### Requirement: Downstream tone filter

On the downstream path, the module SHALL apply a state-variable-filter-based tone shaper whose cutoff frequency is determined by the primary control and whose low-pass versus high-pass emphasis is determined by the secondary control.

#### Scenario: Primary maps to cutoff range

- **WHEN** primary control is at minimum
- **THEN** effective cutoff is at the module's minimum mapped frequency

- **WHEN** primary control is at maximum
- **THEN** effective cutoff is at the module's maximum mapped frequency

#### Scenario: Secondary blends LP and HP

- **WHEN** secondary control is centered
- **THEN** filtering intensity is minimal relative to dry signal

- **WHEN** secondary control moves toward either extreme
- **THEN** output emphasizes low-pass or high-pass character respectively according to the control direction

### Requirement: Saturated 16-bit output

Filtered downstream output SHALL be rounded and clamped to 16-bit PCM range.

#### Scenario: Peaks do not wrap

- **WHEN** filter resonance produces large intermediate values
- **THEN** emitted samples remain within representable 16-bit bounds
