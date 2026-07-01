## ADDED Requirements

### Requirement: Single active module per firmware build

The firmware SHALL compile with exactly one audio processing module active. Module selection SHALL be a build-time choice, not a runtime switch among multiple loaded modules.

#### Scenario: One module linked per build

- **WHEN** the sketch is compiled with a given active module identifier
- **THEN** only that module's setup and loop handlers are invoked for audio processing

### Requirement: Dual-path processing contract

Each audio module SHALL expose a one-time setup hook and two per-buffer processing hooks: one for the upstream I2S path and one for the downstream I2S path. Each hook SHALL receive the current interleaved stereo sample buffer length and SHALL write exactly that many 16-bit samples to its output buffer when output is required.

#### Scenario: Buffer length preserved

- **WHEN** the streaming layer invokes a module loop handler with a positive sample count
- **THEN** the handler produces an output buffer of the same sample count without partial writes

### Requirement: Shared smoothed control surface

Module loop handlers SHALL receive the current smoothed dual-potentiometer state for the buffer period. Modules MAY map primary and secondary controls to behavior; modules that ignore controls SHALL still accept the state without requiring a different call signature.

#### Scenario: Controls available on downstream path

- **WHEN** a module implements downstream processing that responds to user knobs
- **THEN** it reads normalized primary and secondary values from the passed control state rather than performing raw ADC reads inside the audio loop

### Requirement: Path semantics

Upstream processing SHALL transform samples traveling toward the gateway. Downstream processing SHALL transform samples traveling away from the gateway. A module MAY treat the two paths differently or pass one path through unchanged.

#### Scenario: Asymmetric path behavior allowed

- **WHEN** a module applies effects only on the downstream path
- **THEN** the upstream path MAY copy input to output unchanged while downstream applies the effect
