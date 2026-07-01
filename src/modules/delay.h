#ifndef MODULE_DELAY_H
#define MODULE_DELAY_H

#include <Arduino.h>
#include <string.h>
#include <math.h>
#include <esp32-hal-psram.h>
#include "../config/constants.h"
#include "../input/pots.h"

// Internal delay tunables (kept local to this module)
#define DELAY_MAX_MS 2000                 // total tape length in milliseconds (requested upper bound)
#define DELAY_MIN_FRAMES (SAMPLE_RATE / 10)  // ~100 ms minimum span (audible)
#define DELAY_FRAMES_HARDCAP (SAMPLE_RATE * 2) // allow up to ~2000ms if RAM permits
#define DELAY_MS_TO_FRAMES(ms) (((SAMPLE_RATE * (ms)) + 999) / 1000)
#define DELAY_DEFAULT_FRAMES_RAW DELAY_MS_TO_FRAMES(DELAY_MAX_MS)
#define DELAY_BUFFER_FRAMES_DEFAULT ((DELAY_DEFAULT_FRAMES_RAW > DELAY_FRAMES_HARDCAP) ? DELAY_FRAMES_HARDCAP : ((DELAY_DEFAULT_FRAMES_RAW < DELAY_MIN_FRAMES) ? DELAY_MIN_FRAMES : DELAY_DEFAULT_FRAMES_RAW))
// Crossfade window for retime behavior (ms)
#define DELAY_CROSSFADE_MS 8
// Global state and ring buffer pointer (allocated at runtime; prefer PSRAM)
static int16_t* g_ring = nullptr;
static int g_bufferFrames = 0;  // capacity in frames
static int g_writeFrameIndex = 0;        // 0..g_bufferFrames-1
static int g_spanFrames = 0;             // current delay span
static int g_targetSpanFrames = 0;
static uint32_t g_framesWritten = 0;     // total frames ever written
static int g_fadeRemainingFrames = 0;    // reserved for future crossfades
// Drag/hysteresis state for primary pot (downstream only)
static float g_effective_length01 = 0.0f; // filtered control value in [0,1]
static const float kLengthDragThreshold = 0.03f; // X in [0,1] space

// --- Fade staging (lengthen) ---
static int16_t* g_stageB = nullptr;   // staged tail buffer (interleaved)
static int g_stageBFrames = 0;        // frames in stage B
static bool g_hasStageB = false;      // true if B is active for current loop

// Runtime buffer allocation helper (prefer PSRAM, fallback to internal RAM, reduce if needed)
static inline void delayAllocateRing() {
  int framesDesired = DELAY_BUFFER_FRAMES_DEFAULT;
  while (framesDesired >= DELAY_MIN_FRAMES) {
    size_t bytes = (size_t)framesDesired * 2 * sizeof(int16_t);
    int16_t* ptr = nullptr;
    if (psramFound()) {
      ptr = (int16_t*)ps_malloc(bytes);
    }
    if (!ptr) {
      ptr = (int16_t*)malloc(bytes);
    }
    if (ptr) {
      memset(ptr, 0, bytes);
      g_ring = ptr;
      g_bufferFrames = framesDesired;
      const int maxUsable = (g_bufferFrames > 1) ? (g_bufferFrames - 1) : 1;
      g_spanFrames = maxUsable;
      g_targetSpanFrames = g_spanFrames;
      g_writeFrameIndex = 0;
      g_framesWritten = 0;
      g_fadeRemainingFrames = 0;
      return;
    }
    framesDesired /= 2;
  }
  g_ring = nullptr;
  g_bufferFrames = 0;
}

inline void moduleSetup() {
  g_effective_length01 = 0.0f;
  delayAllocateRing();
  Serial.printf("Delay module active. Allocated frames=%d (%.1f ms)\n",
                g_bufferFrames,
                1000.0f * (float)g_bufferFrames / (float)SAMPLE_RATE);
}

// --- Fade helpers (kept separate from main loop for clarity) ---
static inline int frameToSampleIndex(int frameIndex) { return frameIndex * 2; }
static inline int rangeFrameClampCount(int startFrame, int countFrames, int limitFrames) {
  if (startFrame < 0) startFrame = 0;
  if (countFrames < 0) countFrames = 0;
  if (startFrame + countFrames > limitFrames) countFrames = (limitFrames - startFrame);
  if (countFrames < 0) countFrames = 0;
  return countFrames;
}

