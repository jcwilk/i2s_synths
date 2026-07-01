# Delay Module — Focused Specification

Concise description of the unique behavior and mechanisms of the variable‑time delay.

---

## What it does (unique aspects)

- Single‑tap, variable delay line that retimes clicklessly when delay time changes during playback.
- No feedback path and no wet/dry mix: emits only the delayed copy.
- Mimics a tape loop delay, except a circular buffer instead of a tape loop.

## Controls (module‑specific)

- Primary control - length (0..1): linearly maps to the usable delay span from as little delay as possible to full delay of the whole buffer’s capacity. This is sort of like making the "tape loop" longer or shorter.
- Secondary control - speed (0..1): linearly maps between 0 being a ratio of 1:1 where every output sample corresponds to 1 stored sample, vs 1 being full speed where some number of stored samples get averaged together per output sample. This is sort of like making the "tape loop" go faster or slower.
  
Together, the length and speed determine how long the delay is - min length and max speed would be a very short delay, max length and min speed would be a very long delay

## Control filtering (drag/hysteresis)

Analog controls jitter even after smoothing, which causes tiny, rapid span changes and audible artifacts during retime. To suppress this while preserving a responsive feel during motion, apply a simple drag/hysteresis filter to the effective length control value.

- Let `L_in ∈ [0,1]` be the already-smoothed, dead‑zoned length control.
- Maintain an internal `L_eff ∈ [0,1]` used for span mapping.
- Use a threshold `X ∈ (0,1)`, typically small (e.g., 0.02–0.05).

Update rule per control tick:

```
if (L_in - L_eff) > X:    L_eff = L_in - X
else if (L_eff - L_in) > X: L_eff = L_in + X
else:                       L_eff = L_eff  // unchanged
```

Effects and reasoning:
- When the knob is moving, the effective control trails the true control by exactly `X`, creating a smooth, predictable “drag” without stair‑stepping.
- When the knob stops and jitter is within `±X`, `L_eff` freezes, eliminating chatter‑induced retimes.
- Larger `X` increases stability but adds more lag; smaller `X` is snappier but allows more micro‑movement.

This filter operates after basic smoothing/dead‑zoning and before mapping to span. It complements the tape cutoff model (read‑before‑write, wrap within the current span) by minimizing re‑timing work except near the loop boundary.

## Core mechanism

- Circular buffer continuously records input; output reads a fixed span behind the write cursor.
- Span is always an even number of samples to maintain frame alignment.
- Use the terminology of "frame" vs "sample" where each frame includes two samples. Most operations should be performing in terms of "frames" to avoid having to constantly deal with enforcing even alignment, with "samples" only being used by low level functions charged with actually reading/writing.
- In order to support fine-grain speed adjustment, the number of frames should be a float rather than an int, and the remainder from the last iteration should be tracked. When at speed > 0, each output sample should correspond to a range of input samples where the boundary between input sample ranges presumably often falling in the middle of a sample, so that sample should be "split" and weighted appropriately.

## Retime behavior (on delay change)

The potentiometers controlling the parameters are analog reads and will fluctuate back and forth, so it's important to minimize the amount that the sound in the buffer is perturbed by these back-and-forth fluctuations:

- Track how long the "tape" is by storing a "cutoff", for example at half the duration, where that point will be considered the end of the "tape" and it will loop back to the start of the circular buffer once it reaches that earlier "cutoff" point. Whenever it gets to the end of the "tape" (whether due to early cutoff or if its using the full tape) it should do a short fade into the start of the tape to avoid a speaker pop.
- Increasing span (inserting a gap): Increase the "cutoff" and write silence to the new parts of the tape that are exposed. Fade the prior final sample into the beginning of the new silence to avoid a speaker pop.
- Decreasing span (removing history): Move the "cutoff" to the earlier point, essentially removing part of the circular buffer. If the point that's being read/written is past the new cutoff then it should be sent to the beginning, as if it just ran off the end like normal. It's important to make the prior sample fade into the start of the buffer to avoid a speaker pop, just like how it would do this if it reached the "cutoff" like normal without a cutoff shift.

### Fade behavior (precise)

This section defines the exact fade operations to avoid pops while the delay length changes.

- Shortening (X → Y, with Y < X):
  1) Let `A` be a copy of the last `DELAY_CROSSFADE_MS` segment immediately before `X` (copy A before any modifications).
  2) Fade out the `DELAY_CROSSFADE_MS` segment immediately before `Y` (amplitude ramps to 0 at `Y`).
  3) Mix `A` into that same window with a fade‑in (amplitude ramps from 0 at `Y - window` to 1 at `Y`).
  4) Set the cutoff to `Y`. If the write cursor lies beyond `Y`, wrap it to the start.
  - Rationale: The new end leads smoothly into the start while preserving continuity with the prior tail; overlapping windows still work because A was copied first.

- Lengthening (Y → X, with X > Y):
  1) Zero all newly exposed samples `[Y, X)`.
  2) Create a global staging buffer `B` of length `DELAY_CROSSFADE_MS` on the first extension only per loop (i.e., only if `B` is currently empty). `B` contains a version of the previous tail faded in from silence. Important: copy the previous tail into `B` before any fades to silence are applied. Do not overwrite `B` on subsequent extensions within the same loop.
  3) Defer mixing `B` into the tape until playback approaches the new end: when the write/read position is within the last `DELAY_CROSSFADE_MS` frames before the current cutoff, mix in `B` with a fade‑in while the tape contributes a fade‑out. This ensures a single contiguous fade at the end even if multiple small extensions occur.
  4) After the read passes the end and wraps to the start, clear `B`. This makes the next extension in the new loop eligible to stage a fresh tail.
  - Rationale: Subsequent extensions in the same loop typically expose silence, which will not match the beginning of the buffer. Capturing `B` only on the first extension preserves spectral continuity with the loop start and avoids multiple small, mismatched fades; a single, well‑placed fade is applied when the boundary is reached. Clearing `B` on wrap enables a new first‑extension capture in the next loop.

Staged buffer `B` lifecycle (summary):
- Capture: On the first lengthen event after a wrap, if `B` is empty.
- Use: Mixed once as playback approaches the current end; multiple lengthens within the same loop reuse the same `B` and do not restage.
- Clear: Immediately after the read wraps to index 0, regardless of whether a mix occurred, so the next loop can stage anew.

Notes:
- All fades use linear ramps over `DELAY_CROSSFADE_MS` converted to frames (rounded up to at least one frame and clamped to a sane maximum relative to tape length).
- Mix operations use 16‑bit saturation after rounding. Frames are used for alignment; sample indexing stays even.

## Startup

- Buffer is zeroed; write index reset; span begins at maximum. Output is silence until sufficient history accumulates.

## Safety/constraints

- Control values are clamped to [0,1] with deadband filtering.
- Span is clamped to valid range and coerced to even alignment.
- Output uses 16‑bit saturation after rounding.

## Tunables (design‑time)

- Total circular buffer size (limits what the maximum possible delay is)
- Control deadband threshold
- Maximum crossfade duration (in terms of milliseconds, which get converted into frames and rounded up)

## Acceptance criteria

- Changing delay produces no audible clicks/pops; only brief, smoothed transients.
- Output delay matches the configured span within one sample.
- Additional work on parameter changes is bounded by the configured fade window.


