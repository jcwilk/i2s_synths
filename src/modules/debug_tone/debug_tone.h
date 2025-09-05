#ifndef MODULE_DEBUG_TONE_H
#define MODULE_DEBUG_TONE_H

#include <Arduino.h>
#include <math.h>
#include <string.h>
#include "../../config/constants.h"
#include "../../input/pots.h"

#ifndef DEBUG_TONE_FREQ_HZ
#define DEBUG_TONE_FREQ_HZ 100.0f
#endif
#ifndef DEBUG_TONE_AMPLITUDE
#define DEBUG_TONE_AMPLITUDE 1000.0f
#endif

#ifndef DEBUG_TONE_MIN_FREQ_HZ
#define DEBUG_TONE_MIN_FREQ_HZ 200.0f
#endif
#ifndef DEBUG_TONE_MAX_FREQ_HZ
#define DEBUG_TONE_MAX_FREQ_HZ 2000.0f
#endif

// Optional override: define DEBUG_TONE_OVERRIDE_FREQ_HZ (e.g., 440)
// to ignore potentiometers and generate a plain sine wave at that frequency.
// Leave undefined to use pots for frequency and waveshape.
#define DEBUG_TONE_OVERRIDE_FREQ_HZ 440

#ifndef DEBUG_TONE_UPSTREAM
#define DEBUG_TONE_UPSTREAM 1
#endif
#ifndef DEBUG_TONE_DOWNSTREAM
#define DEBUG_TONE_DOWNSTREAM 1
#endif

static float debugTonePhaseUpstream = 0.0f;
static float debugTonePhaseDownstream = 0.0f;
static float debugToneEmaPrimary = 0.0f;
static float debugToneEmaSecondary = 0.0f;

static inline float dtMapPotToFrequency(float pot01) {
  float minF = DEBUG_TONE_MIN_FREQ_HZ;
  float maxF = DEBUG_TONE_MAX_FREQ_HZ;
  float ratio = maxF / minF;
  return minF * powf(ratio, pot01);
}

static inline float dtWaveSquare(float phase01) { return (phase01 < 0.5f) ? 1.0f : -1.0f; }

static inline float dtWaveSawMidrise(float phase01) {
  if (phase01 < 0.5f) return phase01 * 2.0f; else return (phase01 - 1.0f) * 2.0f;
}

static inline float dtWaveSine(float phase01) { return sinf(phase01 * 2.0f * PI); }

static inline float dtSampleBlendedWave(float phase01, float shape01) {
  if (shape01 <= 0.5f) {
    float m = shape01 * 2.0f;
    if (m <= 0.0001f) return dtWaveSquare(phase01);
    if (m >= 0.9999f) return dtWaveSawMidrise(phase01);
    const float half = 0.5f;
    const float rampDuration = half * m;
    if (phase01 < half) {
      if (phase01 < rampDuration) {
        float r = phase01 / rampDuration; return r;
      } else {
        return 1.0f;
      }
    } else {
      float t = phase01 - half;
      if (t < (half - rampDuration)) return -1.0f; else {
        float r = (t - (half - rampDuration)) / rampDuration; return -1.0f + r;
      }
    }
  } else {
    float m = (shape01 - 0.5f) * 2.0f;
    float a = dtWaveSawMidrise(phase01);
    float b = dtWaveSine(phase01);
    return a * (1.0f - m) + b * m;
  }
}

static inline void dtFillWave(int16_t* outputBuffer,
                              int sampleCount,
                              float& phase01,
                              const float phaseIncrement01,
                              const float amplitude,
                              const float shape01) {
  for (int i = 0; i < sampleCount; i++) {
    float sample = dtSampleBlendedWave(phase01, shape01) * amplitude;
    int32_t s = (int32_t)sample;
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    outputBuffer[i] = (int16_t)s;
    phase01 += phaseIncrement01;
    if (phase01 >= 1.0f) phase01 -= 1.0f;
  }
}

static inline void dtProcessDirection(int16_t* inputBuffer,
                                      int16_t* outputBuffer,
                                      int samplesLength,
                                      float& phase01,
                                      bool generateTone) {
  if (!(samplesLength > 0 && outputBuffer)) return;
  float freqHz;
  float shape01;
#ifdef DEBUG_TONE_OVERRIDE_FREQ_HZ
  // Override: plain sine wave at fixed frequency; do not read pots
  freqHz = (float)DEBUG_TONE_OVERRIDE_FREQ_HZ;
  shape01 = 1.0f; // sine
#else
  float potFreq = readPotWithSmoothingAndDeadZone(POT_PIN_PRIMARY, debugToneEmaPrimary);
  freqHz = dtMapPotToFrequency(potFreq);
  shape01 = readPotWithSmoothingAndDeadZone(POT_PIN_SECONDARY, debugToneEmaSecondary);
#endif
  const float phaseIncrement01 = freqHz / (float)SAMPLE_RATE;
  if (generateTone) {
    dtFillWave(outputBuffer, samplesLength, phase01, phaseIncrement01, DEBUG_TONE_AMPLITUDE, shape01);
  } else if (inputBuffer) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

inline void moduleSetup() {
  Serial.println("Debug tone module active (sine generator)");
  debugTonePhaseUpstream = 0.0f;
  debugTonePhaseDownstream = 0.0f;
}

inline void moduleLoopUpstream(int16_t* inputBuffer,
                               int16_t* outputBuffer,
                               int samplesLength) {
  dtProcessDirection(inputBuffer, outputBuffer, samplesLength, debugTonePhaseUpstream, DEBUG_TONE_UPSTREAM);
}

inline void moduleLoopDownstream(int16_t* inputBuffer,
                                 int16_t* outputBuffer,
                                 int samplesLength) {
  dtProcessDirection(inputBuffer, outputBuffer, samplesLength, debugTonePhaseDownstream, DEBUG_TONE_DOWNSTREAM);
}

#endif // MODULE_DEBUG_TONE_H


