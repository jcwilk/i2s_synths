// Cutoff module: downstream variable LP/HP filter with intensity centered at 50%

#if ACTIVE_MODULE == MODULE_CUTOFF

#include <math.h>

// This module uses a topology-preserving transform (TPT) state-variable filter
// (aka Chamberlin/Zurich style) for numerical stability across cutoff sweeps
// and intensity morphing. It is efficient and well-suited for real-time pot
// adjustments.

// Pot smoothing state
static float emaPrimary = 0.0f;
static float emaSecondary = 0.0f;

// Cutoff frequency range (Hz)
static const float CUTOFF_MIN_HZ = 200.0f;
static const float CUTOFF_MAX_HZ = 2000.0f;

// Filter state per direction (downstream only uses it)
typedef struct {
  float ic1eq;  // integrator state 1
  float ic2eq;  // integrator state 2
  float g;      // precomputed tan(pi * fc/fs)
  float k;      // damping (resonance control), fixed moderate value
} TPTSVF;

static TPTSVF dsFilter;

static inline void tptInit() {
  dsFilter.ic1eq = 0.0f;
  dsFilter.ic2eq = 0.0f;
  dsFilter.g = 0.0f;
  dsFilter.k = 0.7f; // light damping to avoid ringing; not user-controlled here
}

static inline void tptSetCutoff(float cutoffHz) {
  // Clamp cutoff to a practical range
  if (cutoffHz < 20.0f) cutoffHz = 20.0f;
  if (cutoffHz > (SAMPLE_RATE * 0.45f)) cutoffHz = SAMPLE_RATE * 0.45f;
  float w = (float)PI * (cutoffHz / (float)SAMPLE_RATE);
  // Use tan(w) for bilinear transform pre-warping
  dsFilter.g = tanf(w);
}

static inline void tptProcessBuffer(const int16_t* in,
                                    int16_t* out,
                                    int n,
                                    float intensitySigned) {
  // Stable TPT SVF difference equations
  // Reference formulation (Vadim Zavalishin):
  // v0 = x[n]
  // v1 = (v0 - ic2eq - k*ic1eq) / (1 + k*g + g*g)
  // v2 = ic1eq + g*v1
  // v3 = ic2eq + g*v2
  // ic1eq = v2 + g*v1
  // ic2eq = v3 + g*v2
  // Outputs: low = v3, band = v2, high = v0 - k*v2 - v3

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
      // Towards high-pass: y = (1-a)*dry + a*HP
      float aHP = intensitySigned * intensitySigned;
      y = (1.0f - aHP) * v0 + aHP * high;
    } else {
      // Towards low-pass: y = (1-b)*dry + b*LP
      float bLP = intensitySigned * intensitySigned; // intensitySigned is negative; square yields 0..1
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
  // Exponential mapping CUTOFF_MIN_HZ .. CUTOFF_MAX_HZ
  float ratio = CUTOFF_MAX_HZ / CUTOFF_MIN_HZ;
  return CUTOFF_MIN_HZ * powf(ratio, pot01);
}

static inline float mapPotSecondaryToBlend(float pot01) {
  // Convert [0..1] with 0.5 as no-op to signed intensity [-1..+1]
  // 0.0 -> -1 (full LP), 0.5 -> 0 (passthrough), 1.0 -> +1 (full HP)
  return (pot01 - 0.5f) * 2.0f;
}

void moduleSetup() {
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
  tptInit();
  tptSetCutoff(1000.0f); // sensible default
  Serial.println("Cutoff module initialized (LP/HP, intensity centered at 50%)");
}

void moduleLoopUpstream(int16_t* inputBuffer,
                        int16_t* outputBuffer,
                        int samplesLength) {
  // Upstream: strict passthrough
  if (samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

void moduleLoopDownstream(int16_t* inputBuffer,
                          int16_t* outputBuffer,
                          int samplesLength) {
  if (!(samplesLength > 0)) return;

  float pCut = readPotWithSmoothingAndDeadZone(POT_PIN_PRIMARY, emaPrimary);
  float pInt = readPotWithSmoothingAndDeadZone(POT_PIN_SECONDARY, emaSecondary);

  // Update cutoff continuously
  float cutoffHz = mapPotPrimaryToCutoff(pCut);
  tptSetCutoff(cutoffHz);

  // Compute signed intensity [-1..+1]
  float intensity = mapPotSecondaryToBlend(pInt);

  tptProcessBuffer(inputBuffer, outputBuffer, samplesLength, intensity);
}

#endif


