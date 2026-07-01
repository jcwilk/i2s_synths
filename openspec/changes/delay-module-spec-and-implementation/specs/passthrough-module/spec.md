## ADDED Requirements

### Requirement: Identity passthrough on both paths

The passthrough module SHALL copy each input sample unchanged to the corresponding output sample on both upstream and downstream paths.

#### Scenario: Downstream copy

- **WHEN** downstream processing receives a valid input and output buffer
- **THEN** the output buffer matches the input buffer sample-for-sample

#### Scenario: Upstream copy

- **WHEN** upstream processing receives a valid input and output buffer
- **THEN** the output buffer matches the input buffer sample-for-sample

### Requirement: No control-dependent behavior

The passthrough module SHALL NOT alter audio based on potentiometer position.

#### Scenario: Knob position irrelevant

- **WHEN** primary or secondary control values change
- **THEN** passthrough output remains identical to input for the same audio buffer
