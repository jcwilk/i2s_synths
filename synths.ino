#include <driver/i2s.h>

// Module configuration
#define ENABLE_DELAY 1
#define ENABLE_GATEWAY 1
#define I2S_IS_SLAVE 0  // Set to 1 for slave mode, 0 for master mode

// I2S pin definitions for ESP32-S3-Zero (GPIO 1-13 only)
#define I2S_WS 13 // Word Select (Left/Right Clock) - shared
#define I2S_SD_IN 12 // Serial Data In (ADC data FROM WM8960)
#define I2S_SD_OUT 10 // Serial Data Out (DAC data TO WM8960)
#define I2S_SCK 11 // Serial Clock (Bit Clock) - shared

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 44100
#define BUFFER_LEN 128

int16_t rxBuffer[BUFFER_LEN];
int16_t txBuffer[BUFFER_LEN];

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3-Zero I2S Audio Processing (GPIO 1-13)");

  // Initialize gateway (WM8960 management)
  gatewaySetup();

  // Setup I2S for bidirectional communication
  setupI2S();

  // Initialize audio processing module
  moduleSetup();

  Serial.println("Setup complete. Audio processing active on GPIO 10-13.");
  Serial.println("Audio Input -> ESP32 -> Audio Processing -> ESP32 -> Audio Output");
}

void loop() {
  // Read audio data from WM8960 ADC via I2S
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, rxBuffer, BUFFER_LEN * sizeof(int16_t), &bytesRead, portMAX_DELAY);

  if (result == ESP_OK && bytesRead > 0) {
    int samplesRead = bytesRead / sizeof(int16_t);

    // Process audio through module
    moduleLoop(rxBuffer, txBuffer, samplesRead);

    // Send processed audio data back to WM8960 DAC via I2S
    size_t bytesWritten = 0;
    i2s_write(I2S_PORT, txBuffer, samplesRead * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
  }
}

void setupI2S() {
  // Configure I2S for bidirectional communication (RX + TX)
  const i2s_config_t i2s_config = {
#if I2S_IS_SLAVE
    .mode = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX | I2S_MODE_TX), // Slave mode, both RX and TX
#else
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX), // Master mode, both RX and TX
#endif
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // Stereo
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 4, // Small buffers for low latency
    .dma_buf_len = BUFFER_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0,
    .mclk_multiple = i2s_mclk_multiple_t(I2S_MCLK_MULTIPLE_512),
    .bits_per_chan = i2s_bits_per_chan_t(I2S_BITS_PER_CHAN_DEFAULT)
  };

  // Install I2S driver
  esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (result != ESP_OK) {
    Serial.printf("I2S driver install failed: %d\n", result);
    return;
  }

  // Configure I2S pins
  const i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE, // No master clock needed
    .bck_io_num = I2S_SCK, // Bit clock (shared) - GPIO 11
    .ws_io_num = I2S_WS, // Word select (shared) - GPIO 13
    .data_out_num = I2S_SD_OUT, // Data output (to WM8960 DAC) - GPIO 10
    .data_in_num = I2S_SD_IN // Data input (from WM8960 ADC) - GPIO 12
  };

  result = i2s_set_pin(I2S_PORT, &pin_config);
  if (result != ESP_OK) {
    Serial.printf("I2S set pin failed: %d\n", result);
    return;
  }

#if I2S_IS_SLAVE
  Serial.println("I2S configured as SLAVE for bidirectional communication on GPIO 10-13");
#else
  Serial.println("I2S configured as MASTER for bidirectional communication on GPIO 10-13");
#endif
}