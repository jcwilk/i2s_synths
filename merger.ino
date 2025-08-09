// Merger module for audio processing

#if ACTIVE_MODULE == MODULE_MERGER

// --- Compressor Configuration ---
#define MAX_SIGNAL_LEVEL 24576 // Max signal level before compression (approx. 75% of int16_t max)
#define COMPRESSOR_RELEASE_RATE 0.99995f // Rate at which compression ratio returns to 1.0

// I2S1 is fully configured in synths.ino; no pin config here

// --- Potentiometer Configuration ---
#define POT_PIN 2
#define ADC_MAX 4095
#define EMA_ALPHA 0.1f // Exponential Moving Average alpha (smoothing factor)
#define DEAD_ZONE_LOW 0.02f // 2% dead zone
#define DEAD_ZONE_HIGH 0.98f // 98% dead zone

// Compressor and Potentiometer state
float currentScaleRatio = 1.0f;
float smoothedMix = 0.0f;
// No local rx buffer; secondary buffers are managed by synths.ino


// No I2S1 setup here; handled in synths.ino

void moduleSetup() {
  // I2S1 already initialized in synths.ino
  // Set ADC attenuation for 0-3.3V range
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  Serial.println("Merger module initialized");
}

void moduleLoop(int16_t* inputBuffer,
                int16_t* outputBuffer,
                int sampleCount,
                int16_t* inputBuffer2,
                int16_t* outputBuffer2,
                int sampleCount2) {

  // Read raw potentiometer value
  float rawMix = (float)analogRead(POT_PIN) / ADC_MAX;
  
  // Apply Exponential Moving Average
  smoothedMix = (EMA_ALPHA * rawMix) + ((1.0f - EMA_ALPHA) * smoothedMix);

  // Apply dead zones and rescale
  float mix;
  if (smoothedMix < DEAD_ZONE_LOW) {
    mix = 0.0f;
  } else if (smoothedMix > DEAD_ZONE_HIGH) {
    mix = 1.0f;
  } else {
    // Rescale the value from the dead zone to the full 0-1 range
    mix = (smoothedMix - DEAD_ZONE_LOW) / (DEAD_ZONE_HIGH - DEAD_ZONE_LOW);
  }

  // Calculate mix coefficients based on pot position
  float primaryCoeff, secondaryCoeff;
  if (mix < 0.5f) {
    // 50% to 0% pot: Primary is 100%, Secondary fades out
    primaryCoeff = mix * 2.0f;
    secondaryCoeff = 0.0f;
  } else {
    // 50% to 100% pot: Secondary is 100%, Primary fades out
    primaryCoeff = 2.0f - 2.0f * mix;
    secondaryCoeff = 2.0f * mix - 1.0f;
  }
  
  //Serial.println("Mixing ratio: " + String(mix));

  // Simple forwarding for secondary by default (up to available samples)
  for (int i = 0; i < sampleCount2; i++) {
    outputBuffer2[i] = inputBuffer2[i];
  }

  for (int i = 0; i < sampleCount; i++) {
    int32_t primarySample = (int32_t)(inputBuffer[i] * primaryCoeff);
    int32_t secondarySample = 0;

    if (i < sampleCount2) {
      secondarySample = (int32_t)(inputBuffer2[i] * secondaryCoeff);
    }
    
    int32_t merged_sample = primarySample + secondarySample;

    // Apply compressor
    if (abs(merged_sample) > MAX_SIGNAL_LEVEL) {
      // If signal exceeds threshold, calculate new scale ratio
      currentScaleRatio = (float)MAX_SIGNAL_LEVEL / abs(merged_sample);
    } else {
      // Gradually release compression
      currentScaleRatio += (1.0f - currentScaleRatio) * (1.0f - COMPRESSOR_RELEASE_RATE);
    }

    // Apply scaling
    merged_sample = (int32_t)(merged_sample * currentScaleRatio);

    // Clip to int16_t range
    if (merged_sample > 32767) merged_sample = 32767;
    if (merged_sample < -32768) merged_sample = -32768;

    outputBuffer[i] = (int16_t)merged_sample;
  }
}

#endif
