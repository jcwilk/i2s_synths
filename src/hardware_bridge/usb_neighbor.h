#ifndef HARDWARE_BRIDGE_USB_NEIGHBOR_H
#define HARDWARE_BRIDGE_USB_NEIGHBOR_H

#include <Arduino.h>
#include <esp_timer.h>
#include <string.h>
#include "../input/pots.h"
#include "../streaming/i2s_interfaces.h"
#include "bridge_audio.h"
#include "bridge_frame.h"
#include "offline_neighbor.h"

typedef struct {
  bool active;
  bool loopback;
  bool processing;
  bool hasQueued;
  bool pendingOverrun;
  uint32_t sequence;
  uint32_t lastProcessedSequence;
  bool hasLastSequence;
  uint64_t streamStartUs;
  float primaryControl;
  float secondaryControl;
  int16_t downstreamIn[BUFFER_LEN];
  int16_t upstreamIn[BUFFER_LEN];
  int16_t downstreamOut[BUFFER_LEN];
  int16_t upstreamOut[BUFFER_LEN];
  BridgeExchangeRequest queuedRequest;
  uint16_t lastStatus;
} UsbNeighborState;

inline void usbNeighborMakeInitial(UsbNeighborState& state) {
  memset(&state, 0, sizeof(state));
  state.lastStatus = BRIDGE_STATUS_OK;
}

inline void usbNeighborEnter(UsbNeighborState& state) {
  state.active = true;
  state.loopback = false;
  state.processing = false;
  state.hasQueued = false;
  state.pendingOverrun = false;
  state.hasLastSequence = false;
  state.streamStartUs = (uint64_t)esp_timer_get_time();
  i2sNeighborSuspend();
  moduleSetup();
  state.lastStatus = BRIDGE_STATUS_OK;
  bridgeLog("usb neighbor: enter");
}

inline void usbNeighborExit(UsbNeighborState& state) {
  state.active = false;
  state.loopback = false;
  state.processing = false;
  state.hasQueued = false;
  state.pendingOverrun = false;
  state.hasLastSequence = false;
  i2sNeighborResume();
  state.lastStatus = BRIDGE_STATUS_OK;
  bridgeLog("usb neighbor: exit");
}

inline void usbNeighborEnableLoopback(UsbNeighborState& state) {
  state.loopback = true;
  state.lastStatus = BRIDGE_STATUS_OK;
  bridgeLog("usb neighbor: loopback self-test");
}

inline void usbNeighborDisableLoopback(UsbNeighborState& state) {
  state.loopback = false;
  state.lastStatus = BRIDGE_STATUS_OK;
}

inline void usbNeighborLoadRequest(UsbNeighborState& state, const BridgeExchangeRequest& request) {
  state.sequence = request.sequence;
  state.primaryControl = request.primaryControl;
  state.secondaryControl = request.secondaryControl;
  memcpy(state.downstreamIn, request.downstreamIn, sizeof(state.downstreamIn));
  memcpy(state.upstreamIn, request.upstreamIn, sizeof(state.upstreamIn));
}


inline bool usbNeighborPopQueued(UsbNeighborState& state) {
  if (!state.hasQueued) {
    state.processing = false;
    return false;
  }
  usbNeighborLoadRequest(state, state.queuedRequest);
  state.hasQueued = false;
  return true;
}

inline uint16_t usbNeighborEvaluateSequence(UsbNeighborState& state) {
  if (!state.hasLastSequence) {
    state.hasLastSequence = true;
    state.lastProcessedSequence = state.sequence;
    return BRIDGE_STATUS_OK;
  }
  const uint32_t expected = state.lastProcessedSequence + 1u;
  if (state.sequence != expected) {
    return (uint16_t)(BRIDGE_STATUS_SEQ_GAP | BRIDGE_STATUS_ERR_BAD_CMD);
  }
  state.lastProcessedSequence = state.sequence;
  return BRIDGE_STATUS_OK;
}

