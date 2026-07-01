#include <emscripten.h>
#include <stdint.h>
#include <string.h>

#ifndef ACTIVE_MODULE
#define ACTIVE_MODULE 0
#endif

#include "config/constants.h"
#include "input/pots.h"

#if ACTIVE_MODULE == MODULE_PASSTHROUGH
#include "modules/passthrough.h"
#elif ACTIVE_MODULE == MODULE_DELAY
#include "modules/delay.h"
#elif ACTIVE_MODULE == MODULE_MERGER
#include "modules/merger.h"
#elif ACTIVE_MODULE == MODULE_DEBUG_TONE
#include "modules/debug_tone.h"
#elif ACTIVE_MODULE == MODULE_CUTOFF
#include "modules/cutoff.h"
#else
#error "Unsupported ACTIVE_MODULE for simulator harness"
#endif

static int16_t g_upstream_in[BUFFER_LEN];
static int16_t g_upstream_out[BUFFER_LEN];
static int16_t g_downstream_in[BUFFER_LEN];
static int16_t g_downstream_out[BUFFER_LEN];
static DualPotsState g_pots;

extern "C" {

EMSCRIPTEN_KEEPALIVE
void sim_setup() {
  memset(g_upstream_in, 0, sizeof(g_upstream_in));
  memset(g_upstream_out, 0, sizeof(g_upstream_out));
  memset(g_downstream_in, 0, sizeof(g_downstream_in));
  memset(g_downstream_out, 0, sizeof(g_downstream_out));
  g_pots = potsMakeInitial(0, 1);
  moduleSetup();
}

EMSCRIPTEN_KEEPALIVE
int16_t* sim_get_upstream_in() { return g_upstream_in; }

EMSCRIPTEN_KEEPALIVE
int16_t* sim_get_upstream_out() { return g_upstream_out; }

EMSCRIPTEN_KEEPALIVE
int16_t* sim_get_downstream_in() { return g_downstream_in; }

EMSCRIPTEN_KEEPALIVE
int16_t* sim_get_downstream_out() { return g_downstream_out; }

EMSCRIPTEN_KEEPALIVE
DualPotsState* sim_get_pots() { return &g_pots; }

EMSCRIPTEN_KEEPALIVE
int sim_get_buffer_len() { return BUFFER_LEN; }

EMSCRIPTEN_KEEPALIVE
void sim_process_upstream() {
  moduleLoopUpstream(g_upstream_in, g_upstream_out, BUFFER_LEN, g_pots);
}

EMSCRIPTEN_KEEPALIVE
void sim_process_downstream() {
  moduleLoopDownstream(g_downstream_in, g_downstream_out, BUFFER_LEN, g_pots);
}

#if ACTIVE_MODULE == MODULE_DELAY
EMSCRIPTEN_KEEPALIVE
int sim_get_delay_buffer_frames() { return g_bufferFrames; }
#endif

#if ACTIVE_MODULE == MODULE_MERGER
EMSCRIPTEN_KEEPALIVE
int sim_consume_merger_stress() {
  int flags = 0;
  if (mergerFlagUnderrunEvent) {
    mergerFlagUnderrunEvent = false;
    flags |= 1;
  }
  if (mergerFlagOverrunEvent) {
    mergerFlagOverrunEvent = false;
    flags |= 2;
  }
#if MERGER_ENABLE_DS_TO_US_FORWARD
  if (mergerRevFlagOverrunEvent) {
    mergerRevFlagOverrunEvent = false;
    flags |= 4;
  }
#endif
  return flags;
}
#endif

} // extern "C"
