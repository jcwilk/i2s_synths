#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck disable=SC1091
source "${ROOT}/scripts/firmware-env.sh"

usage() {
  cat <<EOF
Usage: $(basename "$0") <module|all>

Modules: passthrough, delay, merger, debug_tone, cutoff
  all     Compile every supported module variant
EOF
}

compile_module() {
  local module="$1"
  local module_define
  module_define="$(module_define_for "$module")"

  echo "==> Compiling ${module} (${module_define})"
  arduino-cli "${CLI_CONFIG[@]}" compile \
    --fqbn "$FQBN" \
    --build-property "build.extra_flags=-DACTIVE_MODULE=${module_define}" \
    "$STAGING_DIR"
}

if [[ $# -ne 1 ]]; then
  usage >&2
  exit 1
fi

require_arduino_cli
prepare_sketch_staging

target="$1"
if [[ "$target" == "all" ]]; then
  for module in passthrough delay merger debug_tone cutoff; do
    compile_module "$module"
  done
else
  compile_module "$target"
fi
