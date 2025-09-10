#ifndef MODULE_MERGER_H
#define MODULE_MERGER_H

#include <Arduino.h>
#include <string.h>
#include "../config/constants.h"
#include "../ui/neopixel.h"
#include "../input/pots.h"

// Expect required symbols to be provided by the sketch (BUFFER_LEN, POT_PIN_PRIMARY/SECONDARY, ADC_11db)

// Compressor configuration
#ifndef MERGER_MAX_SIGNAL_LEVEL
#define MERGER_MAX_SIGNAL_LEVEL 24576 // ~75% of int16_t max
#endif
#ifndef MERGER_COMPRESSOR_RELEASE_RATE
#define MERGER_COMPRESSOR_RELEASE_RATE 0.99995f
#endif

// Ring buffer capacity: 64 frames of BUFFER_LEN samples
#ifndef MERGER_RING_CAPACITY_MULTIPLIER
#define MERGER_RING_CAPACITY_MULTIPLIER 64
#endif

// Expect the sketch to define BUFFER_LEN, POT_PIN_PRIMARY, POT_PIN_SECONDARY, and neopixel helpers

// State
static float mergerCurrentScaleRatio = 1.0f;
static float mergerEmaPrimary = 0.0f;
static float mergerEmaSecondary = 0.0f;

#ifndef MERGER_RING_CAPACITY
#define MERGER_RING_CAPACITY (BUFFER_LEN * MERGER_RING_CAPACITY_MULTIPLIER)
#endif
static int16_t mergerRing[MERGER_RING_CAPACITY];
static uint32_t mergerRingHead = 0;         // index of oldest sample
static uint32_t mergerRingTail = 0;         // index one past newest sample
static uint32_t mergerRingCountSamples = 0; // number of valid samples in buffer
static volatile bool mergerFlagOverrunEvent = false;

// Ensure neopixel mode constant exists
#ifndef NEOPIXEL_MODE_HOLD
#define NEOPIXEL_MODE_HOLD 0
#endif

// External helpers expected from sketch
// Provided by input/pots.h

// Arduino environment provides analog* APIs

// --- Ring buffer helpers ---
static inline uint32_t mergerRingSpace() {
  return (uint32_t)(MERGER_RING_CAPACITY - mergerRingCountSamples);
}

static inline uint32_t mergerMinU32(uint32_t a, uint32_t b) { return (a < b) ? a : b; }
static inline int32_t mergerAbs32(int32_t v) { return v < 0 ? -v : v; }

static inline void mergerRingDropOldest(uint32_t dropCount) {
  if (dropCount == 0) return;
  if (dropCount > mergerRingCountSamples) dropCount = mergerRingCountSamples;
  mergerRingHead += dropCount;
  if (mergerRingHead >= MERGER_RING_CAPACITY) mergerRingHead -= MERGER_RING_CAPACITY;
  mergerRingCountSamples -= dropCount;
}

static inline void mergerRingPushSamples(const int16_t* src, int count) {
  if (count <= 0) return;
  uint32_t space = mergerRingSpace();
  if (space < (uint32_t)count) {
    // Overrun: not enough space to store new samples. Drop oldest to make space.
    mergerFlagOverrunEvent = true;
    uint32_t needToFree = (uint32_t)count - space;
    mergerRingDropOldest(needToFree);
  }

  // Now we have enough contiguous logical space (may wrap physically)
  int first = (int)mergerMinU32((uint32_t)count, (uint32_t)(MERGER_RING_CAPACITY - mergerRingTail));
  memcpy(&mergerRing[mergerRingTail], src, first * sizeof(int16_t));
  int remaining = count - first;
  if (remaining > 0) {
    memcpy(&mergerRing[0], src + first, remaining * sizeof(int16_t));
  }
  mergerRingTail += (uint32_t)count;
  if (mergerRingTail >= MERGER_RING_CAPACITY) mergerRingTail -= MERGER_RING_CAPACITY;
  mergerRingCountSamples += (uint32_t)count;
}

