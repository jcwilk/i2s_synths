## ADDED Requirements

### Requirement: Single-tap tape-loop delay

The delay module SHALL implement a single-tap delay line modeled as a looping tape: input is continuously recorded into a circular buffer and output emits the delayed copy from a fixed offset behind the write position. The module SHALL NOT provide feedback mixing or wet/dry balance; audible output on the downstream path SHALL be the delayed signal only.

#### Scenario: No feedback path

- **WHEN** downstream delay processing runs
- **THEN** output does not re-inject delayed audio back into the record path within the same module

#### Scenario: Delayed copy only

- **WHEN** sufficient delay history exists
- **THEN** downstream output reflects audio from earlier in the buffer, not the current input sample

### Requirement: Variable tape length

Primary control SHALL map linearly from minimum usable delay span to maximum usable delay span within the allocated buffer capacity. Increasing primary control lengthens the active tape; decreasing it shortens the active tape.

#### Scenario: Minimum length

- **WHEN** primary control is at minimum
- **THEN** effective delay span is the shortest supported audible span

#### Scenario: Maximum length

- **WHEN** primary control is at maximum
- **THEN** effective delay span uses the fullest usable capacity of the allocated buffer

### Requirement: Variable tape speed

Secondary control SHALL adjust playback speed along the tape from nominal 1:1 frame advance at minimum toward faster advance at maximum. At speeds above nominal, multiple recorded frames MAY be combined per output frame using weighted averaging. Effective delay duration SHALL depend on both primary length and secondary speed together.

#### Scenario: Nominal speed

- **WHEN** secondary control is at minimum
- **THEN** each output frame advances one recorded frame through the tape

#### Scenario: Faster-than-nominal speed

- **WHEN** secondary control is above minimum
- **THEN** output advances through recorded material faster than one frame per output frame and effective delay time shortens relative to the same primary length at nominal speed

### Requirement: Length drag and hysteresis

After upstream control smoothing, the module SHALL apply drag/hysteresis to the effective length control: when the smoothed primary input diverges from the effective length by more than a small threshold, the effective length SHALL trail the input by exactly that threshold; when divergence is within the threshold, the effective length SHALL remain unchanged.

#### Scenario: Knob jitter suppressed

- **WHEN** smoothed primary control jitters within the hysteresis band
- **THEN** mapped tape length does not change

#### Scenario: Deliberate knob motion

- **WHEN** smoothed primary control moves beyond the hysteresis band
- **THEN** effective length follows smoothly, trailing the control by the threshold amount

### Requirement: Stereo frame alignment

Delay bookkeeping SHALL operate on stereo frames (pairs of interleaved samples). Active span and buffer indices SHALL remain frame-aligned.

#### Scenario: Even sample indexing

- **WHEN** read and write positions advance
- **THEN** they always refer to the start of a left-right sample pair

### Requirement: Read-before-write tape advance

Each processing step SHALL emit the sample already stored at the current write position, then overwrite that position with new input, then advance the write cursor within the active span, wrapping to the start when the span end is reached.

#### Scenario: One-loop latency

- **WHEN** the tape has been filling continuously
- **THEN** output at a position reflects content written one loop earlier at that position

### Requirement: Clickless span shortening

When the active span decreases during playback, the module SHALL crossfade from the prior tail into the new loop boundary so that shortening does not produce discontinuities or speaker pops.

#### Scenario: Shorten while playing

- **WHEN** primary control requests a shorter span during active audio
- **THEN** listeners hear a brief smoothed transition rather than a click at the new boundary

### Requirement: Clickless span lengthening

When the active span increases during playback, newly exposed buffer regions SHALL be silent, and the module SHALL preserve continuity between the previous tail and the new boundary using a staged tail capture that is applied once as playback approaches the end of the lengthened span within the current loop.

#### Scenario: Lengthen while playing

- **WHEN** primary control requests a longer span during active audio
- **THEN** newly exposed regions contribute silence until played, and the transition at the lengthened end is smoothed without multiple conflicting fades from repeated small extensions in the same loop

#### Scenario: Staged tail resets on wrap

- **WHEN** playback wraps from the span end back to the start
- **THEN** any staged lengthening tail is cleared so the next lengthen event in the new loop may capture a fresh tail

### Requirement: Clickless normal loop wrap

When playback reaches the end of the active span under unchanged controls, the module SHALL crossfade into the start of the tape so that ordinary looping does not produce speaker pops.

#### Scenario: Continuous loop

- **WHEN** the write cursor completes a full span without a control change
- **THEN** the transition from tape end to tape start is smoothed

### Requirement: Startup behavior

At initialization, the delay buffer SHALL be cleared, the write cursor reset, and the active span set to maximum usable length. Output SHALL remain silent until enough samples have been recorded to fill the active span.

#### Scenario: Power-on silence

- **WHEN** the module starts with no prior history
- **THEN** downstream output is silent

#### Scenario: Initial span at maximum

- **WHEN** initialization completes
- **THEN** active span equals maximum usable length and effective length control is initialized to the value that maps to that maximum span

### Requirement: Saturated crossfades and mixes

All crossfades and additive blends used for retime and loop-wrap behavior SHALL use linear ramps over a bounded fade duration and SHALL saturate to 16-bit PCM range after rounding.

#### Scenario: Fade bounded work

- **WHEN** span changes or loop wrap occurs
- **THEN** additional smoothing work is limited to the configured crossfade duration rather than rewriting unbounded buffer history

### Requirement: Allocation fallback

The module SHALL attempt to allocate delay storage up to the configured maximum duration, preferring external RAM when available, and SHALL reduce requested capacity until allocation succeeds or the minimum supported span is reached. If no buffer can be allocated, downstream processing SHALL pass input through unchanged.

#### Scenario: Reduced capacity on memory pressure

- **WHEN** full requested storage cannot be allocated
- **THEN** the module operates at the largest successfully allocated span down to the minimum supported span

#### Scenario: Total allocation failure

- **WHEN** no viable buffer can be allocated
- **THEN** downstream audio passes through without delay rather than failing silently with corrupted output
