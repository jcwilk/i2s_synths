#ifndef I2S_INTERFACES_H
#define I2S_INTERFACES_H

#include <Arduino.h>
#include <string.h>
#include <driver/i2s_std.h>
#include <driver/i2s_common.h>
#include "../config/constants.h"
#include "i2s_output.h"
#include "i2s_input.h"
#include "../input/pots.h"
#include "i2s_pipeline.h"

// Upstream/Downstream I2S channel handles
static i2s_chan_handle_t i2s_tx_u = NULL;
static i2s_chan_handle_t i2s_rx_u = NULL;
static i2s_chan_handle_t i2s_tx_d = NULL;
static i2s_chan_handle_t i2s_rx_d = NULL;

// Two pipelines: upstream (D -> U) and downstream (U -> D)
static I2SPipelineState i2s_pipeline_upstream_state;
static I2SPipelineState i2s_pipeline_downstream_state;

static float sine_phase = 0.0f;

// TX sent callback now owned/registered by i2s_output module

static inline void generateSineBuffer(int16_t* buffer, int sampleCount, float frequency, float amplitude) {
  const float sampleRate = (float)SAMPLE_RATE;
  const float phaseIncrement = (2.0f * PI * frequency) / sampleRate;
  for (int i = 0; i < sampleCount; i+=2) {
    float sample = sinf(sine_phase) * amplitude;
    int16_t s = (int16_t)sample;
    buffer[i] = s;
    buffer[i + 1] = s;
    sine_phase += phaseIncrement;
    if (sine_phase >= 2.0f * PI) sine_phase -= 2.0f * PI;
  }
}

// Pipeline processing callback: generate a sine buffer regardless of input
static inline void pipeline_generate_sine(int16_t* in_stereo_interleaved,
                                          int16_t* out_stereo_interleaved,
                                          int sampleCount) {
  (void)in_stereo_interleaved;
  generateSineBuffer(out_stereo_interleaved, sampleCount, 440.0f, 8000.0f);
}

// Aggregate states for a single interface
typedef struct {
  I2SInputState input_state;
  I2SOutputState output_state;
} I2SInterfaceStates;

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

static inline I2SInterfaceStates setupI2SU() {
  I2SInterfaceStates io_states = { 0 };
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
  if (!setupI2SOverlap(I2SU_PORT, role, gpio_cfg, i2s_tx_u, i2s_rx_u, true)) return io_states;

  io_states.input_state = i2s_input_make_initial(i2s_rx_u);
  io_states.input_state = i2s_input_finalize(io_states.input_state);
  io_states.output_state = i2s_output_make_initial(i2s_tx_u);
  io_states.output_state = i2s_output_finalize(io_states.output_state);

  Serial.printf("I2SU ready: MCK=%d, BCK=%d, WS=%d, DOUT=%d, DIN=%d\n", I2SU_MCK, I2SU_SCK, I2SU_WS, I2SU_SD_OUT, I2SU_SD_IN);
  return io_states;
}

static inline I2SInterfaceStates setupI2SD() {
  I2SInterfaceStates io_states = { 0 };
  const i2s_std_gpio_config_t gpio_cfg = {
    .mclk = (gpio_num_t)I2SD_MCK,
    .bclk = (gpio_num_t)I2SD_SCK,
    .ws = (gpio_num_t)I2SD_WS,
    .dout = (gpio_num_t)I2SD_SD_OUT,
    .din = (gpio_num_t)I2SD_SD_IN,
    .invert_flags = { .mclk_inv = 0, .bclk_inv = 0, .ws_inv = 0 },
  };
  if (!setupI2SOverlap(I2SD_PORT, I2S_ROLE_MASTER, gpio_cfg, i2s_tx_d, i2s_rx_d, true)) return io_states;

  io_states.input_state = i2s_input_make_initial(i2s_rx_d);
  io_states.input_state = i2s_input_finalize(io_states.input_state);
  io_states.output_state = i2s_output_make_initial(i2s_tx_d);
  io_states.output_state = i2s_output_finalize(io_states.output_state);

  Serial.printf("I2SD ready: MCK=%d, BCK=%d, WS=%d, DOUT=%d, DIN=%d\n", I2SD_MCK, I2SD_SCK, I2SD_WS, I2SD_SD_OUT, I2SD_SD_IN);
  return io_states;
}

static inline void i2sSetup(I2SPipelineProcessFn downstream_process,
                            I2SPipelineProcessFn upstream_process) {
  I2SInterfaceStates d_states = setupI2SD();
  I2SInterfaceStates u_states = setupI2SU();
  #if ENABLE_GATEWAY == 1
    pinMode(POT_PIN_SECONDARY, OUTPUT);
    digitalWrite(POT_PIN_SECONDARY, HIGH);
    delay(100);
    digitalWrite(POT_PIN_SECONDARY, LOW);
    delay(50);
  #endif

  // Upstream: downstream RX -> upstream TX
  i2s_pipeline_upstream_state = i2s_pipeline_make_initial(d_states.input_state, u_states.output_state, upstream_process);
  // Downstream: upstream RX -> downstream TX
  i2s_pipeline_downstream_state = i2s_pipeline_make_initial(u_states.input_state, d_states.output_state, downstream_process);
}

static inline void i2sLoop(bool inStartupMute, DualPotsState pots_state) {
  if (inStartupMute) {
    return;
  }
  i2s_pipeline_upstream_state = i2s_pipeline_process(i2s_pipeline_upstream_state, pots_state);
  i2s_pipeline_downstream_state = i2s_pipeline_process(i2s_pipeline_downstream_state, pots_state);
}

#endif // I2S_INTERFACES_H


