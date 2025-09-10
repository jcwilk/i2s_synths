#ifndef I2S_INTERFACES_H
#define I2S_INTERFACES_H

#include <Arduino.h>
#include <string.h>
#include <driver/i2s_std.h>
#include <driver/i2s_common.h>
#include "../config/constants.h"
#include "i2s_output.h"
#include "i2s_input.h"

// Upstream/Downstream I2S channel handles
static i2s_chan_handle_t i2s_tx_u = NULL;
static i2s_chan_handle_t i2s_rx_u = NULL;
static i2s_chan_handle_t i2s_tx_d = NULL;
static i2s_chan_handle_t i2s_rx_d = NULL;

// Event counters/flags
static I2SOutputState i2s_output_state = { 0, 0, false, false, 0, NULL, NULL };
static I2SInputState i2s_input_state = { false, false, NULL };

// Rotating TX buffer state for simple generator
#define SINE_NUM_BUFFERS 4
#define SINE_TARGET_BUFFER_DEPTH 2
static float sine_phase = 0.0f;
static int16_t sine_buffers[SINE_NUM_BUFFERS][BUFFER_LEN];
static int sine_buffer_index = 0;
static int16_t rx_sink_buffer[BUFFER_LEN]; // throwaway RX buffer for gating

// TX sent callback now owned/registered by i2s_output module

static inline void generateSineBuffer(int16_t* buffer, int frames, float frequency, float amplitude) {
  const float sampleRate = (float)SAMPLE_RATE;
  const float phaseIncrement = (2.0f * PI * frequency) / sampleRate;
  for (int i = 0; i < frames; i++) {
    float sample = sinf(sine_phase) * amplitude;
    int16_t s = (int16_t)sample;
    buffer[(i << 1) + 0] = s;
    buffer[(i << 1) + 1] = s;
    sine_phase += phaseIncrement;
    if (sine_phase >= 2.0f * PI) sine_phase -= 2.0f * PI;
  }
}

static inline void writeSineBufferWhenRxReady() {
  const size_t fullBytes = sizeof(sine_buffers[0]);
  I2SInputReadOutcome r = i2s_input_read(i2s_input_state, rx_sink_buffer, fullBytes);
  i2s_input_state = r.state;
  if (r.result != I2S_INPUT_READ_OK) {
    return; // no RX data available; don't transmit
  }
  const int framesPerBuf = BUFFER_LEN / 2;
  int16_t* buf = sine_buffers[sine_buffer_index];
  generateSineBuffer(buf, framesPerBuf, 440.0f, 8000.0f);
  I2SOutputWriteOutcome w = i2s_output_write(i2s_output_state, i2s_tx_u, buf, sizeof(sine_buffers[0]));
  i2s_output_state = w.state;
  if (w.result == I2S_OUTPUT_WRITE_OK) {
    sine_buffer_index = (sine_buffer_index + 1) % SINE_NUM_BUFFERS;
  }
}

static inline void refillTxBuffersWithSilence() {
  const int maxQueued = (int)(I2S_DMA_BUF_COUNT / 2);
  while (i2s_output_can_queue(i2s_output_state) && i2s_output_state.tx_buffers_queued < maxQueued) {
    int16_t* sbuf = sine_buffers[sine_buffer_index];
    memset(sbuf, 0, sizeof(sine_buffers[0]));
    I2SOutputWriteOutcome w = i2s_output_write(i2s_output_state, i2s_tx_u, sbuf, sizeof(sine_buffers[0]));
    i2s_output_state = w.state;
    if (w.result == I2S_OUTPUT_WRITE_OK) {
      sine_buffer_index = (sine_buffer_index + 1) % SINE_NUM_BUFFERS;
    } else if (w.result == I2S_OUTPUT_WRITE_TIMEOUT) {
      break;
    } else {
      // Error already reflected in state by wrapper
      break;
    }
  }
}

