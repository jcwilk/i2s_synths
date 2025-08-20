#include <driver/i2s.h>

// Module selection
#define MODULE_PASSTHROUGH 0
#define MODULE_DELAY 1
#define MODULE_MERGER 2
#define MODULE_DEBUG_TONE 3
#define ACTIVE_MODULE MODULE_MERGER

#define ENABLE_GATEWAY 1
// Interface role policy:
// - Interface 1 (I2S0) is ALWAYS master
// - Interface 2 (I2S1) is slave, EXCEPT when ENABLE_GATEWAY == 1, then it is master too
// The old I2S_IS_SLAVE toggle is no longer used for I2S0

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

int16_t rxBuffer[BUFFER_LEN];
int16_t txBuffer[BUFFER_LEN];
int16_t rxBuffer2[BUFFER_LEN];
int16_t txBuffer2[BUFFER_LEN];

// Small FIFOs to accumulate and synchronize reads across both interfaces
static const int ACCUM_CAPACITY = BUFFER_LEN * 4;
static int16_t inputAccum1[ACCUM_CAPACITY];
static int inputAccumCount1 = 0;
static int16_t inputAccum2[ACCUM_CAPACITY];
static int inputAccumCount2 = 0;

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
                int16_t* inputBuffer2,
                int16_t* outputBuffer2,
                int samplesLength) {
  // Pure passthrough per interface (1->1, 2->2). Cross-routing is handled in loop().
  if (outputBuffer && inputBuffer && samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
  }
  if (outputBuffer2 && inputBuffer2 && samplesLength > 0) {
    memcpy(outputBuffer2, inputBuffer2, samplesLength * sizeof(int16_t));
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

// Per-interface toggle: 1 = generate tone, 0 = passthrough input
// Comment out or set to 0 to passthrough that interface instead of generating tone
#ifndef DEBUG_TONE_PRIMARY
#define DEBUG_TONE_PRIMARY 0
#endif
#ifndef DEBUG_TONE_SECONDARY
#define DEBUG_TONE_SECONDARY 1
#endif

static float debugTonePhasePrimary = 0.0f;
static float debugTonePhaseSecondary = 0.0f;

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
  debugTonePhasePrimary = 0.0f;
  debugTonePhaseSecondary = 0.0f;
}
void moduleLoop(int16_t* inputBuffer,
                int16_t* outputBuffer,
                int16_t* inputBuffer2,
                int16_t* outputBuffer2,
                int samplesLength) {
  const float phaseIncrement = 2.0f * PI * DEBUG_TONE_FREQ_HZ / (float)SAMPLE_RATE;

  // Primary interface
  if (samplesLength > 0 && outputBuffer) {
  #if DEBUG_TONE_PRIMARY
    fillSine(outputBuffer, samplesLength, debugTonePhasePrimary, phaseIncrement, DEBUG_TONE_AMPLITUDE);
  #else
    if (inputBuffer) {
      memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
    }
  #endif
  }

  // Secondary interface
  if (samplesLength > 0 && outputBuffer2) {
  #if DEBUG_TONE_SECONDARY
    fillSine(outputBuffer2, samplesLength, debugTonePhaseSecondary, phaseIncrement, DEBUG_TONE_AMPLITUDE);
  #else
    if (inputBuffer2) {
      memcpy(outputBuffer2, inputBuffer2, samplesLength * sizeof(int16_t));
    }
  #endif
  }
}
#endif

// All modules now use a single samplesLength across both interfaces.

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
  // Accumulate non-blocking reads from both interfaces, then process equal-length chunks
  int16_t tempIn1[BUFFER_LEN];
  int16_t tempIn2[BUFFER_LEN];

  // Read from primary I2S (non-blocking)
  size_t bytesRead1 = 0;
  i2s_read(I2S_PORT, tempIn1, BUFFER_LEN * sizeof(int16_t), &bytesRead1, 0);
  int samples1 = bytesRead1 / sizeof(int16_t);
  if (samples1 > 0 && inputAccumCount1 < ACCUM_CAPACITY) {
    int space1 = ACCUM_CAPACITY - inputAccumCount1;
    int toCopy1 = min(samples1, space1);
    if (toCopy1 > 0) {
      memcpy(inputAccum1 + inputAccumCount1, tempIn1, toCopy1 * sizeof(int16_t));
      inputAccumCount1 += toCopy1;
    }
  }

  // Read from secondary I2S (non-blocking)
  size_t bytesRead2 = 0;
  i2s_read(I2S_PORT_1, tempIn2, BUFFER_LEN * sizeof(int16_t), &bytesRead2, 0);
  int samples2 = bytesRead2 / sizeof(int16_t);
  if (samples2 > 0 && inputAccumCount2 < ACCUM_CAPACITY) {
    int space2 = ACCUM_CAPACITY - inputAccumCount2;
    int toCopy2 = min(samples2, space2);
    if (toCopy2 > 0) {
      memcpy(inputAccum2 + inputAccumCount2, tempIn2, toCopy2 * sizeof(int16_t));
      inputAccumCount2 += toCopy2;
    }
  }

  // During startup mute window, do not process; keep outputs silent and clear accumulators
  if (!(startup_time == 0 || millis() - startup_time > 1000)) {
    inputAccumCount1 = 0;
    inputAccumCount2 = 0;
    return;
  }

  // Determine unified samplesLength that both interfaces can supply
  int samplesLength = min(inputAccumCount1, inputAccumCount2);
  if (samplesLength <= 0) {
    return; // wait for both to have data
  }
  if (samplesLength > BUFFER_LEN) {
    samplesLength = BUFFER_LEN;
  }

  // Prepare equal-length inputs
  memcpy(rxBuffer, inputAccum1, samplesLength * sizeof(int16_t));
  memcpy(rxBuffer2, inputAccum2, samplesLength * sizeof(int16_t));

  // Process through module (1->1 and 2->2) with a single unified length
  moduleLoop(rxBuffer, txBuffer, rxBuffer2, txBuffer2, samplesLength);

  // Cross-route at write stage using the same samplesLength for both
  size_t bytesWrittenA = 0;
  i2s_write(I2S_PORT_1, txBuffer, samplesLength * sizeof(int16_t), &bytesWrittenA, 0);

  size_t bytesWrittenB = 0;
  i2s_write(I2S_PORT, txBuffer2, samplesLength * sizeof(int16_t), &bytesWrittenB, portMAX_DELAY);

  // Consume from accumulators
  if (inputAccumCount1 - samplesLength > 0) {
    memmove(inputAccum1, inputAccum1 + samplesLength, (inputAccumCount1 - samplesLength) * sizeof(int16_t));
  }
  inputAccumCount1 -= samplesLength;

  if (inputAccumCount2 - samplesLength > 0) {
    memmove(inputAccum2, inputAccum2 + samplesLength, (inputAccumCount2 - samplesLength) * sizeof(int16_t));
  }
  inputAccumCount2 -= samplesLength;
}

void setupI2S() {
  // Configure I2S for bidirectional communication (RX + TX)
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX), // Interface 1 ALWAYS master
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

  Serial.println("I2S configured as MASTER for bidirectional communication on GPIO 10-13");
}

void setupI2S1() {
  // Configure I2S1 as SLAVE for RX + TX
  const i2s_config_t i2s_config = {
  #if ENABLE_GATEWAY
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX), // With gateway, Interface 2 is MASTER
  #else
    .mode = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX | I2S_MODE_TX), // Otherwise, Interface 2 is SLAVE
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

  #if ENABLE_GATEWAY
  Serial.printf("I2S1 configured as MASTER RX+TX: BCK=%d, WS=%d, SD_OUT=%d, SD_IN=%d\n",
                I2S1_SCK, I2S1_WS, I2S1_SD_OUT, I2S1_SD_IN);
  #else
  Serial.printf("I2S1 configured as SLAVE RX+TX: BCK=%d, WS=%d, SD_OUT=%d, SD_IN=%d\n",
                I2S1_SCK, I2S1_WS, I2S1_SD_OUT, I2S1_SD_IN);
  #endif
}