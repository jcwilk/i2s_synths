## ADDED Requirements

### Requirement: Upstream capture with passthrough

On the upstream path, the merger module SHALL copy input to output and SHALL enqueue the same samples into a delay line for later use on the downstream path.

#### Scenario: Upstream audible passthrough

- **WHEN** upstream audio arrives
- **THEN** upstream output matches upstream input while a copy is retained for downstream mixing

### Requirement: Downstream delayed blend

On the downstream path, the module SHALL mix the current downstream input with delayed upstream samples retrieved from the capture buffer. Primary control SHALL scale the downstream contribution; secondary control SHALL scale the delayed upstream contribution.

#### Scenario: Independent mix coefficients

- **WHEN** primary is zero and secondary is maximum
- **THEN** output is dominated by delayed upstream content

- **WHEN** secondary is zero and primary is maximum
- **THEN** output is dominated by current downstream input

### Requirement: Gentle peak limiting on merge

When merged downstream samples exceed a configured peak threshold, the module SHALL reduce gain with a releasing envelope rather than hard-clipping immediately, then SHALL clamp final samples to 16-bit range.

#### Scenario: Loud merge does not wrap

- **WHEN** downstream and delayed upstream combine above the limiting threshold
- **THEN** output peaks are reduced smoothly and remain within 16-bit bounds

### Requirement: Upstream underrun handling

If fewer delayed upstream samples are available than required for a downstream buffer, the module SHALL substitute silence for missing samples, SHALL signal an underrun indication, and SHALL attempt to stabilize the capture buffer.

#### Scenario: Missing upstream history

- **WHEN** the capture buffer cannot supply a full downstream buffer of delayed upstream audio
- **THEN** missing positions emit silence and a visible underrun indication is triggered

### Requirement: Optional downstream-to-upstream forwarding

When enabled for the build, the module SHALL forward a scaled copy of the downstream contribution into a reverse capture path and SHALL mix that forwarded audio into upstream output on subsequent upstream buffers, using the same limiting behavior as the downstream merge.

#### Scenario: Forward path active

- **WHEN** downstream-to-upstream forwarding is enabled
- **THEN** downstream energy reappears on the upstream path after buffering and limiting consistent with downstream processing

### Requirement: Overrun indication

When a capture buffer overflows because incoming audio exceeds capacity, the module SHALL discard oldest data according to its ring policy and SHALL signal an overrun indication.

#### Scenario: Capture buffer full

- **WHEN** upstream enqueue would exceed ring capacity
- **THEN** overrun is signaled without halting audio processing
