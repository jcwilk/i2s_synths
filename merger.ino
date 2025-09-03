// Merger module for audio processing

#if ACTIVE_MODULE == MODULE_MERGER

// --- Compressor Configuration ---
#define MAX_SIGNAL_LEVEL 24576 // Max signal level before compression (approx. 75% of int16_t max)
#define COMPRESSOR_RELEASE_RATE 0.99995f // Rate at which compression ratio returns to 1.0

// I2S1 is fully configured in synths.ino; no pin config here

// --- Potentiometer Configuration ---
// Pins are defined globally in synths.ino so all modules share the same assignments

// Compressor and Potentiometer state
float currentScaleRatio = 1.0f;
float emaPrimary = 0.0f;
float emaSecondary = 0.0f;
// No local rx buffer; secondary buffers are managed by synths.ino

// --- Upstream Ring Buffer (for merger) ---
// Stores upstream samples to be optionally mixed into downstream per secondary potentiometer
#define MERGER_RING_CAPACITY (BUFFER_LEN * 64) // 64 frames of 128 samples -> 8192 samples (~186 ms at 44.1kHz mono-sample stream)
static int16_t mergerRing[MERGER_RING_CAPACITY];
static uint32_t ringHead = 0;           // index of oldest sample
static uint32_t ringTail = 0;           // index one past newest sample
static uint32_t ringCountSamples = 0;   // number of valid samples in buffer
static volatile bool flagOverrunEvent = false;

static inline uint32_t ringSpace() {
  return (uint32_t)(MERGER_RING_CAPACITY - ringCountSamples);
}

static inline void ringDropOldest(uint32_t dropCount) {
  if (dropCount == 0) return;
  if (dropCount > ringCountSamples) dropCount = ringCountSamples;
  ringHead += dropCount;
  if (ringHead >= MERGER_RING_CAPACITY) ringHead -= MERGER_RING_CAPACITY;
  ringCountSamples -= dropCount;
}

static inline void ringPushSamples(const int16_t* src, int count) {
  if (count <= 0) return;
  uint32_t space = ringSpace();
  if (space < (uint32_t)count) {
    // Overrun: not enough space to store new samples. Drop oldest to make space.
    flagOverrunEvent = true;
    uint32_t needToFree = (uint32_t)count - space;
    ringDropOldest(needToFree);
  }

  // Now we have enough contiguous logical space (may wrap physically)
  int first = (int)min((uint32_t)count, (uint32_t)(MERGER_RING_CAPACITY - ringTail));
  memcpy(&mergerRing[ringTail], src, first * sizeof(int16_t));
  int remaining = count - first;
  if (remaining > 0) {
    memcpy(&mergerRing[0], src + first, remaining * sizeof(int16_t));
  }
  ringTail += (uint32_t)count;
  if (ringTail >= MERGER_RING_CAPACITY) ringTail -= MERGER_RING_CAPACITY;
  ringCountSamples += (uint32_t)count;
}

static inline int ringPopSamples(int16_t* dst, int count) {
  if (count <= 0) return 0;
  int avail = (int)ringCountSamples;
  int toRead = avail < count ? avail : count;
  if (toRead <= 0) return 0;

  int first = (int)min((uint32_t)toRead, (uint32_t)(MERGER_RING_CAPACITY - ringHead));
  memcpy(dst, &mergerRing[ringHead], first * sizeof(int16_t));
  int remaining = toRead - first;
  if (remaining > 0) {
    memcpy(dst + first, &mergerRing[0], remaining * sizeof(int16_t));
  }
  ringHead += (uint32_t)toRead;
  if (ringHead >= MERGER_RING_CAPACITY) ringHead -= MERGER_RING_CAPACITY;
  ringCountSamples -= (uint32_t)toRead;
  return toRead;
}

static inline void ringPrefillSilence(int count) {
  if (count <= 0) return;
  static int16_t zeros[BUFFER_LEN];
  memset(zeros, 0, sizeof(zeros));
  while (count > 0) {
    int chunk = count > BUFFER_LEN ? BUFFER_LEN : count;
    ringPushSamples(zeros, chunk);
    count -= chunk;
  }
}


// No I2S1 setup here; handled in synths.ino

void moduleSetup() {
  // I2S1 already initialized in synths.ino
  // Set ADC attenuation for 0-3.3V range
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  // Initialize ring buffer state
  ringHead = 0;
  ringTail = 0;
  ringCountSamples = 0;
  flagOverrunEvent = false;
  memset(mergerRing, 0, sizeof(mergerRing));

  Serial.println("Merger module initialized");
}

void moduleLoopUpstream(int16_t* inputBuffer,
                        int16_t* outputBuffer,
                        int samplesLength) {
  // Upstream: passthrough AND enqueue samples into ring for downstream use
  if (samplesLength > 0) {
    memcpy(outputBuffer, inputBuffer, samplesLength * sizeof(int16_t));
    ringPushSamples(inputBuffer, samplesLength);
  }

  // Handle any overrun event resulting from enqueue
  if (flagOverrunEvent) {
    flagOverrunEvent = false;
    neopixelSetTimedColor(25, 0, 0, 500, NEOPIXEL_MODE_HOLD); // red flash for overrun
  }
}

void moduleLoopDownstream(int16_t* inputBuffer,
                          int16_t* outputBuffer,
                          int samplesLength) {
  // Read both potentiometers for mix control
  float primaryCoeff = readPotWithSmoothingAndDeadZone(POT_PIN_PRIMARY, emaPrimary);
  float secondaryCoeff = readPotWithSmoothingAndDeadZone(POT_PIN_SECONDARY, emaSecondary);
  // Allow up to 110% gain on secondary at max to slightly boost overall volume
  secondaryCoeff *= 1.10f;
  if (secondaryCoeff > 1.10f) secondaryCoeff = 1.10f;

  // Downstream path: mix downstream input (scaled by primary) with upstream data
  // pulled from the ring buffer (scaled by secondary). Apply simple compressor on the sum.
  int underrunMissing = 0;
  for (int i = 0; i < samplesLength; i++) {
    int16_t upstreamSample = 0;
    int popped = ringPopSamples(&upstreamSample, 1);
    if (popped != 1) {
      underrunMissing++;
      upstreamSample = 0; // substitute silence for missing upstream sample
    }

    int32_t merged_sample = (int32_t)(inputBuffer[i] * primaryCoeff) + (int32_t)(upstreamSample * secondaryCoeff);

    if (abs(merged_sample) > MAX_SIGNAL_LEVEL) {
      currentScaleRatio = (float)MAX_SIGNAL_LEVEL / abs(merged_sample);
    } else {
      currentScaleRatio += (1.0f - currentScaleRatio) * (1.0f - COMPRESSOR_RELEASE_RATE);
    }

    merged_sample = (int32_t)(merged_sample * currentScaleRatio);
    if (merged_sample > 32767) merged_sample = 32767;
    if (merged_sample < -32768) merged_sample = -32768;
    outputBuffer[i] = (int16_t)merged_sample;
  }

  // If we encountered underrun, indicate and try to recover by padding buffer with silence
  if (underrunMissing > 0) {
    neopixelSetTimedColor(25, 25, 0, 500, NEOPIXEL_MODE_HOLD); // yellow flash for underrun
    ringPrefillSilence(BUFFER_LEN); // add a frame of silence to help stabilize buffer
  }
}

#endif
