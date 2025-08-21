#include <driver/i2s_std.h>
#include <driver/i2s_common.h>
#include <esp_rom_gpio.h>
#include <hal/i2s_hal.h>
#include <string.h>
#include <math.h>

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

// Upstream I2S (I2S_NUM_1) pin definitions
#define I2SU_WS 7
#define I2SU_SD_IN 9
#define I2SU_SD_OUT 4
#define I2SU_SCK 8
#define I2SU_PORT I2S_NUM_1

// Shared potentiometer pin assignments (available to all modules)
#define POT_PIN_PRIMARY 1
#define POT_PIN_SECONDARY 2

#define STARTUP_TIME_MS 1000

#define SAMPLE_RATE 44100
#define BUFFER_LEN 128
#define I2S_DMA_BUF_COUNT 4
// Timed color effect with optional fade behavior
#define NEOPIXEL_MODE_HOLD 0       // stay at full color for duration, then off
#define NEOPIXEL_MODE_LINEAR 1     // fade linearly to off over duration
#define NEOPIXEL_MODE_QUADRATIC 2  // fade quadratically to off over duration
// Additional configuration for buffer priming
#define SILENCE_BUFFER_SIZE (BUFFER_LEN * 2)  // bytes, for 16-bit mono words count compatibility

int16_t rxBufferDownstream[BUFFER_LEN];   // input from I2SU → to I2SD (unused)
int16_t txBufferDownstream[BUFFER_LEN];
int16_t rxBufferUpstream[BUFFER_LEN];     // input from I2SD → to I2SU (unused)
int16_t txBufferUpstream[BUFFER_LEN];
static int16_t txStereoBuffer[BUFFER_LEN * 2]; // interleaved L/R, words == 2 * frames
static float simpleSinePhase = 0.0f;
#ifndef SIMPLE_SINE_FREQ_HZ
#define SIMPLE_SINE_FREQ_HZ 220.0f
#endif

// I2S std channel handles (upstream only)
static i2s_chan_handle_t i2su_tx_handle = NULL;
static i2s_chan_handle_t i2su_rx_handle = NULL;


// No module processing used in this simplified build

static void setupI2SChannels() {
  // Configure only I2S1 (Upstream) TX as MASTER and start it
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
  chan_cfg.dma_desc_num = I2S_DMA_BUF_COUNT;
  chan_cfg.dma_frame_num = BUFFER_LEN;
  i2s_new_channel(&chan_cfg, &i2su_tx_handle, NULL);

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = (gpio_num_t)I2SU_SCK,
      .ws = (gpio_num_t)I2SU_WS,
      .dout = (gpio_num_t)I2SU_SD_OUT,
      .din = I2S_GPIO_UNUSED,
    },
  };
  std_cfg.slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_16BIT;
#ifdef I2S_CLK_SRC_APLL
  std_cfg.clk_cfg.clk_src = I2S_CLK_SRC_APLL;
#else
  std_cfg.clk_cfg.clk_src = I2S_CLK_SRC_DEFAULT;
#endif
  std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;
  i2s_channel_init_std_mode(i2su_tx_handle, &std_cfg);

  i2s_channel_enable(i2su_tx_handle);
}

unsigned long startup_time = 0;

void setup() {
  neopixelSetTimedColor(20,20, 0, STARTUP_TIME_MS, NEOPIXEL_MODE_LINEAR);

  Serial.begin(115200);
  Serial.println("ESP32-S3-Zero I2S Audio Processing (GPIO 1-13)");

  // Initialize gateway (WM8960 management)
  gatewaySetup();

  // Setup I2S standard channels for bidirectional communication
  setupI2SChannels();

  // Initialize audio processing module
  moduleSetup();

  Serial.println("Setup complete. Audio processing active on GPIO 10-13.");
  Serial.println("Audio Input -> ESP32 -> Audio Processing -> ESP32 -> Audio Output");
  startup_time = millis();
}

void loop() {
  // Optional neopixel update (no-op if you prefer to remove visual feedback)
  neopixelUpdate(0);

  // Minimal output path: generate a simple sine and write interleaved stereo to I2S0 TX
  {
    const int stereoFrames = BUFFER_LEN;         // number of LR frames to generate
    const int totalWords = stereoFrames * 2;     // L+R words
    const float phaseIncrement = 2.0f * 3.14159265358979323846f * SIMPLE_SINE_FREQ_HZ / (float)SAMPLE_RATE;
    const float amplitude = 12000.0f;
    for (int f = 0; f < stereoFrames; f++) {
      float s = sinf(simpleSinePhase) * amplitude;
      int16_t v = (int16_t)(s > 32767.0f ? 32767.0f : (s < -32768.0f ? -32768.0f : s));
      txStereoBuffer[(f << 1)] = v;      // L
      txStereoBuffer[(f << 1) + 1] = v;  // R
      simpleSinePhase += phaseIncrement;
      if (simpleSinePhase >= 2.0f * 3.14159265358979323846f) simpleSinePhase -= 2.0f * 3.14159265358979323846f;
    }
    size_t bytesWritten = 0;
    i2s_channel_write(i2su_tx_handle, (const uint8_t*)txStereoBuffer,
                      totalWords * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
  }
}
// Legacy setup functions removed; using setupI2SChannels instead