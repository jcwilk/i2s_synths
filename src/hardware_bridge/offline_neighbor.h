#ifndef HARDWARE_BRIDGE_OFFLINE_NEIGHBOR_H
#define HARDWARE_BRIDGE_OFFLINE_NEIGHBOR_H

#include <Arduino.h>
#include <string.h>
#include "../config/constants.h"
#include "../input/pots.h"
#include "bridge_audio.h"
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

inline void offlineNeighborMakeInitial(OfflineNeighborState& state) {
  memset(&state, 0, sizeof(state));
  state.lastStatus = BRIDGE_STATUS_OK;
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

inline void offlineNeighborEnter(OfflineNeighborState& state) {
  state.active = true;
  state.loopback = false;
  moduleSetup();
  state.lastStatus = BRIDGE_STATUS_OK;
  bridgeLog("offline neighbor: enter");
}

inline void offlineNeighborExit(OfflineNeighborState& state) {
  state.active = false;
  state.loopback = false;
  state.lastStatus = BRIDGE_STATUS_OK;
  bridgeLog("offline neighbor: exit");
}

inline void offlineNeighborEnableLoopback(OfflineNeighborState& state) {
  state.loopback = true;
  state.lastStatus = BRIDGE_STATUS_OK;
  bridgeLog("offline neighbor: loopback self-test");
}

inline void offlineNeighborDisableLoopback(OfflineNeighborState& state) {
  state.loopback = false;
  state.lastStatus = BRIDGE_STATUS_OK;
}

inline void offlineNeighborLoadRequest(OfflineNeighborState& state,
                                       const BridgeExchangeRequest& request) {
  state.sequence = request.sequence;
  state.primaryControl = request.primaryControl;
  state.secondaryControl = request.secondaryControl;
  memcpy(state.downstreamIn, request.downstreamIn, sizeof(state.downstreamIn));
  memcpy(state.upstreamIn, request.upstreamIn, sizeof(state.upstreamIn));
}

inline void offlineNeighborProcessExchange(OfflineNeighborState& state) {
  if (!state.active) {
    state.lastStatus = BRIDGE_STATUS_ERR_NOT_ACTIVE;
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

inline void offlineNeighborBuildResponse(const OfflineNeighborState& state,
                                         BridgeExchangeResponse& response) {
  memset(&response, 0, sizeof(response));
  response.sequence = state.sequence;
  response.status = state.lastStatus;
  response.timestampUs = 0;
  memcpy(response.downstreamOut, state.downstreamOut, sizeof(response.downstreamOut));
  memcpy(response.upstreamOut, state.upstreamOut, sizeof(response.upstreamOut));
}

#endif  // HARDWARE_BRIDGE_OFFLINE_NEIGHBOR_H
