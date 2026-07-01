## ADDED Requirements

### Requirement: Firmware-equivalent audio format

The module chain simulator SHALL process audio using 44.1 kHz sample rate, stereo interleaved 16-bit signed PCM, and a fixed per-call buffer size matching the firmware streaming buffer length (512 int16 samples per path invocation).

#### Scenario: Buffer contract matches firmware

- **WHEN** the simulator invokes module processing for one buffer period
- **THEN** each path receives and produces exactly the firmware buffer sample count at the firmware sample rate

### Requirement: Phase 0 single virtual unit

In Phase 0, the simulator SHALL host exactly one virtual processing unit. Multi-unit chains, per-slot module assignment, and rightmost loopback SHALL NOT be required.

#### Scenario: One unit only

- **WHEN** the Phase 0 simulator is running
- **THEN** audio passes through a single virtual unit before reaching speakers

### Requirement: Compiled module processing

The simulator SHALL execute processing logic compiled from the same firmware module sources used on device, not a reimplemented approximation in the host language.

#### Scenario: At least two module variants

- **WHEN** the operator selects different module types offered by the spike
- **THEN** each selection runs the corresponding compiled module variant and produces audibly distinct behavior (including at least one passthrough-like variant and one effect variant)

### Requirement: Dual-path invocation order

For each audio buffer period, the simulator SHALL invoke upstream processing before downstream processing on the virtual unit, passing the same simulated control state to both calls.

#### Scenario: Path order preserved

- **WHEN** one buffer period is processed
- **THEN** upstream processing completes before downstream processing begins for that period

### Requirement: Simulated potentiometer controls

The simulator SHALL expose primary and secondary control values in the normalized zero-to-one range and supply them to module processing as smoothed pot state without requiring physical ADC input.

#### Scenario: Controls affect downstream behavior

- **WHEN** the operator adjusts a simulated pot while an effect module that responds to controls is active
- **THEN** the audible output changes in a way consistent with that module's downstream control mapping

### Requirement: Optional microphone input

The simulator SHALL allow the operator to enable or disable feeding captured microphone audio into the virtual unit's downstream input. When disabled, downstream input SHALL be silence.

#### Scenario: Mic enabled

- **WHEN** microphone input is enabled and capture permission is granted
- **THEN** live microphone audio is audible at the speakers after passing through the active module processing

#### Scenario: Mic disabled

- **WHEN** microphone input is disabled
- **THEN** downstream input is silence and output reflects processing of silence (or internal module behavior such as self-generated tones if applicable)

#### Scenario: Mic unavailable

- **WHEN** microphone input is enabled but capture is unavailable or denied
- **THEN** the simulator continues without crashing and downstream input is treated as silence

### Requirement: Speaker output from upstream path

The simulator SHALL play the virtual unit's upstream processing output to the host device speakers (or default audio output).

#### Scenario: Audible round trip

- **WHEN** audio is running and a non-silent input or self-generating module is active
- **THEN** the operator hears continuous output on the default playback device

### Requirement: Real-time interactive operation

The simulator SHALL process audio continuously in real time while running, with control changes taking effect without restarting the audio engine.

#### Scenario: Live control adjustment

- **WHEN** the operator moves a simulated pot or toggles microphone input during playback
- **THEN** processing continues and output reflects the new settings within a small number of buffer periods