static inline int delayCrossfadeFrames() {
  int frames = DELAY_MS_TO_FRAMES(DELAY_CROSSFADE_MS);
  if (frames < 1) frames = 1;
  int maxHalf = (g_bufferFrames > 1) ? (g_bufferFrames / 2) : 1;
  if (frames > maxHalf) frames = maxHalf;
  return frames;
}

static inline void delayBlendSaturating(int16_t aL, int16_t aR,
                                        int16_t bL, int16_t bR,
                                        int num, int den,
                                        int16_t& outL, int16_t& outR) {
  if (den <= 0) { outL = aL; outR = aR; return; }
  if (num < 0) num = 0; if (num > den) num = den;
  const int inv = den - num;
  int32_t l = (int32_t)aL * inv + (int32_t)bL * num;
  int32_t r = (int32_t)aR * inv + (int32_t)bR * num;
  if (l >= 0) l += den / 2; else l -= den / 2;
  if (r >= 0) r += den / 2; else r -= den / 2;
  l /= den; r /= den;
  if (l > 32767) l = 32767; if (l < -32768) l = -32768;
  if (r > 32767) r = 32767; if (r < -32768) r = -32768;
  outL = (int16_t)l; outR = (int16_t)r;
}

// Mutate buffer range: apply linear fade out to silence over [start, start+count)
static inline void buffer_apply_fade_out_linear(int16_t* buf, int startFrame, int countFrames) {
  if (!buf || countFrames <= 0) return;
  int den = countFrames;
  for (int k = 0; k < countFrames; ++k) {
    int num = den - (k + 1); // 1->0 across window
    int si = frameToSampleIndex(startFrame + k);
    int16_t inL = buf[si + 0];
    int16_t inR = buf[si + 1];
    int16_t outL, outR;
    delayBlendSaturating(inL, inR, 0, 0, num, den, outL, outR);
    buf[si + 0] = outL;
    buf[si + 1] = outR;
  }
}

// Mutate buffer range: apply linear fade in from silence to current content over range
static inline void buffer_apply_fade_in_linear(int16_t* buf, int startFrame, int countFrames) {
  if (!buf || countFrames <= 0) return;
  int den = countFrames;
  for (int k = 0; k < countFrames; ++k) {
    int num = (k + 1); // 0->1 across window
    int si = frameToSampleIndex(startFrame + k);
    int16_t inL = buf[si + 0];
    int16_t inR = buf[si + 1];
    int16_t outL, outR;
    delayBlendSaturating(0, 0, inL, inR, num, den, outL, outR);
    buf[si + 0] = outL;
    buf[si + 1] = outR;
  }
}

// Copy a portion of a buffer into a newly allocated buffer (interleaved stereo)
static inline int16_t* buffer_copy_alloc(const int16_t* src, int startFrame, int countFrames) {
  if (!src || countFrames <= 0) return nullptr;
  size_t samples = (size_t)countFrames * 2;
  int16_t* out = (int16_t*)malloc(sizeof(int16_t) * samples);
  if (!out) return nullptr;
  for (int k = 0; k < countFrames; ++k) {
    int si = frameToSampleIndex(startFrame + k);
    out[2 * k + 0] = src[si + 0];
    out[2 * k + 1] = src[si + 1];
  }
  return out;
}

// Crossfade: dst[dstStart .. dstStart+count) = crossfade(dst, src) with linear window
static inline void buffer_mix_crossfade_linear(int16_t* dst, int dstStartFrame,
                                               const int16_t* src, int srcStartFrame,
                                               int countFrames) {
  if (!dst || !src || countFrames <= 0) return;
  int den = countFrames;
  for (int k = 0; k < countFrames; ++k) {
    int num = (k + 1);
    int dsi = frameToSampleIndex(dstStartFrame + k);
    int ssi = frameToSampleIndex(srcStartFrame + k);
    int16_t curL = dst[dsi + 0];
    int16_t curR = dst[dsi + 1];
    int16_t srcL = src[ssi + 0];
    int16_t srcR = src[ssi + 1];
    int16_t outL, outR;
    delayBlendSaturating(curL, curR, srcL, srcR, num, den, outL, outR);
    dst[dsi + 0] = outL;
    dst[dsi + 1] = outR;
  }
}

// Mix a buffer segment into output with a linear ramp (0->1) at step k (0-based)
static inline void output_mix_linear_from_buffer_segment(int16_t& outL,
                                                        int16_t& outR,
                                                        const int16_t* src,
                                                        int srcFrameStart,
                                                        int k,
                                                        int countFrames) {
  if (!src || countFrames <= 0) return;
  int den = countFrames;
  int num = (k + 1);
  int ssi = frameToSampleIndex(srcFrameStart + k);
  int16_t srcL = src[ssi + 0];
  int16_t srcR = src[ssi + 1];
  int16_t mixedL, mixedR;
  delayBlendSaturating(outL, outR, srcL, srcR, num, den, mixedL, mixedR);
  outL = mixedL;
  outR = mixedR;
}

