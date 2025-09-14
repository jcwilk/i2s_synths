#ifndef MODULE_DELAY_H
#define MODULE_DELAY_H

#include <Arduino.h>
#include <math.h>
#include <string.h>
#include "../config/constants.h"
#include "../input/pots.h"

// Delay buffer configuration
#ifndef DELAY_SAMPLES
#define DELAY_SAMPLES (SAMPLE_RATE * 1)
#endif
#ifndef MIN_DELAY_RATIO
#define MIN_DELAY_RATIO 0.10f
#endif
#ifndef DELAY_DEADBAND
#define DELAY_DEADBAND 0.05f
#endif
#ifndef DELAY_FADE_MS
#define DELAY_FADE_MS 15
#endif
#ifndef MAX_FADE_FRAMES
#define MAX_FADE_FRAMES ((SAMPLE_RATE * DELAY_FADE_MS) / 1000 + 1)
#endif

static int16_t delayBuffer[DELAY_SAMPLES * 2];
static uint32_t writeIndex = 0;
static uint32_t currentDelaySpan = DELAY_SAMPLES * 2;
static float lastAppliedPot01 = -1.0f;
static int16_t fadeTemp[2 * MAX_FADE_FRAMES];

static inline uint32_t ensureEvenAtLeastTwo(uint32_t value) {
  if (value < 2u) return 2u;
  return (value & 1u) ? (value - 1u) : value;
}
static inline uint32_t addWrap(uint32_t index, uint32_t amount, uint32_t capacity) {
  index += amount; if (index >= capacity) index -= capacity; return index;
}
static inline uint32_t subWrap(uint32_t index, uint32_t amount, uint32_t capacity) {
  if (index >= amount) return index - amount; return capacity - (amount - index);
}

inline void moduleSetup() {
  memset(delayBuffer, 0, sizeof(delayBuffer));
  writeIndex = 0;
  currentDelaySpan = DELAY_SAMPLES * 2;
  Serial.println("Delay module initialized: variable delay using primary pot");
}

inline void moduleLoopUpstream(int16_t* inputBuffer,
                               int16_t* outputBuffer,
                               int samplesLength,
                               DualPotsState /*pots_state*/) {
  if (samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

inline void moduleLoopDownstream(int16_t* inputBuffer,
                                 int16_t* outputBuffer,
                                 int samplesLength,
                                 DualPotsState pots_state) {
  if (samplesLength <= 0) return;
  const uint32_t capacity = (uint32_t)DELAY_SAMPLES * 2u;

  float pot01Raw = potsPrimaryLinear(pots_state);
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
    targetSpan = capacity - (capacity & 1u ? 1u : 0u);
  }

  if (targetSpan != currentDelaySpan) {
    uint32_t readIndexStable = (writeIndex + capacity - currentDelaySpan) % capacity;
    uint32_t newWriteIndex = (readIndexStable + targetSpan) % capacity;

    if (targetSpan > currentDelaySpan) {
      uint32_t gapSamples = (newWriteIndex >= writeIndex) ? (newWriteIndex - writeIndex) : (capacity - writeIndex + newWriteIndex);
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
      uint32_t deltaSamples = currentDelaySpan - targetSpan;
      uint32_t shiftFrames = (deltaSamples / 2u);
      uint32_t fadeFrames = shiftFrames < (uint32_t)MAX_FADE_FRAMES ? shiftFrames : (uint32_t)MAX_FADE_FRAMES;
      if (fadeFrames > 0) {
        uint32_t srcStart = subWrap(writeIndex, fadeFrames * 2u, capacity);
        uint32_t destStart = subWrap(newWriteIndex, fadeFrames * 2u, capacity);
        for (uint32_t f = 0; f < fadeFrames; f++) {
          uint32_t s = addWrap(srcStart, f * 2u, capacity);
          fadeTemp[f * 2u] = delayBuffer[s];
          fadeTemp[f * 2u + 1u] = delayBuffer[s + 1u];
        }
        for (uint32_t f = 0; f < fadeFrames; f++) {
          float t = (float)(f + 1u) / (float)fadeFrames;
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

  for (int i = 0; i < samplesLength; i++) {
    uint32_t readIndex = (writeIndex + capacity - currentDelaySpan) % capacity;
    outputBuffer[i] = delayBuffer[readIndex];
    delayBuffer[writeIndex] = inputBuffer[i];
    writeIndex++;
    if (writeIndex == capacity) writeIndex = 0;
  }
}

#endif // MODULE_DELAY_H


