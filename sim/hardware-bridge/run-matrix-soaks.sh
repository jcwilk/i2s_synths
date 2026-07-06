#!/usr/bin/env bash
# Run phase-3 bridge relay soaks S1–S6 with matching firmware flashed.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BRIDGE="${BRIDGE_URL:-ws://localhost:8766}"
PORT="${BRIDGE_PORT:-8766}"
DURATION="${SOAK_DURATION_S:-600}"
ATT_DIR="$ROOT/sim/hardware-bridge"
LOG="$ATT_DIR/phase3-attestation-matrix.log"
cd "$ROOT"

stop_bridge() {
  pkill -f "hardware-bridge/server.mjs" 2>/dev/null || true
  sleep 1
}

wait_bridge() {
  for _ in $(seq 1 40); do
    if (cd "$ROOT/sim/hardware-bridge" && BRIDGE_URL="$BRIDGE" node --input-type=module -e "
      import { WebSocket } from 'ws';
      const w = new WebSocket(process.env.BRIDGE_URL);
      w.on('open', () => { w.close(); process.exit(0); });
      w.on('error', () => process.exit(1));
      setTimeout(() => process.exit(1), 1500);
    "); then
      return 0
    fi
    sleep 1
  done
  echo "Bridge not ready at $BRIDGE" >&2
  return 1
}

start_bridge() {
  stop_bridge
  FIRMWARE_PORT=/dev/ttyACM0 node sim/hardware-bridge/server.mjs --port="$PORT" >>"$LOG" 2>&1 &
  sleep 4
  BRIDGE_URL="$BRIDGE" wait_bridge
}

run_one() {
  local scenario="$1"
  local module="$2"
  {
    echo ""
    echo "=== $(date -Is) $scenario ($module) ==="
    echo "==> Flash $module"
  } | tee -a "$LOG"
  stop_bridge
  fuser -k /dev/ttyACM0 2>/dev/null || true
  sleep 2
  ./scripts/upload.sh "$module" 2>&1 | tee -a "$LOG"
  if [[ "${PIPESTATUS[0]}" -ne 0 ]]; then
    echo "Upload failed for $module" >&2 | tee -a "$LOG"
    exit 1
  fi
  sleep 8
  start_bridge
  echo "==> Soak $scenario (${DURATION}s)" | tee -a "$LOG"
  node sim/hardware-bridge/phase3-soak.mjs \
    --scenario "$scenario" \
    --duration "$DURATION" \
    --bridge "$BRIDGE" \
    | tee "$ATT_DIR/phase3-attestation-${scenario}.json"
  if [[ "${PIPESTATUS[0]}" -ne 0 ]]; then
    echo "Soak $scenario failed pass criteria" >&2 | tee -a "$LOG"
    exit 1
  fi
}

: >"$LOG"
run_one S1 passthrough
run_one S2 delay
run_one S3 merger
run_one S4 cutoff
run_one S5 debug_tone
run_one S6 delay

echo "Matrix complete — see $ATT_DIR/phase3-attestation-S*.json"
