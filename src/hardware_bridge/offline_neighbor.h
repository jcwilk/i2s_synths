#ifndef HARDWARE_BRIDGE_OFFLINE_NEIGHBOR_H
#define HARDWARE_BRIDGE_OFFLINE_NEIGHBOR_H

#include <Arduino.h>
#include <string.h>
#include "../config/constants.h"
#include "../input/pots.h"
#include "bridge_frame.h"

typedef struct {
  bool active;
  bool loopback;
  uint32_t sequence;
  float primaryControl;
  float secondaryControl;
  int16_t downstreamIn[BUFFER_LEN];
  int16_t upstreamIn[BUFFER_LEN];
  int16_t downstreamOut[BUFFER_LEN];
  int16_t upstreamOut[BUFFER_LEN];
  uint16_t lastStatus;
} OfflineNeighborState;

inline OfflineNeighborState offlineNeighborMakeInitial() {
  OfflineNeighborState state = {};
  state.lastStatus = BRIDGE_STATUS_OK;
  return state;
}

inline float offlineClampControl01(float value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  if (value > 1.0f) {
    return 1.0f;
  }
  return value;
}

// Injected controls bypass ADC; raw and smoothed are set to the host value.
inline DualPotsState offlineInjectedPots(float primaryControl, float secondaryControl) {
  DualPotsState pots = potsMakeInitial(POT_PIN_PRIMARY, POT_PIN_SECONDARY);
  const float primary = offlineClampControl01(primaryControl);
  const float secondary = offlineClampControl01(secondaryControl);
  pots.primary.raw = primary;
  pots.primary.smoothed = primary;
  pots.secondary.raw = secondary;
  pots.secondary.smoothed = secondary;
  return pots;
}

inline OfflineNeighborState offlineNeighborEnter(OfflineNeighborState state) {
  state.active = true;
  state.loopback = false;
  moduleSetup();
  state.lastStatus = BRIDGE_STATUS_OK;
  Serial.println("offline neighbor: enter");
  return state;
}

inline OfflineNeighborState offlineNeighborExit(OfflineNeighborState state) {
  state.active = false;
  state.loopback = false;
  state.lastStatus = BRIDGE_STATUS_OK;
  Serial.println("offline neighbor: exit");
  return state;
}

inline OfflineNeighborState offlineNeighborEnableLoopback(OfflineNeighborState state) {
  state.loopback = true;
  state.lastStatus = BRIDGE_STATUS_OK;
  Serial.println("offline neighbor: loopback self-test");
  return state;
}

inline OfflineNeighborState offlineNeighborDisableLoopback(OfflineNeighborState state) {
  state.loopback = false;
  state.lastStatus = BRIDGE_STATUS_OK;
  return state;
}

inline OfflineNeighborState offlineNeighborLoadRequest(OfflineNeighborState state,
                                                       const BridgeExchangeRequest& request) {
  state.sequence = request.sequence;
  state.primaryControl = request.primaryControl;
  state.secondaryControl = request.secondaryControl;
  memcpy(state.downstreamIn, request.downstreamIn, sizeof(state.downstreamIn));
  memcpy(state.upstreamIn, request.upstreamIn, sizeof(state.upstreamIn));
  return state;
}

inline OfflineNeighborState offlineNeighborProcessExchange(OfflineNeighborState state) {
  if (!state.active) {
    state.lastStatus = BRIDGE_STATUS_ERR_NOT_ACTIVE;
    return state;
  }

  if (state.loopback) {
    memcpy(state.downstreamOut, state.downstreamIn, sizeof(state.downstreamOut));
    memcpy(state.upstreamOut, state.upstreamIn, sizeof(state.upstreamOut));
    state.lastStatus = BRIDGE_STATUS_OK;
    return state;
  }

  const DualPotsState pots = offlineInjectedPots(state.primaryControl, state.secondaryControl);
  moduleLoopUpstream(state.upstreamIn, state.upstreamOut, BUFFER_LEN, pots);
  moduleLoopDownstream(state.downstreamIn, state.downstreamOut, BUFFER_LEN, pots);
  state.lastStatus = BRIDGE_STATUS_OK;
  return state;
}

inline BridgeExchangeResponse offlineNeighborBuildResponse(const OfflineNeighborState& state) {
  BridgeExchangeResponse response = {};
  response.sequence = state.sequence;
  response.status = state.lastStatus;
  memcpy(response.downstreamOut, state.downstreamOut, sizeof(response.downstreamOut));
  memcpy(response.upstreamOut, state.upstreamOut, sizeof(response.upstreamOut));
  return response;
}

#endif  // HARDWARE_BRIDGE_OFFLINE_NEIGHBOR_H
