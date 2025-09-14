#ifndef INPUT_POTS_H
#define INPUT_POTS_H

#include <Arduino.h>
#include <math.h>

#ifndef ADC_MAX
#define ADC_MAX 4095
#endif

// Base EMA constant per millisecond (used to build a time-invariant alpha)
#ifndef EMA_ALPHA
#define EMA_ALPHA 0.008f
#endif

#ifndef DEAD_ZONE_LOW
#define DEAD_ZONE_LOW 0.02f
#endif
#ifndef DEAD_ZONE_HIGH
#define DEAD_ZONE_HIGH 0.98f
#endif

// Single potentiometer state
typedef struct {
  int pin;
  float raw;       // latest raw sample in [0,1]
  float smoothed;  // EMA-smoothed value in [0,1]
} PotState;

// Convenience container for two pots used across the project
typedef struct {
  PotState primary;
  PotState secondary;
} DualPotsState;


// Compute effective alpha for a given deltaMs using time-invariant EMA
inline float potsEffectiveAlpha(unsigned long deltaMs) {
  if (deltaMs == 0) return 0.0f;
  float oneMinusBase = 1.0f - EMA_ALPHA;
  float powTerm = 1.0f;
  // powf is safe but a bit heavy; still fine for ms-scale intervals
  powTerm = powf(oneMinusBase, (float)deltaMs);
  float alpha = 1.0f - powTerm;
  if (alpha < 0.0f) alpha = 0.0f;
  if (alpha > 1.0f) alpha = 1.0f;
  return alpha;
}

inline float potsReadAnalog01(int pin) {
  int v = analogRead(pin);
  if (v < 0) v = 0;
  if (v > ADC_MAX) v = ADC_MAX;
  return (float)v / (float)ADC_MAX;
}

// Update a single PotState using time-scaled EMA
inline PotState potUpdate(PotState state, unsigned long deltaMs) {
  float raw01 = potsReadAnalog01(state.pin);
  float a = potsEffectiveAlpha(deltaMs);
  state.raw = raw01;
  state.smoothed = (a * raw01) + ((1.0f - a) * state.smoothed);
  return state;
}

// Update both pots and return the new state
inline DualPotsState potsUpdate(DualPotsState state, unsigned long deltaMs) {
  state.primary = potUpdate(state.primary, deltaMs);
  state.secondary = potUpdate(state.secondary, deltaMs);
  return state;
}

// Initializer helpers
inline PotState potMakeInitial(int pin) {
  PotState s;
  s.pin = pin;
  s.raw = 0.0f;
  s.smoothed = 0.0f;
  return s;
}

inline DualPotsState potsMakeInitial(int primaryPin, int secondaryPin) {
  DualPotsState s;
  s.primary = potMakeInitial(primaryPin);
  s.secondary = potMakeInitial(secondaryPin);
  return s;
}

// --- Mapping helpers (apply dead-zones and tapers) ---

inline float potsMapDeadzone01(float v, float low = DEAD_ZONE_LOW, float high = DEAD_ZONE_HIGH) {
  if (v <= low) return 0.0f;
  if (v >= high) return 1.0f;
  return (v - low) / (high - low);
}

// Linear mapping using smoothed value
inline float potLinear01(const PotState& s, float low = DEAD_ZONE_LOW, float high = DEAD_ZONE_HIGH) {
  return potsMapDeadzone01(s.smoothed, low, high);
}

// Low-biased curve (more resolution near 0)
inline float potSquared01(const PotState& s, float low = DEAD_ZONE_LOW, float high = DEAD_ZONE_HIGH) {
  float x = potLinear01(s, low, high);
  return x * x;
}

// High-biased curve (more resolution near 1)
inline float potSqrt01(const PotState& s, float low = DEAD_ZONE_LOW, float high = DEAD_ZONE_HIGH) {
  float x = potLinear01(s, low, high);
  return sqrtf(x);
}

// Convenience accessors for dual pots
inline float potsPrimaryLinear(const DualPotsState& s) { return potLinear01(s.primary); }
inline float potsSecondaryLinear(const DualPotsState& s) { return potLinear01(s.secondary); }
inline float potsPrimarySquared(const DualPotsState& s) { return potSquared01(s.primary); }
inline float potsSecondarySquared(const DualPotsState& s) { return potSquared01(s.secondary); }
inline float potsPrimarySqrt(const DualPotsState& s) { return potSqrt01(s.primary); }
inline float potsSecondarySqrt(const DualPotsState& s) { return potSqrt01(s.secondary); }

#endif // INPUT_POTS_H


