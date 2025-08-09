#include <driver/i2s.h>

// Module selection
#define MODULE_PASSTHROUGH 0
#define MODULE_DELAY 1
#define MODULE_MERGER 2
#define ACTIVE_MODULE MODULE_MERGER

#define ENABLE_GATEWAY 1
#define I2S_IS_SLAVE 0  // Set to 1 for slave mode, 0 for master mode

// I2S pin definitions for ESP32-S3-Zero (GPIO 1-13 only)
#define I2S_WS 13 // Word Select (Left/Right Clock) - shared
#define I2S_SD_IN 12 // Serial Data In (ADC data FROM WM8960)
#define I2S_SD_OUT 10 // Serial Data Out (DAC data TO WM8960)
#define I2S_SCK 11 // Serial Clock (Bit Clock) - shared

#define I2S_PORT I2S_NUM_0

// Secondary I2S (I2S_NUM_1) pin definitions
#define I2S1_WS 7
#define I2S1_SD_IN 9
#define I2S1_SD_OUT 4
#define I2S1_SCK 8
#define I2S_PORT_1 I2S_NUM_1

#define SAMPLE_RATE 44100
#define BUFFER_LEN 128
#define I2S_DMA_BUF_COUNT 4
#define I2S_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define I2S_CHANNEL_FORMAT I2S_CHANNEL_FMT_RIGHT_LEFT
#define I2S_COMM_FORMAT I2S_COMM_FORMAT_STAND_I2S
#define I2S_INTR_ALLOC_FLAGS 0
#define I2S_USE_APLL false
#define I2S_TX_DESC_AUTO_CLEAR false
#define I2S_FIXED_MCLK 0
#define I2S_MCLK_MULTIPLE I2S_MCLK_MULTIPLE_512
#define I2S_BITS_PER_CHAN I2S_BITS_PER_CHAN_DEFAULT

int16_t rxBuffer[BUFFER_LEN];
int16_t txBuffer[BUFFER_LEN];
int16_t rxBuffer2[BUFFER_LEN];
int16_t txBuffer2[BUFFER_LEN];

// Error indication via onboard NeoPixel (21): blocks and blinks red at 1 Hz
#define STATUS_NEOPIXEL_PIN 21
#define STATUS_NEOPIXEL_BRIGHTNESS 25  // ~1/10 of full brightness

// Many ESP32 boards wire WS2812 as GRB. Provide a helper to avoid channel confusion.
static inline void setNeoPixelColor(uint8_t r, uint8_t g, uint8_t b) {
  // Map RGB -> GRB for neopixelWrite
  neopixelWrite(STATUS_NEOPIXEL_PIN, g, r, b);
}
void reportError(const char* message) {
  Serial.printf("ERROR: %s\n", message);
  bool ledOn = false;
  unsigned long lastToggleMs = 0;
  for (;;) {
    unsigned long now = millis();
    if (now - lastToggleMs >= 500) { // toggle every 500ms -> 1 Hz blink
      lastToggleMs = now;
      ledOn = !ledOn;
      if (ledOn) {
        setNeoPixelColor(STATUS_NEOPIXEL_BRIGHTNESS, 0, 0); // dim red
      } else {
        setNeoPixelColor(0, 0, 0); // off
      }
    }
    delay(10);
  }
}

#if ACTIVE_MODULE == MODULE_PASSTHROUGH
void moduleSetup() {
  Serial.println("Passthrough module active");
}
void moduleLoop(int16_t* inputBuffer,
                int16_t* outputBuffer,
                int sampleCount,
                int16_t* inputBuffer2,
                int16_t* outputBuffer2,
                int sampleCount2) {
  if (sampleCount > 0) {
    memcpy(outputBuffer, inputBuffer, sampleCount * sizeof(int16_t));
  }
  if (sampleCount2 > 0) {
    memcpy(outputBuffer2, inputBuffer2, sampleCount2 * sizeof(int16_t));
  }
}
#endif

unsigned long startup_time = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3-Zero I2S Audio Processing (GPIO 1-13)");

  // Initialize gateway (WM8960 management)
  gatewaySetup();

  // Setup I2S for bidirectional communication
  setupI2S();

  // Setup secondary I2S interface (RX + TX as slave)
  setupI2S1();

  // Initialize audio processing module
  moduleSetup();

  Serial.println("Setup complete. Audio processing active on GPIO 10-13.");
  Serial.println("Audio Input -> ESP32 -> Audio Processing -> ESP32 -> Audio Output");
  startup_time = millis();
  // NeoPixel off at start
  setNeoPixelColor(0, 0, 0);
}

