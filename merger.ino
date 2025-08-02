// Merger module for audio processing

#if ACTIVE_MODULE == MODULE_MERGER

// --- Compressor Configuration ---
#define MAX_SIGNAL_LEVEL 24576 // Max signal level before compression (approx. 75% of int16_t max)
#define COMPRESSOR_RELEASE_RATE 0.99995f // Rate at which compression ratio returns to 1.0

// --- Second I2S Interface (I2S_NUM_1) Pinout ---
#define I2S1_WS 2       // Word Select for second input
#define I2S1_SD_IN 1    // Serial Data In for second input
#define I2S1_SCK 3      // Serial Clock for second input
#define I2S_PORT_1 I2S_NUM_1

// Compressor state
float currentScaleRatio = 1.0f;
int16_t rxBuffer2[BUFFER_LEN];


void setupI2S1() {
  // Configure I2S for slave RX
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX), // Slave mode, RX only
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FORMAT,
    .communication_format = I2S_COMM_FORMAT,
    .intr_alloc_flags = I2S_INTR_ALLOC_FLAGS,
    .dma_buf_count = I2S_DMA_BUF_COUNT,
    .dma_buf_len = BUFFER_LEN,
    .use_apll = I2S_USE_APLL,
    .tx_desc_auto_clear = I2S_TX_DESC_AUTO_CLEAR,
    .fixed_mclk = I2S_FIXED_MCLK
  };

  // Install I2S driver for port 1
  i2s_driver_install(I2S_PORT_1, &i2s_config, 0, NULL);

  // Configure I2S pins for port 1
  const i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = I2S_PIN_NO_CHANGE,
    .ws_io_num = I2S_PIN_NO_CHANGE,
    .data_out_num = I2S_PIN_NO_CHANGE, // Not used
    .data_in_num = I2S1_SD_IN
  };

  i2s_set_pin(I2S_PORT_1, &pin_config);
  Serial.println("I2S1 (Slave RX) configured on GPIO 1, 2, 3");
}

void moduleSetup() {
  // Initialize second I2S interface
  setupI2S1();
  Serial.println("Merger module initialized");
}

void moduleLoop(int16_t* inputBuffer, int16_t* outputBuffer, int sampleCount) {
  // Read from second I2S input
  size_t bytesRead2 = 0;
  i2s_read(I2S_PORT_1, rxBuffer2, BUFFER_LEN * sizeof(int16_t), &bytesRead2, 0); // Non-blocking read

  int samplesRead2 = bytesRead2 / sizeof(int16_t);

  for (int i = 0; i < sampleCount; i++) {
    // Merge signals (simple addition)
    int32_t merged_sample = (int32_t)inputBuffer[i];
    if (i < samplesRead2) {
      merged_sample += (int32_t)rxBuffer2[i];
    }

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
