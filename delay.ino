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

void moduleLoop(int16_t* inputBuffer,
                int16_t* outputBuffer,
                int16_t* inputBuffer2,
                int16_t* outputBuffer2,
                int samplesLength) {
  // Forward secondary input to secondary output unchanged
  if (samplesLength > 0) {
    memcpy(outputBuffer2, inputBuffer2, samplesLength * sizeof(int16_t));
  }

  // Process primary samples through the delay buffer
  for (int i = 0; i < samplesLength; i++) {
    // Store current input sample in delay buffer
    delayBuffer[delayIndex] = inputBuffer[i];

    // Get delayed sample from buffer (1 second ago)
    uint32_t delayedIndex = (delayIndex + 1) % (DELAY_SAMPLES * 2);
    outputBuffer[i] = delayBuffer[delayedIndex];

    // Advance delay buffer index
    delayIndex = (delayIndex + 1) % (DELAY_SAMPLES * 2);
  }
}
#endif