inline void usbNeighborProcessExchange(UsbNeighborState& state) {
  if (!state.active) {
    state.lastStatus = BRIDGE_STATUS_ERR_NOT_ACTIVE;
    return;
  }

  uint16_t status = usbNeighborEvaluateSequence(state);
  if (status != BRIDGE_STATUS_OK) {
    state.lastStatus = status;
    return;
  }

  if (state.loopback) {
    memcpy(state.downstreamOut, state.downstreamIn, sizeof(state.downstreamOut));
    memcpy(state.upstreamOut, state.upstreamIn, sizeof(state.upstreamOut));
    state.lastStatus = BRIDGE_STATUS_OK;
    return;
  }

  const DualPotsState pots = offlineInjectedPots(state.primaryControl, state.secondaryControl);
  moduleLoopUpstream(state.upstreamIn, state.upstreamOut, BUFFER_LEN, pots);
  moduleLoopDownstream(state.downstreamIn, state.downstreamOut, BUFFER_LEN, pots);
  state.lastStatus = BRIDGE_STATUS_OK;
}

inline void usbNeighborBuildResponse(const UsbNeighborState& state, BridgeExchangeResponse& response) {
  memset(&response, 0, sizeof(response));
  response.sequence = state.sequence;
  response.status = state.lastStatus;
  response.timestampUs = (uint32_t)((uint64_t)esp_timer_get_time() - state.streamStartUs);
  memcpy(response.downstreamOut, state.downstreamOut, sizeof(response.downstreamOut));
  memcpy(response.upstreamOut, state.upstreamOut, sizeof(response.upstreamOut));
}

inline uint16_t usbNeighborProcessPack4Exchange(UsbNeighborState& state,
                                                uint32_t baseSequence,
                                                const int16_t* downstreamIn,
                                                const int16_t* upstreamIn,
                                                int16_t* downstreamOut,
                                                int16_t* upstreamOut) {
  if (!state.active) {
    return BRIDGE_STATUS_ERR_NOT_ACTIVE;
  }

  const DualPotsState pots = offlineInjectedPots(state.primaryControl, state.secondaryControl);
  for (uint32_t period = 0; period < BRIDGE_USB_PACK_PERIODS; ++period) {
    state.sequence = baseSequence + period;
    uint16_t status = usbNeighborEvaluateSequence(state);
    if (status != BRIDGE_STATUS_OK) {
      return status;
    }

    const size_t offset = (size_t)period * (size_t)BUFFER_LEN;
    int16_t* dsIn = const_cast<int16_t*>(downstreamIn + offset);
    int16_t* usIn = const_cast<int16_t*>(upstreamIn + offset);
    int16_t* dsOut = downstreamOut + offset;
    int16_t* usOut = upstreamOut + offset;

    if (state.loopback) {
      memcpy(dsOut, dsIn, BRIDGE_AUDIO_SAMPLE_BYTES);
      memcpy(usOut, usIn, BRIDGE_AUDIO_SAMPLE_BYTES);
      continue;
    }

    moduleLoopUpstream(usIn, usOut, BUFFER_LEN, pots);
    moduleLoopDownstream(dsIn, dsOut, BUFFER_LEN, pots);
  }

  state.lastStatus = BRIDGE_STATUS_OK;
  return BRIDGE_STATUS_OK;
}

inline uint16_t usbNeighborProcess22kMonoExchange(UsbNeighborState& state,
                                                  uint32_t sequence,
                                                  const int16_t* downstreamIn,
                                                  const int16_t* upstreamIn,
                                                  int16_t* downstreamOut,
                                                  int16_t* upstreamOut) {
  if (!state.active) {
    return BRIDGE_STATUS_ERR_NOT_ACTIVE;
  }

  state.sequence = sequence;
  const uint16_t seqStatus = usbNeighborEvaluateSequence(state);
  if (seqStatus != BRIDGE_STATUS_OK) {
    return seqStatus;
  }

  const DualPotsState pots = offlineInjectedPots(state.primaryControl, state.secondaryControl);
  const int samples = (int)BRIDGE_SPIKE_MONO_BUFFER_LEN;

  if (state.loopback) {
    memcpy(downstreamOut, downstreamIn, BRIDGE_SPIKE_MONO_AUDIO_BYTES);
    memcpy(upstreamOut, upstreamIn, BRIDGE_SPIKE_MONO_AUDIO_BYTES);
    state.lastStatus = BRIDGE_STATUS_OK;
    return BRIDGE_STATUS_OK;
  }

  moduleLoopUpstream(const_cast<int16_t*>(upstreamIn), upstreamOut, samples, pots);
  moduleLoopDownstream(const_cast<int16_t*>(downstreamIn), downstreamOut, samples, pots);
  state.lastStatus = BRIDGE_STATUS_OK;
  return BRIDGE_STATUS_OK;
}

#endif  // HARDWARE_BRIDGE_USB_NEIGHBOR_H
