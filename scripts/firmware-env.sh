#!/usr/bin/env bash
# Shared firmware build settings (sourced by build/upload/monitor scripts).

set -euo pipefail

if [[ -z "${ROOT:-}" ]]; then
  ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fi

CONFIG_FILE="${ROOT}/arduino-cli.yaml"
CLI_CONFIG=(--config-file "$CONFIG_FILE")

read_firmware_yaml_value() {
  local key="$1"
  grep -E "^[[:space:]]*${key}:" "$CONFIG_FILE" | head -1 | sed -E 's/^[[:space:]]*[^:]+:[[:space:]]*"?([^"#]+)"?.*/\1/' | tr -d ' '
}

FQBN="$(read_firmware_yaml_value fqbn)"
SKETCH_NAME="$(read_firmware_yaml_value sketch_name)"
SERIAL_BAUD="$(read_firmware_yaml_value serial_baud)"

if [[ -z "$FQBN" || -z "$SKETCH_NAME" || -z "$SERIAL_BAUD" ]]; then
  echo "Missing firmware_build settings in ${CONFIG_FILE}" >&2
  exit 1
fi

STAGING_DIR="${ROOT}/.build-sketch/${SKETCH_NAME}"
SKETCH_INO="${ROOT}/${SKETCH_NAME}.ino"

if [[ ! -f "$SKETCH_INO" ]]; then
  echo "Sketch not found: ${SKETCH_INO}" >&2
  exit 1
fi

prepare_sketch_staging() {
  local module_define="${1:-}"

  rm -rf "$STAGING_DIR"
  mkdir -p "$STAGING_DIR"
  cp "$SKETCH_INO" "${STAGING_DIR}/${SKETCH_NAME}.ino"
  ln -s "${ROOT}/src" "${STAGING_DIR}/src"

  # Per-module defines go in build_opt.h so we do not override build.extra_flags
  # (which would strip ARDUINO_USB_* and route Serial to UART0 instead of USB CDC).
  if [[ -n "$module_define" ]]; then
    printf '%s\n' "-DACTIVE_MODULE=${module_define}" "-DBRIDGE_USE_SERIAL_FOR_AUDIO=1" > "${STAGING_DIR}/build_opt.h"
  fi
}

require_arduino_cli() {
  if ! command -v arduino-cli >/dev/null 2>&1; then
    echo "arduino-cli not found. Install from https://arduino.github.io/arduino-cli/" >&2
    exit 1
  fi
}

load_firmware_port() {
  FIRMWARE_PORT="${FIRMWARE_PORT:-}"

  if [[ -f "${ROOT}/.env" ]]; then
    # shellcheck disable=SC1091
    set -a
    source "${ROOT}/.env"
    set +a
  fi

  FIRMWARE_PORT="${FIRMWARE_PORT:-}"
}

resolve_firmware_port() {
  load_firmware_port

  if [[ -n "$FIRMWARE_PORT" ]]; then
    if [[ ! -e "$FIRMWARE_PORT" ]]; then
      echo "FIRMWARE_PORT=${FIRMWARE_PORT} does not exist." >&2
      exit 1
    fi
    return 0
  fi

  local candidates=()
  local dev
  shopt -s nullglob
  for dev in /dev/ttyACM* /dev/ttyUSB*; do
    candidates+=("$dev")
  done
  shopt -u nullglob

  if [[ ${#candidates[@]} -eq 0 ]]; then
    echo "No serial port found. Connect an ESP32-S3-Zero or set FIRMWARE_PORT in .env (see .env.example)." >&2
    exit 1
  fi

  if [[ ${#candidates[@]} -gt 1 ]]; then
    echo "Multiple serial ports found: ${candidates[*]}" >&2
    echo "Set FIRMWARE_PORT in .env to select one (see .env.example)." >&2
    exit 1
  fi

  FIRMWARE_PORT="${candidates[0]}"
}

module_define_for() {
  local module="$1"
  case "$module" in
    passthrough) echo "MODULE_PASSTHROUGH" ;;
    delay) echo "MODULE_DELAY" ;;
    merger) echo "MODULE_MERGER" ;;
    debug_tone) echo "MODULE_DEBUG_TONE" ;;
    cutoff) echo "MODULE_CUTOFF" ;;
    *)
      echo "Unknown module: ${module}" >&2
      echo "Supported: passthrough, delay, merger, debug_tone, cutoff, all" >&2
      exit 1
      ;;
  esac
}
