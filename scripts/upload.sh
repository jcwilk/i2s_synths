#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck disable=SC1091
source "${ROOT}/scripts/firmware-env.sh"

usage() {
  cat <<EOF
Usage: $(basename "$0") <module|all>

Builds the selected module, then flashes it to a connected ESP32-S3-Zero.

If upload fails, hold BOOT on the S3-Zero while connecting or resetting USB.
On Linux, ensure your user is in the dialout group for serial access.
EOF
}

if [[ $# -ne 1 ]]; then
  usage >&2
  exit 1
fi

require_arduino_cli
resolve_firmware_port
prepare_sketch_staging

module="$1"
module_define="$(module_define_for "$module")"

echo "==> Uploading ${module} to ${FIRMWARE_PORT}"
echo "Tip: hold BOOT on ESP32-S3-Zero if the port does not appear or upload fails."

arduino-cli "${CLI_CONFIG[@]}" compile \
  --upload \
  --port "$FIRMWARE_PORT" \
  --fqbn "$FQBN" \
  --build-property "build.extra_flags=-DACTIVE_MODULE=${module_define}" \
  "$STAGING_DIR"
