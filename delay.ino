// Delay module for audio processing
#include <math.h>

// Use shared potentiometer helper
#include "src/input/pots.h"

#if ACTIVE_MODULE == MODULE_DELAY
// Delay buffer configuration
#define DELAY_SAMPLES (SAMPLE_RATE * 1) // 1 second worth of stereo frames
#define MIN_DELAY_RATIO 0.10f           // 10% of max when pot at 0
#define DELAY_DEADBAND 0.05f            // 5% deadband on smoothed pot value
#define DELAY_FADE_MS 15                // crossfade duration in milliseconds
#define MAX_FADE_FRAMES ((SAMPLE_RATE * DELAY_FADE_MS) / 1000 + 1)

// Circular buffer for up to 1-second delay (stereo interleaved L+R)
static int16_t delayBuffer[DELAY_SAMPLES * 2];
static uint32_t writeIndex = 0;                // write head (tail)
static uint32_t currentDelaySpan = DELAY_SAMPLES * 2; // distance between write and read, in int16_t units
static float primaryPotEma = 0.0f;             // EMA state for primary pot
static float lastAppliedPot01 = -1.0f;         // last pot value after smoothing and deadband

// temp storage for fade operations (stereo interleaved)
static int16_t fadeTemp[2 * MAX_FADE_FRAMES];

static inline uint32_t ensureEvenAtLeastTwo(uint32_t value) {
  if (value < 2u) return 2u;
  return (value & 1u) ? (value - 1u) : value;
}

static inline uint32_t addWrap(uint32_t index, uint32_t amount, uint32_t capacity) {
  index += amount;
  if (index >= capacity) index -= capacity;
  return index;
}

static inline uint32_t subWrap(uint32_t index, uint32_t amount, uint32_t capacity) {
  if (index >= amount) return index - amount;
  return capacity - (amount - index);
}

void moduleSetup() {
  // Initialize delay buffer to silence and start at max delay
  memset(delayBuffer, 0, sizeof(delayBuffer));
  writeIndex = 0;
  currentDelaySpan = DELAY_SAMPLES * 2; // even by construction
  Serial.println("Delay module initialized: variable delay using primary pot");
}

void moduleLoopUpstream(int16_t* inputBuffer,
                        int16_t* outputBuffer,
                        int samplesLength) {
  // Upstream path: passthrough by default
  if (samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

void moduleLoopDownstream(int16_t* inputBuffer,
                          int16_t* outputBuffer,
                          int samplesLength) {
  if (samplesLength <= 0) {
    return;
  }

  const uint32_t capacity = (uint32_t)DELAY_SAMPLES * 2u; // total int16_t slots (L+R interleaved)

  // Read primary potentiometer via helper and map to [MIN_RATIO .. 1.0]
  float pot01Raw = readPotWithSmoothingAndDeadZone(POT_PIN_PRIMARY, primaryPotEma);
  if (lastAppliedPot01 < 0.0f) {
    lastAppliedPot01 = pot01Raw;
  } else {
    if (fabsf(pot01Raw - lastAppliedPot01) >= DELAY_DEADBAND) {
      lastAppliedPot01 = pot01Raw;
    }
  }
  float ratio = MIN_DELAY_RATIO + lastAppliedPot01 * (1.0f - MIN_DELAY_RATIO);
  uint32_t targetSpan = ensureEvenAtLeastTwo((uint32_t)lroundf(ratio * (float)capacity));
  if (targetSpan > capacity) {
    targetSpan = capacity - (capacity & 1u ? 1u : 0u); // clamp, keep even
  }

  // Adjust write head to keep read position stable when span changes
  if (targetSpan != currentDelaySpan) {
    uint32_t readIndexStable = (writeIndex + capacity - currentDelaySpan) % capacity;
    uint32_t newWriteIndex = (readIndexStable + targetSpan) % capacity;

    if (targetSpan > currentDelaySpan) {
      // Lengthening: fill the newly created gap with a decaying hold of the last sample, then zeros
      uint32_t gapSamples = (newWriteIndex >= writeIndex)
        ? (newWriteIndex - writeIndex)
        : (capacity - writeIndex + newWriteIndex);
      // ensure even (stereo frames)
      gapSamples = ensureEvenAtLeastTwo(gapSamples);

      int16_t lastL = delayBuffer[subWrap(writeIndex, 2u, capacity)];
      int16_t lastR = delayBuffer[subWrap(writeIndex, 1u, capacity)];

      uint32_t gapFrames = gapSamples / 2u;
      uint32_t fadeFrames = gapFrames < (uint32_t)MAX_FADE_FRAMES ? gapFrames : (uint32_t)MAX_FADE_FRAMES;
      for (uint32_t f = 0; f < gapFrames; f++) {
        float t = (fadeFrames > 0 && f < fadeFrames) ? (1.0f - ((float)f / (float)fadeFrames)) : 0.0f;
        int32_t outL = (int32_t)((float)lastL * t);
        int32_t outR = (int32_t)((float)lastR * t);
        uint32_t pos = addWrap(writeIndex, f * 2u, capacity);
        delayBuffer[pos] = (int16_t)outL;
        delayBuffer[pos + 1u] = (int16_t)outR;
      }
    } else {
      // Shortening: crossfade the destination region (just before newWriteIndex)
      uint32_t deltaSamples = currentDelaySpan - targetSpan; // even
      uint32_t shiftFrames = (deltaSamples / 2u);
      uint32_t fadeFrames = shiftFrames < (uint32_t)MAX_FADE_FRAMES ? shiftFrames : (uint32_t)MAX_FADE_FRAMES;
      if (fadeFrames > 0) {
        // Source: the last fadeFrames before old writeIndex
        uint32_t srcStart = subWrap(writeIndex, fadeFrames * 2u, capacity);
        // Destination: the last fadeFrames before new write index
        uint32_t destStart = subWrap(newWriteIndex, fadeFrames * 2u, capacity);

        // Copy source into temp to avoid overlap issues
        for (uint32_t f = 0; f < fadeFrames; f++) {
          uint32_t s = addWrap(srcStart, f * 2u, capacity);
          fadeTemp[f * 2u] = delayBuffer[s];
          fadeTemp[f * 2u + 1u] = delayBuffer[s + 1u];
        }

        // Crossfade existing destination content -> source content
        for (uint32_t f = 0; f < fadeFrames; f++) {
          float t = (float)(f + 1u) / (float)fadeFrames; // 0..1
          uint32_t d = addWrap(destStart, f * 2u, capacity);
          int16_t destL = delayBuffer[d];
          int16_t destR = delayBuffer[d + 1u];
          int16_t srcL = fadeTemp[f * 2u];
          int16_t srcR = fadeTemp[f * 2u + 1u];

          int32_t mixL = (int32_t)((1.0f - t) * (float)destL + t * (float)srcL);
          int32_t mixR = (int32_t)((1.0f - t) * (float)destR + t * (float)srcR);
          delayBuffer[d] = (int16_t)mixL;
          delayBuffer[d + 1u] = (int16_t)mixR;
        }
      }
    }

    writeIndex = newWriteIndex;
    currentDelaySpan = targetSpan;
  }

  // Process samples using current delay span
  for (int i = 0; i < samplesLength; i++) {
    uint32_t readIndex = (writeIndex + capacity - currentDelaySpan) % capacity;
    outputBuffer[i] = delayBuffer[readIndex];
    delayBuffer[writeIndex] = inputBuffer[i];
    writeIndex++;
    if (writeIndex == capacity) writeIndex = 0;
  }
}
#endif
