#include "src/config/constants.h"
#include "src/input/pots.h"
#include "src/streaming/i2s_interfaces.h"

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

// Directional processing API (implemented by active module)
void moduleLoopUpstream(int16_t* inputBuffer, int16_t* outputBuffer, int samplesLength, DualPotsState pots_state);
void moduleLoopDownstream(int16_t* inputBuffer, int16_t* outputBuffer, int samplesLength, DualPotsState pots_state);

// Include the selected audio processing module header when needed
#if ACTIVE_MODULE == MODULE_MERGER
#include "src/modules/merger.h"
#elif ACTIVE_MODULE == MODULE_DEBUG_TONE
#include "src/modules/debug_tone.h"
#elif ACTIVE_MODULE == MODULE_DELAY
#include "src/modules/delay.h"
#elif ACTIVE_MODULE == MODULE_CUTOFF
#include "src/modules/cutoff.h"
#elif ACTIVE_MODULE == MODULE_PASSTHROUGH
#include "src/modules/passthrough.h"
#endif

unsigned long startup_time = 0;
static unsigned long accumulatedDeltaMs = 0;
static unsigned long baseLoopMs = 0;
static bool startup_active = true;

// Pots state is owned by the sketch and passed by value
static DualPotsState sketch_pots_state;

void setup() {
  neopixelSetTimedColor(20,20, 0, STARTUP_TIME_MS, NEOPIXEL_MODE_LINEAR);

  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-S3-Zero I2S Audio Processing");

  // Setup I2S interfaces (both downstream and upstream) with module processing callbacks
  i2sSetup(moduleLoopDownstream, moduleLoopUpstream);

  // Initialize audio processing module
  moduleSetup();

  // Initialize pots after module setup (ADC configured there)
  sketch_pots_state = potsMakeInitial(POT_PIN_PRIMARY, POT_PIN_SECONDARY);

  Serial.println("Setup complete.");
  startup_time = millis();
}

void loop() {
  // Coherent delta time tracking for UI updates
  unsigned long nowMs = millis();
  if (baseLoopMs == 0) {
    baseLoopMs = nowMs;
  }
  unsigned long normalizedNow = nowMs - baseLoopMs;
  unsigned long deltaMs = 0;
  if (normalizedNow > accumulatedDeltaMs) {
    deltaMs = normalizedNow - accumulatedDeltaMs;
    accumulatedDeltaMs += deltaMs;
  }
  neopixelUpdate((uint32_t)deltaMs);

  const bool inStartupMute = !(startup_time == 0 || millis() - startup_time > 1000);
  if (!inStartupMute && startup_active) {
    startup_active = false;
    neopixelSetTimedColor(0, 25, 0, 200, NEOPIXEL_MODE_LINEAR);
  }

  // Update potentiometers using elapsed time scaling
  sketch_pots_state = potsUpdate(sketch_pots_state, (unsigned long)deltaMs);

  // Maintain I2S TX buffer depth (silence during startup, sine afterwards)
  i2sLoop(inStartupMute, sketch_pots_state);
}



