# module-chain-simulator Specification

## Purpose
The module chain simulator is a Phase 0 browser host for a single virtual audio processing unit. It runs module logic compiled from the same firmware sources as on-device builds (WebAssembly), processes stereo 44.1 kHz audio in real time through upstream and downstream paths, exposes simulated potentiometer controls whose smoothed values mirror firmware polling behavior, and displays scrolling peak-level metering for downstream input and upstream output so operators can observe dynamics while interactively adjusting effects.
## Requirements
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

### Requirement: Dual-path level metering

While audio is running, the module chain simulator SHALL display scrolling peak-level history for the virtual unit's downstream input and upstream output. Each history SHALL advance continuously without restarting the audio engine.

#### Scenario: Meters run during playback

- **WHEN** audio is running and at least one path carries non-silent signal
- **THEN** peak-level history for that path advances over time and remains visible to the operator

#### Scenario: Independent in and out channels

- **WHEN** audio is running
- **THEN** downstream input peaks and upstream output peaks are presented as distinguishable channels so the operator can compare input versus processed output

#### Scenario: Buffer-aligned peak samples

- **WHEN** the simulator completes processing for one firmware buffer period
- **THEN** at most one new peak sample per path is appended to that path's level history for that period

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

