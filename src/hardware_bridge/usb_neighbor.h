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

#endif  // HARDWARE_BRIDGE_USB_NEIGHBOR_H