// Shorten: pre-bake crossfade at the new end using the tail from the old end
static inline void delayPrebakeShortenCrossfade(int oldSpan, int newSpan) {
  int count = delayCrossfadeFrames();
  if (!g_ring) return;
  if (count > newSpan) count = newSpan;
  if (count > oldSpan) count = oldSpan;
  if (count <= 0) return;
  const int startNew = newSpan - count;
  const int startOld = oldSpan - count;
  int16_t* tmpA = buffer_copy_alloc(g_ring, startOld, count);
  if (!tmpA) return;
  buffer_mix_crossfade_linear(g_ring, startNew, tmpA, 0, count);
  free(tmpA);
}

// Lengthen: stage the previous tail and defer mixing until approaching the end
static inline void delayStageLengthenTail(int oldSpan, int newSpan) {
  if (g_hasStageB) return;
  int count = delayCrossfadeFrames();
  if (count > oldSpan) count = oldSpan;
  if (count > newSpan) count = newSpan;
  if (!g_ring || count <= 0) return;
  const int startOld = oldSpan - count;
  int16_t* buf = buffer_copy_alloc(g_ring, startOld, count);
  if (!buf) return;
  // Pre-shape staged buffer with fade-in so read-side can add it directly
  buffer_apply_fade_in_linear(buf, 0, count);
  // Also fade out the corresponding old tail region in the ring to reduce pre-boundary pops
  buffer_apply_fade_out_linear(g_ring, startOld, count);
  g_stageB = buf;
  g_stageBFrames = count;
  g_hasStageB = true;
}

static inline void delayClearStageBOnWrap(int nextWriteIdx) {
  if (nextWriteIdx == 0 && g_hasStageB) {
    free(g_stageB);
    g_stageB = nullptr;
    g_stageBFrames = 0;
    g_hasStageB = false;
  }
}

static inline void delayMaybeMixStagedTailIntoOutput(int writeIdx,
                                                     int16_t& outL,
                                                     int16_t& outR) {
  if (!g_hasStageB || !g_stageB || g_stageBFrames <= 0) return;
  int startFade = g_spanFrames - g_stageBFrames;
  if (startFade < 0) startFade = 0;
  if (writeIdx < startFade || writeIdx >= g_spanFrames) return;
  int k = writeIdx - startFade; // 0..g_stageBFrames-1
  // Add staged buffer sample (already faded) into output with saturation
  int ssi = frameToSampleIndex(k);
  int32_t l = (int32_t)outL + (int32_t)g_stageB[ssi + 0];
  int32_t r = (int32_t)outR + (int32_t)g_stageB[ssi + 1];
  if (l > 32767) l = 32767; if (l < -32768) l = -32768;
  if (r > 32767) r = 32767; if (r < -32768) r = -32768;
  outL = (int16_t)l;
  outR = (int16_t)r;
}

static inline int delayMapPotToSpanFrames(float pot01) {
  if (pot01 < 0.0f) pot01 = 0.0f;
  if (pot01 > 1.0f) pot01 = 1.0f;
  const int maxF = g_bufferFrames;
  const int maxUsable = (maxF > 1) ? (maxF - 1) : 1;
  int span = DELAY_MIN_FRAMES + (int)floorf(pot01 * (float)(maxUsable - DELAY_MIN_FRAMES) + 0.5f);
  if (span < DELAY_MIN_FRAMES) span = DELAY_MIN_FRAMES;
  if (span > maxUsable) span = maxUsable;
  return span;
}

