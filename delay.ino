// Delay module for audio processing

#if ACTIVE_MODULE == MODULE_DELAY
// Delay buffer configuration
#define DELAY_SAMPLES (SAMPLE_RATE * 1) // 1 second delay at 44.1kHz

// Circular buffer for 1-second delay
int16_t delayBuffer[DELAY_SAMPLES * 2]; // Stereo samples (L+R)
uint32_t delayIndex = 0;

void moduleSetup() {
  // Initialize delay buffer to silence
  memset(delayBuffer, 0, sizeof(delayBuffer));
  Serial.println("Delay module initialized: 1-second delay active");
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
  // Process downstream samples through the delay buffer
  for (int i = 0; i < samplesLength; i++) {
    delayBuffer[delayIndex] = inputBuffer[i];
    uint32_t delayedIndex = (delayIndex + 1) % (DELAY_SAMPLES * 2);
    outputBuffer[i] = delayBuffer[delayedIndex];
    delayIndex = (delayIndex + 1) % (DELAY_SAMPLES * 2);
  }
}
#endif
