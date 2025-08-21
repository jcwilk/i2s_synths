// Debug tone module (separate file)

#if ACTIVE_MODULE == MODULE_DEBUG_TONE

#ifndef DEBUG_TONE_FREQ_HZ
#define DEBUG_TONE_FREQ_HZ 100.0f
#endif
#ifndef DEBUG_TONE_AMPLITUDE
#define DEBUG_TONE_AMPLITUDE 1000.0f
#endif

// Per-direction toggle: 1 = generate tone, 0 = passthrough input
#ifndef DEBUG_TONE_UPSTREAM
#define DEBUG_TONE_UPSTREAM 1
#endif
#ifndef DEBUG_TONE_DOWNSTREAM
#define DEBUG_TONE_DOWNSTREAM 1
#endif

static float debugTonePhaseUpstream = 0.0f;
static float debugTonePhaseDownstream = 0.0f;

static inline void fillSine(int16_t* outputBuffer,
                            int sampleCount,
                            float& phase,
                            const float phaseIncrement,
                            const float amplitude) {
  for (int i = 0; i < sampleCount; i++) {
    float sample = sinf(phase) * amplitude;
    int32_t s = (int32_t)sample;
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    outputBuffer[i] = (int16_t)s;

    phase += phaseIncrement;
    if (phase >= 2.0f * PI) {
      phase -= 2.0f * PI;
    }
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
  const float phaseIncrement = 2.0f * PI * DEBUG_TONE_FREQ_HZ / (float)SAMPLE_RATE;
  if (samplesLength > 0 && outputBuffer) {
#if DEBUG_TONE_UPSTREAM
    fillSine(outputBuffer, samplesLength, debugTonePhaseUpstream, phaseIncrement, DEBUG_TONE_AMPLITUDE);
#else
    if (inputBuffer) {
      memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
    }
#endif
  }
}

void moduleLoopDownstream(int16_t* inputBuffer,
                          int16_t* outputBuffer,
                          int samplesLength) {
  const float phaseIncrement = 2.0f * PI * DEBUG_TONE_FREQ_HZ / (float)SAMPLE_RATE;
  if (samplesLength > 0 && outputBuffer) {
#if DEBUG_TONE_DOWNSTREAM
    fillSine(outputBuffer, samplesLength, debugTonePhaseDownstream, phaseIncrement, DEBUG_TONE_AMPLITUDE);
#else
    if (inputBuffer) {
      memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
    }
#endif
  }
}

#endif




