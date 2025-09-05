#ifndef CONFIG_CONSTANTS_H
#define CONFIG_CONSTANTS_H

#include <driver/i2s.h>

// Module selection
#define MODULE_PASSTHROUGH 0
#define MODULE_DELAY 1
#define MODULE_MERGER 2
#define MODULE_DEBUG_TONE 3
#define MODULE_CUTOFF 4

// Active module selection (can be overridden before including this header)
#ifndef ACTIVE_MODULE
#define ACTIVE_MODULE MODULE_DEBUG_TONE
#endif

// Gateway and system policy
#define ENABLE_GATEWAY 1

// I2S pin definitions for downstream interface
#define I2SD_MCK 12
#define I2SD_WS 11
#define I2SD_SCK 10
#define I2SD_SD_OUT 9
#define I2SD_SD_IN 8
#define I2SD_PORT I2S_NUM_0

#if ENABLE_GATEWAY == 0
// I2S pin definitions for upstream interface
#define I2SU_MCK 1
#define I2SU_WS 2
#define I2SU_SCK 3
#define I2SU_SD_OUT 4
#define I2SU_SD_IN 5
#define I2SU_PORT I2S_NUM_1
#else
#define I2SU_MCK 2
#define I2SU_WS 3
#define I2SU_SCK 4
#define I2SU_SD_OUT 5
#define I2SU_SD_IN 6
#define I2SU_PORT I2S_NUM_1
#endif
// Shared potentiometer pins
#if ENABLE_GATEWAY == 0
#define POT_PIN_PRIMARY 6
#else
#define POT_PIN_PRIMARY 13 // gateway module doesn't have pots, but might as well assign it to an unused pin
#endif
#define POT_PIN_SECONDARY 7

// Timings and audio config
#define STARTUP_TIME_MS 1000

// TODO: these are arbitrary and may not be optimal
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

#endif // CONFIG_CONSTANTS_H


