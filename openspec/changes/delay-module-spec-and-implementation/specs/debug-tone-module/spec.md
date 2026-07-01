## ADDED Requirements

### Requirement: Independent path tone generation

The debug-tone module SHALL maintain separate tone-generation state for upstream and downstream paths. Each path SHALL be configurable at build time to either generate a test tone or pass audio through unchanged.

#### Scenario: Downstream tone enabled

- **WHEN** downstream tone generation is enabled for the build
- **THEN** downstream output is a stereo test waveform regardless of downstream input content

#### Scenario: Path passthrough when generation disabled

- **WHEN** tone generation is disabled for a path
- **THEN** that path copies input to output when input is present

### Requirement: Frequency mapping

When not overridden by a fixed-frequency build option, primary control SHALL map logarithmically across the module's configured minimum and maximum test frequencies.

#### Scenario: Frequency sweep

- **WHEN** primary control sweeps from minimum to maximum
- **THEN** output fundamental frequency increases monotonically across the configured range

### Requirement: Waveshape blending

When not using a fixed-frequency override, secondary control SHALL blend among square, saw, and sine character according to the module's waveshape mapping.

#### Scenario: Shape morph

- **WHEN** secondary control moves across its range
- **THEN** the generated waveform morphs continuously among the supported shapes without discontinuities in the control dimension

### Requirement: Saturated stereo output

Generated tones SHALL be written as identical left and right samples, clamped to 16-bit PCM range.

#### Scenario: Mono-in-stereo test signal

- **WHEN** a tone is generated for a buffer period
- **THEN** each stereo frame contains the same sample value in both channels
