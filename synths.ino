#include <driver/i2s.h>

// Module selection
#define MODULE_PASSTHROUGH 0
#define MODULE_DELAY 1
#define MODULE_MERGER 2
#define MODULE_DEBUG_TONE 3
#define ACTIVE_MODULE MODULE_DEBUG_TONE

#define ENABLE_GATEWAY 1
// Interface role policy:
// - Downstream (I2SD / I2S0) is ALWAYS master
// - Upstream (I2SU / I2S1) is slave, EXCEPT when ENABLE_GATEWAY == 1, then it is master too
// The old I2S_IS_SLAVE toggle is no longer used for I2SD

// I2S pin definitions for ESP32-S3-Zero (GPIO 1-13 only)
#define I2SD_WS 13 // Word Select (Left/Right Clock) - shared (Downstream)
#define I2SD_SD_IN 12 // Serial Data In (ADC data FROM WM8960)
#define I2SD_SD_OUT 10 // Serial Data Out (DAC data TO WM8960)
#define I2SD_SCK 11 // Serial Clock (Bit Clock) - shared

#define I2SD_PORT I2S_NUM_0

// Upstream I2S (I2S_NUM_1) pin definitions
#define I2SU_WS 7
#define I2SU_SD_IN 9
#define I2SU_SD_OUT 4
#define I2SU_SCK 8
#define I2SU_PORT I2S_NUM_1

// Shared potentiometer pin assignments (available to all modules)
#define POT_PIN_PRIMARY 1
#define POT_PIN_SECONDARY 2

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

int16_t rxBufferDownstream[BUFFER_LEN];   // input from I2SU → to I2SD
int16_t txBufferDownstream[BUFFER_LEN];
int16_t rxBufferUpstream[BUFFER_LEN];     // input from I2SD → to I2SU
int16_t txBufferUpstream[BUFFER_LEN];

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

#if ACTIVE_MODULE == MODULE_DEBUG_TONE
// Simple 100 Hz sine generator for debugging
// --- Debug Tone Configuration ---
// Frequency and amplitude
#ifndef DEBUG_TONE_FREQ_HZ
#define DEBUG_TONE_FREQ_HZ 100.0f
#endif
#ifndef DEBUG_TONE_AMPLITUDE
#define DEBUG_TONE_AMPLITUDE 1000.0f
#endif

// Per-direction toggle: 1 = generate tone, 0 = passthrough input
// Comment out or set to 0 to passthrough that direction instead of generating tone
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

// All modules now use a single samplesLength across both interfaces.

unsigned long startup_time = 0;

// Directional processing API (implemented by active module)
void moduleLoopUpstream(int16_t* inputBuffer, int16_t* outputBuffer, int samplesLength);
void moduleLoopDownstream(int16_t* inputBuffer, int16_t* outputBuffer, int samplesLength);

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

void setup() {
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
  // NeoPixel off at start
  setNeoPixelColor(0, 0, 0);
}

void loop() {
  // During startup mute window, do not process; keep outputs silent
  if (!(startup_time == 0 || millis() - startup_time > 1000)) {
    return;
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