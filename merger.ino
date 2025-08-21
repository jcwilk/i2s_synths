// Merger module for audio processing

#if ACTIVE_MODULE == MODULE_MERGER

// --- Compressor Configuration ---
#define MAX_SIGNAL_LEVEL 24576 // Max signal level before compression (approx. 75% of int16_t max)
#define COMPRESSOR_RELEASE_RATE 0.99995f // Rate at which compression ratio returns to 1.0

// I2S1 is fully configured in synths.ino; no pin config here

// --- Potentiometer Configuration ---
// Pins are defined globally in synths.ino so all modules share the same assignments
#define ADC_MAX 4095
#define EMA_ALPHA 0.1f // Exponential Moving Average alpha (smoothing factor)
#define DEAD_ZONE_LOW 0.02f // 2% dead zone
#define DEAD_ZONE_HIGH 0.98f // 98% dead zone

// Compressor and Potentiometer state
float currentScaleRatio = 1.0f;
float emaPrimary = 0.0f;
float emaSecondary = 0.0f;
// No local rx buffer; secondary buffers are managed by synths.ino


// No I2S1 setup here; handled in synths.ino

static float readPotWithSmoothingAndDeadZone(int pin, float& emaState) {
  float raw = (float)analogRead(pin) / ADC_MAX;
  emaState = (EMA_ALPHA * raw) + ((1.0f - EMA_ALPHA) * emaState);
  Serial.println("Pin" + String(pin) + " Pot value: " + String(raw)+ " EMA: " + String(emaState));

  if (emaState < DEAD_ZONE_LOW) {
    return 0.0f;
  }
  if (emaState > DEAD_ZONE_HIGH) {
    return 1.0f;
  }
  return (emaState - DEAD_ZONE_LOW) / (DEAD_ZONE_HIGH - DEAD_ZONE_LOW);
}

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
