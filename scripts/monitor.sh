#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck disable=SC1091
source "${ROOT}/scripts/firmware-env.sh"

DURATION_SECONDS="${MONITOR_SECONDS:-10}"

usage() {
  cat <<EOF
Usage: $(basename "$0") [seconds]

Capture serial output from the connected board for a bounded duration.
Default duration: ${DURATION_SECONDS}s (override with MONITOR_SECONDS).
EOF
}

if [[ $# -gt 1 ]]; then
  usage >&2
  exit 1
fi

if [[ $# -eq 1 ]]; then
  DURATION_SECONDS="$1"
fi

require_arduino_cli
resolve_firmware_port

echo "==> Monitoring ${FIRMWARE_PORT} at ${SERIAL_BAUD} baud for ${DURATION_SECONDS}s"
timeout "$DURATION_SECONDS" \
  arduino-cli "${CLI_CONFIG[@]}" monitor \
  --port "$FIRMWARE_PORT" \
  --config baudrate="${SERIAL_BAUD}"