static bool setupI2SOverlap(i2s_port_t port,
                            i2s_role_t role,
                            const i2s_std_gpio_config_t& gpio_cfg,
                            i2s_chan_handle_t& out_tx,
                            i2s_chan_handle_t& out_rx,
                            bool preload_tx_zero) {
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(port, role);
  chan_cfg.dma_frame_num = (uint32_t)(BUFFER_LEN / 2);
  chan_cfg.dma_desc_num = (uint32_t)I2S_DMA_BUF_COUNT;
  esp_err_t result = i2s_new_channel(&chan_cfg, &out_tx, &out_rx);
  if (result != ESP_OK) {
    Serial.println("I2S channel allocation failed");
    return false;
  }

  const i2s_std_clk_config_t clk_cfg = {
    .sample_rate_hz = SAMPLE_RATE,
    .clk_src = I2S_CLK_SRC_DEFAULT,
    .ext_clk_freq_hz = 0,
    .mclk_multiple = I2S_MCLK_MULTIPLE_256,
    .bclk_div = 8,
  };
  const i2s_std_slot_config_t slot_cfg = {
    .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
    .slot_bit_width = I2S_SLOT_BIT_WIDTH_16BIT,
    .slot_mode = I2S_SLOT_MODE_STEREO,
    .slot_mask = I2S_STD_SLOT_BOTH,
    .ws_width = 16,
    .ws_pol = false,
    .bit_shift = true,
    .left_align = false,
    .big_endian = false,
    .bit_order_lsb = false,
  };
  const i2s_std_config_t std_cfg = { .clk_cfg = clk_cfg, .slot_cfg = slot_cfg, .gpio_cfg = gpio_cfg };

  result = i2s_channel_init_std_mode(out_tx, &std_cfg);
  if (result != ESP_OK) { Serial.println("I2S TX init failed"); return false; }
  result = i2s_channel_init_std_mode(out_rx, &std_cfg);
  if (result != ESP_OK) { Serial.println("I2S RX init failed"); return false; }


  return true;
}

static inline void setupI2SU() {
  const i2s_std_gpio_config_t gpio_cfg = {
    .mclk = (gpio_num_t)I2SU_MCK,
    .bclk = (gpio_num_t)I2SU_SCK,
    .ws = (gpio_num_t)I2SU_WS,
    .dout = (gpio_num_t)I2SU_SD_OUT,
    .din = (gpio_num_t)I2SU_SD_IN,
    .invert_flags = { .mclk_inv = 0, .bclk_inv = 0, .ws_inv = 0 },
  };
  #if ENABLE_GATEWAY
    const i2s_role_t role = I2S_ROLE_MASTER;
  #else
    const i2s_role_t role = I2S_ROLE_SLAVE;
  #endif
  if (!setupI2SOverlap(I2SU_PORT, role, gpio_cfg, i2s_tx_u, i2s_rx_u, true)) return;
  
  // Initialize output state, allocating mailbox and registering callbacks
  i2s_output_state = i2s_output_make_initial(i2s_tx_u);

  i2s_channel_enable(i2s_tx_u);
  i2s_channel_enable(i2s_rx_u);

  Serial.printf("I2SU ready: MCK=%d, BCK=%d, WS=%d, DOUT=%d, DIN=%d\n", I2SU_MCK, I2SU_SCK, I2SU_WS, I2SU_SD_OUT, I2SU_SD_IN);
}

static inline void setupI2SD() {
  const i2s_std_gpio_config_t gpio_cfg = {
    .mclk = (gpio_num_t)I2SD_MCK,
    .bclk = (gpio_num_t)I2SD_SCK,
    .ws = (gpio_num_t)I2SD_WS,
    .dout = (gpio_num_t)I2SD_SD_OUT,
    .din = (gpio_num_t)I2SD_SD_IN,
    .invert_flags = { .mclk_inv = 0, .bclk_inv = 0, .ws_inv = 0 },
  };
  if (!setupI2SOverlap(I2SD_PORT, I2S_ROLE_MASTER, gpio_cfg, i2s_tx_d, i2s_rx_d, true)) return;

  i2s_input_state = i2s_input_make_initial(i2s_rx_d);

  i2s_channel_enable(i2s_tx_d);
  i2s_channel_enable(i2s_rx_d);
  
  Serial.printf("I2SD ready: MCK=%d, BCK=%d, WS=%d, DOUT=%d, DIN=%d\n", I2SD_MCK, I2SD_SCK, I2SD_WS, I2SD_SD_OUT, I2SD_SD_IN);
}

static inline void i2sSetup() {
  setupI2SD();
  setupI2SU();
  #if ENABLE_GATEWAY == 1
    pinMode(POT_PIN_SECONDARY, OUTPUT);
    digitalWrite(POT_PIN_SECONDARY, HIGH);
    delay(100);
    digitalWrite(POT_PIN_SECONDARY, LOW);
    delay(50);
  #endif
}

static inline void i2sLoop(bool inStartupMute) {
  // Sync from ISR mailbox owned by output module
  i2s_output_state = i2s_output_sync_from_isr(i2s_output_state);
  if (inStartupMute) {
    refillTxBuffersWithSilence();
    return;
  }
  writeSineBufferWhenRxReady();
}

#endif // I2S_INTERFACES_H


