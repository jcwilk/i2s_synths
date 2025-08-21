// Merger module for audio processing

#if ACTIVE_MODULE == MODULE_MERGER

// --- Compressor Configuration ---
#define MAX_SIGNAL_LEVEL 24576 // Max signal level before compression (approx. 75% of int16_t max)
#define COMPRESSOR_RELEASE_RATE 0.99995f // Rate at which compression ratio returns to 1.0

// I2S1 is fully configured in synths.ino; no pin config here

// --- Potentiometer Configuration ---
// Pins are defined globally in synths.ino so all modules share the same assignments

// Compressor and Potentiometer state
float currentScaleRatio = 1.0f;
float emaPrimary = 0.0f;
float emaSecondary = 0.0f;
// No local rx buffer; secondary buffers are managed by synths.ino


// No I2S1 setup here; handled in synths.ino

void moduleSetup() {
  // I2S1 already initialized in synths.ino
  // Set ADC attenuation for 0-3.3V range
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  Serial.println("Merger module initialized");
}

void moduleLoopUpstream(int16_t* inputBuffer,
                        int16_t* outputBuffer,
                        int samplesLength) {
  // Upstream: simple passthrough
  if (samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}

void moduleLoopDownstream(int16_t* inputBuffer,
                          int16_t* outputBuffer,
                          int samplesLength) {
  // Read both potentiometers for mix control
  float primaryCoeff = readPotWithSmoothingAndDeadZone(POT_PIN_PRIMARY, emaPrimary);
  float secondaryCoeff = readPotWithSmoothingAndDeadZone(POT_PIN_SECONDARY, emaSecondary);

  // For merger in downstream, assume 'inputBuffer' is the downstream stream to be mixed with
  // upstream content is not provided here; treat secondaryCoeff as additional gain on same stream
  for (int i = 0; i < samplesLength; i++) {
    int32_t merged_sample = (int32_t)(inputBuffer[i] * primaryCoeff) + (int32_t)(inputBuffer[i] * secondaryCoeff);

    if (abs(merged_sample) > MAX_SIGNAL_LEVEL) {
      currentScaleRatio = (float)MAX_SIGNAL_LEVEL / abs(merged_sample);
    } else {
      currentScaleRatio += (1.0f - currentScaleRatio) * (1.0f - COMPRESSOR_RELEASE_RATE);
    }

    merged_sample = (int32_t)(merged_sample * currentScaleRatio);
    if (merged_sample > 32767) merged_sample = 32767;
    if (merged_sample < -32768) merged_sample = -32768;
    outputBuffer[i] = (int16_t)merged_sample;
  }
}

#endif