static inline int mergerRingPopSamples(int16_t* dst, int count) {
  if (count <= 0) return 0;
  int avail = (int)mergerRingCountSamples;
  int toRead = avail < count ? avail : count;
  if (toRead <= 0) return 0;

  int first = (int)mergerMinU32((uint32_t)toRead, (uint32_t)(MERGER_RING_CAPACITY - mergerRingHead));
  memcpy(dst, &mergerRing[mergerRingHead], first * sizeof(int16_t));
  int remaining = toRead - first;
  if (remaining > 0) {
    memcpy(dst + first, &mergerRing[0], remaining * sizeof(int16_t));
  }
  mergerRingHead += (uint32_t)toRead;
  if (mergerRingHead >= MERGER_RING_CAPACITY) mergerRingHead -= MERGER_RING_CAPACITY;
  mergerRingCountSamples -= (uint32_t)toRead;
  return toRead;
}

static inline void mergerRingPrefillSilence(int count) {
  if (count <= 0) return;
  static int16_t zeros[BUFFER_LEN];
  memset(zeros, 0, sizeof(zeros));
  while (count > 0) {
    int chunk = count > BUFFER_LEN ? BUFFER_LEN : count;
    mergerRingPushSamples(zeros, chunk);
    count -= chunk;
  }
}

// --- Module API ---
inline void moduleSetup() {
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  mergerRingHead = 0;
  mergerRingTail = 0;
  mergerRingCountSamples = 0;
  mergerFlagOverrunEvent = false;
  memset(mergerRing, 0, sizeof(mergerRing));

  mergerCurrentScaleRatio = 1.0f;
  mergerEmaPrimary = 0.0f;
  mergerEmaSecondary = 0.0f;

  Serial.println("Merger module initialized");
}

inline void moduleLoopUpstream(int16_t* inputBuffer,
                               int16_t* outputBuffer,
                               int samplesLength) {
  // Upstream: passthrough AND enqueue samples into ring for downstream use
  if (samplesLength > 0 && inputBuffer && outputBuffer) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
    mergerRingPushSamples(inputBuffer, samplesLength);
  }

  // Handle any overrun event resulting from enqueue
  if (mergerFlagOverrunEvent) {
    mergerFlagOverrunEvent = false;
    neopixelSetTimedColor(25, 0, 0, 500, NEOPIXEL_MODE_HOLD); // red flash for overrun
  }
}

inline void moduleLoopDownstream(int16_t* inputBuffer,
                                 int16_t* outputBuffer,
                                 int samplesLength) {
  if (!(samplesLength > 0 && outputBuffer)) {
    return;
  }

  // Read both potentiometers for mix control
  float primaryCoeff = readPotWithSmoothingAndDeadZone(POT_PIN_PRIMARY, mergerEmaPrimary);
  float secondaryCoeff = readPotWithSmoothingAndDeadZone(POT_PIN_SECONDARY, mergerEmaSecondary);
  // Allow up to 110% gain on secondary at max to slightly boost overall volume
  secondaryCoeff *= 1.10f;
  if (secondaryCoeff > 1.10f) secondaryCoeff = 1.10f;

  int underrunMissing = 0;
  for (int i = 0; i < samplesLength; i++) {
    int16_t upstreamSample = 0;
    int popped = mergerRingPopSamples(&upstreamSample, 1);
    if (popped != 1) {
      underrunMissing++;
      upstreamSample = 0; // substitute silence for missing upstream sample
    }

    int32_t mergedSample = (int32_t)(inputBuffer ? (int32_t)inputBuffer[i] * primaryCoeff : 0) +
                           (int32_t)((int32_t)upstreamSample * secondaryCoeff);

    int32_t mergedAbs = mergerAbs32(mergedSample);
    if (mergedAbs > MERGER_MAX_SIGNAL_LEVEL) {
      mergerCurrentScaleRatio = (float)MERGER_MAX_SIGNAL_LEVEL / (float)mergedAbs;
    } else {
      mergerCurrentScaleRatio += (1.0f - mergerCurrentScaleRatio) * (1.0f - MERGER_COMPRESSOR_RELEASE_RATE);
    }

    mergedSample = (int32_t)(mergedSample * mergerCurrentScaleRatio);
    if (mergedSample > 32767) mergedSample = 32767;
    if (mergedSample < -32768) mergedSample = -32768;
    outputBuffer[i] = (int16_t)mergedSample;
  }

  // If we encountered underrun, indicate and try to recover by padding buffer with silence
  if (underrunMissing > 0) {
    neopixelSetTimedColor(25, 25, 0, 500, NEOPIXEL_MODE_HOLD); // yellow flash for underrun
    mergerRingPrefillSilence(BUFFER_LEN); // add a frame of silence to help stabilize buffer
  }
}

#endif // MODULE_MERGER_H