void loop() {
  // Normal audio loop; reportError() will lock and blink if called
  // Read audio data from WM8960 ADC via I2S
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, rxBuffer, BUFFER_LEN * sizeof(int16_t), &bytesRead, portMAX_DELAY);

  // Read from secondary I2S (non-blocking)
  size_t bytesRead2 = 0;
  i2s_read(I2S_PORT_1, rxBuffer2, BUFFER_LEN * sizeof(int16_t), &bytesRead2, 0);

  if (result == ESP_OK && bytesRead > 0) {
    int samplesRead = bytesRead / sizeof(int16_t);
    int samplesRead2 = bytesRead2 / sizeof(int16_t);

    if(startup_time == 0 || millis() - startup_time > 1000) {
      // Process audio through module (primary and secondary)
      moduleLoop(rxBuffer, txBuffer, samplesRead,
                 rxBuffer2, txBuffer2, samplesRead2);
    } else {
      memset(txBuffer, 0, sizeof(txBuffer));
    }

    // Send processed audio data back to WM8960 DAC via I2S
    size_t bytesWritten = 0;
    i2s_write(I2S_PORT, txBuffer, samplesRead * sizeof(int16_t), &bytesWritten, portMAX_DELAY);

    // Send secondary processed audio if any was read
    if (samplesRead2 > 0) {
      size_t bytesWritten2 = 0;
      i2s_write(I2S_PORT_1, txBuffer2, samplesRead2 * sizeof(int16_t), &bytesWritten2, 0);
    }
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
    .bits_per_sample = I2S_BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FORMAT,
    .communication_format = I2S_COMM_FORMAT,
    .intr_alloc_flags = I2S_INTR_ALLOC_FLAGS,
    .dma_buf_count = I2S_DMA_BUF_COUNT,
    .dma_buf_len = BUFFER_LEN,
    .use_apll = I2S_USE_APLL,
    .tx_desc_auto_clear = I2S_TX_DESC_AUTO_CLEAR,
    .fixed_mclk = I2S_FIXED_MCLK,
    .mclk_multiple = I2S_MCLK_MULTIPLE,
    .bits_per_chan = I2S_BITS_PER_CHAN
  };

  // Install I2S driver
  esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (result != ESP_OK) {
    reportError("I2S0 driver install failed");
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
    reportError("I2S0 set pin failed");
    return;
  }

#if I2S_IS_SLAVE
  Serial.println("I2S configured as SLAVE for bidirectional communication on GPIO 10-13");
#else
  Serial.println("I2S configured as MASTER for bidirectional communication on GPIO 10-13");
#endif
}

void setupI2S1() {
  // Configure I2S1 as SLAVE for RX + TX
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FORMAT,
    .communication_format = I2S_COMM_FORMAT,
    .intr_alloc_flags = I2S_INTR_ALLOC_FLAGS,
    .dma_buf_count = I2S_DMA_BUF_COUNT,
    .dma_buf_len = BUFFER_LEN,
    .use_apll = I2S_USE_APLL,
    .tx_desc_auto_clear = I2S_TX_DESC_AUTO_CLEAR,
    .fixed_mclk = I2S_FIXED_MCLK,
    .mclk_multiple = I2S_MCLK_MULTIPLE,
    .bits_per_chan = I2S_BITS_PER_CHAN
  };

  esp_err_t result = i2s_driver_install(I2S_PORT_1, &i2s_config, 0, NULL);
  if (result != ESP_OK) {
    reportError("I2S1 driver install failed");
    return;
  }

  const i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = I2S1_SCK,
    .ws_io_num = I2S1_WS,
    .data_out_num = I2S1_SD_OUT,
    .data_in_num = I2S1_SD_IN
  };

  result = i2s_set_pin(I2S_PORT_1, &pin_config);
  if (result != ESP_OK) {
    reportError("I2S1 set pin failed");
    return;
  }

  Serial.printf("I2S1 configured as SLAVE RX+TX: BCK=%d, WS=%d, SD_OUT=%d, SD_IN=%d\n",
                I2S1_SCK, I2S1_WS, I2S1_SD_OUT, I2S1_SD_IN);
}