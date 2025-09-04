#ifndef MODULE_CUTOFF_H
#define MODULE_CUTOFF_H

#include <Arduino.h>
#include <math.h>
#include <string.h>
#include "../../config/constants.h"
#include "../../input/pots.h"

static float cutoffEmaPrimary = 0.0f;
static float cutoffEmaSecondary = 0.0f;

static const float CUTOFF_MIN_HZ = 200.0f;
static const float CUTOFF_MAX_HZ = 2000.0f;

typedef struct {
  float ic1eq;
  float ic2eq;
  float g;
  float k;
} TPTSVF;

static TPTSVF dsFilter;

static inline void tptInit() {
  dsFilter.ic1eq = 0.0f;
  dsFilter.ic2eq = 0.0f;
  dsFilter.g = 0.0f;
  dsFilter.k = 0.7f;
}

static inline void tptSetCutoff(float cutoffHz) {
  if (cutoffHz < 20.0f) cutoffHz = 20.0f;
  if (cutoffHz > (SAMPLE_RATE * 0.45f)) cutoffHz = SAMPLE_RATE * 0.45f;
  float w = (float)PI * (cutoffHz / (float)SAMPLE_RATE);
  dsFilter.g = tanf(w);
}

static inline void tptProcessBuffer(const int16_t* in,
                                    int16_t* out,
                                    int n,
                                    float intensitySigned) {
  const float g = dsFilter.g;
  const float k = dsFilter.k;
  float ic1eq = dsFilter.ic1eq;
  float ic2eq = dsFilter.ic2eq;
  const float a = 1.0f / (1.0f + k * g + g * g);

  for (int i = 0; i < n; i++) {
    float v0 = (float)in[i];
    float v1 = (v0 - ic2eq - k * ic1eq) * a;
    float v2 = ic1eq + g * v1;
    float v3 = ic2eq + g * v2;
    ic1eq = v2 + g * v1;
    ic2eq = v3 + g * v2;
    float low = v3;
    float high = v0 - k * v2 - v3;
    float y;
    if (intensitySigned >= 0.0f) {
      float aHP = intensitySigned * intensitySigned;
      y = (1.0f - aHP) * v0 + aHP * high;
    } else {
      float bLP = intensitySigned * intensitySigned;
      y = (1.0f - bLP) * v0 + bLP * low;
    }
    int32_t s = (int32_t)roundf(y);
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    out[i] = (int16_t)s;
  }

  dsFilter.ic1eq = ic1eq;
  dsFilter.ic2eq = ic2eq;
}

static inline float mapPotPrimaryToCutoff(float pot01) {
  float ratio = CUTOFF_MAX_HZ / CUTOFF_MIN_HZ;
  return CUTOFF_MIN_HZ * powf(ratio, pot01);
}
static inline float mapPotSecondaryToBlend(float pot01) {
  return (pot01 - 0.5f) * 2.0f;
}

inline void moduleSetup() {
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
  tptInit();
  tptSetCutoff(1000.0f);
  Serial.println("Cutoff module initialized (LP/HP, intensity centered at 50%)");
}

inline void moduleLoopUpstream(int16_t* inputBuffer,
                               int16_t* outputBuffer,
                               int samplesLength) {
  if (samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

inline void moduleLoopDownstream(int16_t* inputBuffer,
                                 int16_t* outputBuffer,
                                 int samplesLength) {
  if (!(samplesLength > 0)) return;
  float pCut = readPotWithSmoothingAndDeadZone(POT_PIN_PRIMARY, cutoffEmaPrimary);
  float pInt = readPotWithSmoothingAndDeadZone(POT_PIN_SECONDARY, cutoffEmaSecondary);
  float cutoffHz = mapPotPrimaryToCutoff(pCut);
  tptSetCutoff(cutoffHz);
  float intensity = mapPotSecondaryToBlend(pInt);
  tptProcessBuffer(inputBuffer, outputBuffer, samplesLength, intensity);
}

#endif // MODULE_CUTOFF_H


