## ADDED Requirements

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

## MODIFIED Requirements

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