static inline void delayUpdateSpan(int newSpanFrames) {
  if (newSpanFrames < DELAY_MIN_FRAMES) newSpanFrames = DELAY_MIN_FRAMES;
  int maxUsable = (g_bufferFrames > 1) ? (g_bufferFrames - 1) : 1;
  if (newSpanFrames > maxUsable) newSpanFrames = maxUsable;
  if (newSpanFrames == g_spanFrames) return;
  const int oldSpan = g_spanFrames;
  if (oldSpan == 0) {
    g_spanFrames = newSpanFrames;
    if (g_writeFrameIndex >= g_spanFrames) g_writeFrameIndex = 0;
    return;
  }
  if (newSpanFrames > oldSpan) {
    // Increasing length: zero newly exposed region [oldSpan, newSpan)
    const int startFrame = oldSpan;
    const int endFrame = newSpanFrames;
    if (g_ring && endFrame > startFrame) {
      const int startSample = startFrame * 2;
      const int countSamples = (endFrame - startFrame) * 2;
      memset(&g_ring[startSample], 0, (size_t)countSamples * sizeof(int16_t));
    }
    // Stage previous tail for a deferred crossfade at the boundary (use newSpan for clamps)
    delayStageLengthenTail(oldSpan, newSpanFrames);
    g_spanFrames = newSpanFrames;
  } else {
    // Decreasing length: pre-bake crossfade then move cutoff
    delayPrebakeShortenCrossfade(oldSpan, newSpanFrames);
    g_spanFrames = newSpanFrames;
    if (g_writeFrameIndex >= g_spanFrames) g_writeFrameIndex = 0;
  }
}

static inline void delayProcessDownstream(int16_t* inputBuffer, // interleaved
                                         int16_t* outputBuffer,
                                         int samplesLength,
                                         DualPotsState pots_state) {
  if (!(samplesLength > 0 && outputBuffer)) return;
  if (!g_ring) {
    // Allocation failed; passthrough
    if (inputBuffer) memcpy(outputBuffer, inputBuffer, (size_t)samplesLength * sizeof(int16_t));
    else memset(outputBuffer, 0, (size_t)samplesLength * sizeof(int16_t));
    return;
  }

  // Map primary pot with drag/hysteresis: only change when farther than threshold,
  // and when changing, trail the actual pot by exactly the threshold.
  float potLen01 = potsPrimaryLinear(pots_state);
  float delta = potLen01 - g_effective_length01;
  if (delta > kLengthDragThreshold) {
    g_effective_length01 = potLen01 - kLengthDragThreshold;
  } else if (delta < -kLengthDragThreshold) {
    g_effective_length01 = potLen01 + kLengthDragThreshold;
  } // else keep previous effective value
  if (g_effective_length01 < 0.0f) g_effective_length01 = 0.0f;
  if (g_effective_length01 > 1.0f) g_effective_length01 = 1.0f;
  g_targetSpanFrames = delayMapPotToSpanFrames(g_effective_length01);
  if (g_targetSpanFrames != g_spanFrames) {
    delayUpdateSpan(g_targetSpanFrames);
  }
  // (debug log removed)

  // Process per frame (2 samples per frame)
  const int frames = samplesLength / 2;
  int writeIdx = g_writeFrameIndex;
  const int tapeFrames = g_spanFrames;
  for (int f = 0, si = 0; f < frames; ++f, si += 2) {
    // Read-before-write: emit the sample from one loop ago
    int16_t outL = 0;
    int16_t outR = 0;
    if (g_framesWritten >= (uint32_t)tapeFrames) {
      const int readSi = writeIdx * 2;
      outL = g_ring[readSi + 0];
      outR = g_ring[readSi + 1];
    }

    // Optionally blend staged B into output as we approach the end of the tape
    delayMaybeMixStagedTailIntoOutput(writeIdx, outL, outR);

    // Write current input (or silence if missing) into the current slot
    int16_t inL = 0, inR = 0;
    if (inputBuffer) {
      inL = inputBuffer[si + 0];
      inR = inputBuffer[si + 1];
    }
    const int bufSi = writeIdx * 2;
    g_ring[bufSi + 0] = inL;
    g_ring[bufSi + 1] = inR;

    outputBuffer[si + 0] = outL;
    outputBuffer[si + 1] = outR;

    // Advance write cursor within the current tape length
    writeIdx++;
    if (writeIdx >= g_spanFrames) writeIdx = 0;
    g_framesWritten++;
    // Clear staged buffer on wrap so next loop can restage if needed
    delayClearStageBOnWrap(writeIdx);
  }
  g_writeFrameIndex = writeIdx;
}

inline void moduleLoopUpstream(int16_t* inputBuffer,
                               int16_t* outputBuffer,
                               int samplesLength,
                               DualPotsState /*pots_state*/) {
  if (samplesLength > 0 && outputBuffer && inputBuffer) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

inline void moduleLoopDownstream(int16_t* inputBuffer,
                                 int16_t* outputBuffer,
                                 int samplesLength,
                                 DualPotsState pots_state) {
  delayProcessDownstream(inputBuffer, outputBuffer, samplesLength, pots_state);
}

#endif // MODULE_DELAY_H


