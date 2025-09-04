// Debug tone module (separate file)

#if ACTIVE_MODULE == MODULE_DEBUG_TONE

#ifndef DEBUG_TONE_FREQ_HZ
#define DEBUG_TONE_FREQ_HZ 100.0f
#endif
#ifndef DEBUG_TONE_AMPLITUDE
#define DEBUG_TONE_AMPLITUDE 1000.0f
#endif


// Frequency range controlled by the primary potentiometer
#include "src/input/pots.h"
#ifndef DEBUG_TONE_MIN_FREQ_HZ
#define DEBUG_TONE_MIN_FREQ_HZ 200.0f
#endif
#ifndef DEBUG_TONE_MAX_FREQ_HZ
#define DEBUG_TONE_MAX_FREQ_HZ 2000.0f
#endif

// Per-direction toggle: 1 = generate tone, 0 = passthrough input
#ifndef DEBUG_TONE_UPSTREAM
#define DEBUG_TONE_UPSTREAM 0
#endif
#ifndef DEBUG_TONE_DOWNSTREAM
#define DEBUG_TONE_DOWNSTREAM 1
#endif

// Phase is tracked normalized in [0.0, 1.0)
static float debugTonePhaseUpstream = 0.0f;
static float debugTonePhaseDownstream = 0.0f;

static float emaPrimary = 0.0f;
static float emaSecondary = 0.0f;

static inline float mapPotToFrequency(float pot01) {
  // Exponential mapping for more natural control
  float minF = DEBUG_TONE_MIN_FREQ_HZ;
  float maxF = DEBUG_TONE_MAX_FREQ_HZ;
  float ratio = maxF / minF;
  return minF * powf(ratio, pot01);
}

static inline float waveSquare(float phase01) {
  return (phase01 < 0.5f) ? 1.0f : -1.0f;
}

static inline float waveSawMidrise(float phase01) {
  if (phase01 < 0.5f) {
    return phase01 * 2.0f;            // 0 .. +1
  } else {
    return (phase01 - 1.0f) * 2.0f;   // -1 .. 0
  }
}

static inline float waveSine(float phase01) {
  return sinf(phase01 * 2.0f * PI);
}

static inline float sampleBlendedWave(float phase01, float shape01) {
  // shape01: 0.0 → square, 0.5 → saw (midrise), 1.0 → sine
  if (shape01 <= 0.5f) {
    // Custom morph between square and midrise saw with controlled ramp+plateau
    // m = 0 → square (instant to +1, hold half cycle, instant to -1, hold)
    // m = 1 → midrise saw (rise 0→+1 over first half, jump to -1, rise -1→0 over second half)
    float m = shape01 * 2.0f; // [0..1]
    if (m <= 0.0001f) {
      return waveSquare(phase01);
    }
    if (m >= 0.9999f) {
      return waveSawMidrise(phase01);
    }

    const float half = 0.5f;
    const float rampDuration = half * m; // portion of each half-cycle spent ramping

    if (phase01 < half) {
      // First half: ramp from 0 to +1 over rampDuration, then plateau at +1
      if (phase01 < rampDuration) {
        float r = phase01 / rampDuration; // 0..1
        return r; // 0..+1
      } else {
        return 1.0f;
      }
    } else {
      // Second half: plateau at -1, then ramp up to 0 over last rampDuration
      float t = phase01 - half; // 0..0.5
      if (t < (half - rampDuration)) {
        return -1.0f;
      } else {
        float r = (t - (half - rampDuration)) / rampDuration; // 0..1
        return -1.0f + r; // -1..0
      }
    }
  } else {
    float m = (shape01 - 0.5f) * 2.0f; // 0..1 between saw and sine
    float a = waveSawMidrise(phase01);
    float b = waveSine(phase01);
    return a * (1.0f - m) + b * m;
  }
}

static inline void fillWave(int16_t* outputBuffer,
                            int sampleCount,
                            float& phase01,
                            const float phaseIncrement01,
                            const float amplitude,
                            const float shape01) {
  for (int i = 0; i < sampleCount; i++) {
    float sample = sampleBlendedWave(phase01, shape01) * amplitude;
    int32_t s = (int32_t)sample;
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    outputBuffer[i] = (int16_t)s;

    phase01 += phaseIncrement01;
    if (phase01 >= 1.0f) phase01 -= 1.0f;
  }
}

static inline void processDebugToneDirection(int16_t* inputBuffer,
                                             int16_t* outputBuffer,
                                             int samplesLength,
                                             float& phase01,
                                             bool generateTone) {
  if (!(samplesLength > 0 && outputBuffer)) {
    return;
  }

  float potFreq = readPotWithSmoothingAndDeadZone(POT_PIN_PRIMARY, emaPrimary);
  float freqHz = mapPotToFrequency(potFreq);
  float shape01 = readPotWithSmoothingAndDeadZone(POT_PIN_SECONDARY, emaSecondary);
  const float phaseIncrement01 = freqHz / (float)SAMPLE_RATE;

  if (generateTone) {
    fillWave(outputBuffer, samplesLength, phase01, phaseIncrement01, DEBUG_TONE_AMPLITUDE, shape01);
  } else if (inputBuffer) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

void moduleSetup() {
  Serial.println("Debug tone module active (sine generator)");
  debugTonePhaseUpstream = 0.0f;
  debugTonePhaseDownstream = 0.0f;
}

void moduleLoopUpstream(int16_t* inputBuffer,
                        int16_t* outputBuffer,
                        int samplesLength) {
  processDebugToneDirection(inputBuffer, outputBuffer, samplesLength, debugTonePhaseUpstream, DEBUG_TONE_UPSTREAM);
}

void moduleLoopDownstream(int16_t* inputBuffer,
                          int16_t* outputBuffer,
                          int samplesLength) {
  processDebugToneDirection(inputBuffer, outputBuffer, samplesLength, debugTonePhaseDownstream, DEBUG_TONE_DOWNSTREAM);
}

#endif




