#include <driver/i2s.h>
#include "src/config/constants.h"

// Select active module by defining ACTIVE_MODULE before constants.h, or rely on its default.

// Interface role policy:
// - Downstream (I2SD / I2S0) is ALWAYS master
// - Upstream (I2SU / I2S1) is slave, EXCEPT when ENABLE_GATEWAY == 1, then it is master too
// The old I2S_IS_SLAVE toggle is no longer used for I2SD

// Pins and config come from src/config/constants.h

// Timed color effect with optional fade behavior
#include "src/ui/neopixel.h"

int16_t rxBufferDownstream[BUFFER_LEN];   // input from I2SU → to I2SD
int16_t txBufferDownstream[BUFFER_LEN];
int16_t rxBufferUpstream[BUFFER_LEN];     // input from I2SD → to I2SU
int16_t txBufferUpstream[BUFFER_LEN];


#if ACTIVE_MODULE == MODULE_PASSTHROUGH
void moduleSetup() {
  Serial.println("Passthrough module active");
}
void moduleLoopDownstream(int16_t* inputBuffer,
                          int16_t* outputBuffer,
                          int samplesLength) {
  if (outputBuffer && inputBuffer && samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}
void moduleLoopUpstream(int16_t* inputBuffer,
                        int16_t* outputBuffer,
                        int samplesLength) {
  if (outputBuffer && inputBuffer && samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
}
#endif

// Directional processing API (implemented by active module)
void moduleLoopUpstream(int16_t* inputBuffer, int16_t* outputBuffer, int samplesLength);
void moduleLoopDownstream(int16_t* inputBuffer, int16_t* outputBuffer, int samplesLength);

// Include the selected audio processing module header when needed
#if ACTIVE_MODULE == MODULE_MERGER
#include "src/modules/merger/merger.h"
#endif

static inline void processPath(i2s_port_t readPort,
                               i2s_port_t writePort,
                               int16_t* rxBuffer,
                               int16_t* txBuffer,
                               void (*processFn)(int16_t*, int16_t*, int)) {
  size_t bytesRead = 0;
  if (i2s_read(readPort, rxBuffer, BUFFER_LEN * sizeof(int16_t), &bytesRead, 0) == ESP_OK) {
    int samples = (int)(bytesRead / sizeof(int16_t));
    if (samples > 0) {
      processFn(rxBuffer, txBuffer, samples);
      size_t bytesWritten = 0;
      i2s_write(writePort, txBuffer, samples * sizeof(int16_t), &bytesWritten, 0);
    }
  }
}

unsigned long startup_time = 0;
static unsigned long lastLoopMs = 0;
static unsigned long accumulatedDeltaMs = 0;
static unsigned long baseLoopMs = 0;
static bool startup_active = true;

#include "src/input/pots.h"

void setup() {
  neopixelSetTimedColor(20,20, 0, STARTUP_TIME_MS, NEOPIXEL_MODE_LINEAR);

  Serial.begin(115200);
  Serial.println("ESP32-S3-Zero I2S Audio Processing (GPIO 1-13)");

  // Initialize gateway (WM8960 management)
  gatewaySetup();

  // Setup I2S for bidirectional communication
  setupI2SD();

  // Setup secondary I2S interface (RX + TX as slave)
  setupI2SU();

  // Initialize audio processing module
  moduleSetup();

  Serial.println("Setup complete. Audio processing active on GPIO 10-13.");
  Serial.println("Audio Input -> ESP32 -> Audio Processing -> ESP32 -> Audio Output");
  startup_time = millis();
}

void loop() {
  // Coherent delta time tracking for effects updates
  unsigned long nowMs = millis();
  if (lastLoopMs == 0) {
    lastLoopMs = nowMs;
    baseLoopMs = nowMs;
  }
  unsigned long normalizedNow = nowMs - baseLoopMs;
  unsigned long deltaMs = 0;
  if (normalizedNow > accumulatedDeltaMs) {
    deltaMs = normalizedNow - accumulatedDeltaMs;
    accumulatedDeltaMs += deltaMs;
  }

  neopixelUpdate((uint32_t)deltaMs);

  // During startup mute window, do not process; keep outputs silent
  if (!(startup_time == 0 || millis() - startup_time > 1000)) {
    return;
  }

  if (startup_active) {
    startup_active = false;
    neopixelSetTimedColor(0, 25, 0, 200, NEOPIXEL_MODE_LINEAR);
  }

  // Upstream path: I2SD -> processing -> I2SU
  processPath(I2SD_PORT, I2SU_PORT, rxBufferUpstream, txBufferUpstream, moduleLoopUpstream);
  // Downstream path: I2SU -> processing -> I2SD
  processPath(I2SU_PORT, I2SD_PORT, rxBufferDownstream, txBufferDownstream, moduleLoopDownstream);
}

void setupI2SD() {
  // Configure I2S for bidirectional communication (RX + TX)
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX), // Downstream interface ALWAYS master
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
  esp_err_t result = i2s_driver_install(I2SD_PORT, &i2s_config, 0, NULL);
  if (result != ESP_OK) {
    reportError("I2SD driver install failed");
    return;
  }

  // Configure I2S pins
  const i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE, // No master clock needed
    .bck_io_num = I2SD_SCK, // Bit clock (shared) - GPIO 11
    .ws_io_num = I2SD_WS, // Word select (shared) - GPIO 13
    .data_out_num = I2SD_SD_OUT, // Data output (to WM8960 DAC) - GPIO 10
    .data_in_num = I2SD_SD_IN // Data input (from WM8960 ADC) - GPIO 12
  };

  result = i2s_set_pin(I2SD_PORT, &pin_config);
  if (result != ESP_OK) {
    reportError("I2SD set pin failed");
    return;
  }

  Serial.println("I2SD configured as MASTER for bidirectional communication on GPIO 10-13");
}

void setupI2SU() {
  // Configure I2S1 as SLAVE for RX + TX
  const i2s_config_t i2s_config = {
  #if ENABLE_GATEWAY
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX), // With gateway, Upstream interface is MASTER
  #else
    .mode = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX | I2S_MODE_TX), // Otherwise, Upstream interface is SLAVE
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

  esp_err_t result = i2s_driver_install(I2SU_PORT, &i2s_config, 0, NULL);
  if (result != ESP_OK) {
    reportError("I2SU driver install failed");
    return;
  }

  const i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = I2SU_SCK,
    .ws_io_num = I2SU_WS,
    .data_out_num = I2SU_SD_OUT,
    .data_in_num = I2SU_SD_IN
  };

  result = i2s_set_pin(I2SU_PORT, &pin_config);
  if (result != ESP_OK) {
    reportError("I2SU set pin failed");
    return;
  }

  #if ENABLE_GATEWAY
  Serial.printf("I2SU configured as MASTER RX+TX: BCK=%d, WS=%d, SD_OUT=%d, SD_IN=%d\n",
                I2SU_SCK, I2SU_WS, I2SU_SD_OUT, I2SU_SD_IN);
  #else
  Serial.printf("I2SU configured as SLAVE RX+TX: BCK=%d, WS=%d, SD_OUT=%d, SD_IN=%d\n",
                I2SU_SCK, I2SU_WS, I2SU_SD_OUT, I2SU_SD_IN);
  #endif
}