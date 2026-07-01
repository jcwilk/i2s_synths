#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SHIM="$ROOT/sim/shim"
SRC="$ROOT/src"
OUT="$ROOT/sim/wasm/out"
HARNESS="$ROOT/sim/wasm/harness.cpp"

if ! command -v emcc >/dev/null 2>&1; then
  if [[ -f "$HOME/emsdk/emsdk_env.sh" ]]; then
    # shellcheck disable=SC1091
    source "$HOME/emsdk/emsdk_env.sh"
  else
    echo "Emscripten (emcc) not found. Install emsdk and source emsdk_env.sh." >&2
    exit 1
  fi
fi

mkdir -p "$OUT"

COMMON_FLAGS=(
  "$HARNESS"
  "-I$SHIM"
  "-I$SRC"
  "-DSIM_BUILD=1"
  "-O2"
  "-s" "INITIAL_MEMORY=33554432"
  "-s" "ALLOW_MEMORY_GROWTH=1"
  "-s" "MODULARIZE=1"
  "-s" "EXPORT_ES6=1"
  "-s" "EXPORT_NAME=createSimModule"
  "-s" "ENVIRONMENT=web"
  "-s" "EXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\",\"HEAP16\",\"HEAPF32\"]"
)

build_variant() {
  local name="$1"
  local module_id="$2"
  local exports="_sim_setup,_sim_process_upstream,_sim_process_downstream,_sim_get_upstream_in,_sim_get_upstream_out,_sim_get_downstream_in,_sim_get_downstream_out,_sim_get_pots,_sim_get_buffer_len"
  if [[ "$name" == "delay" ]]; then
    exports="${exports},_sim_get_delay_buffer_frames"
  fi
  echo "Building WASM: $name (ACTIVE_MODULE=$module_id)"
  emcc "${COMMON_FLAGS[@]}" \
    -DACTIVE_MODULE="$module_id" \
    -s "EXPORTED_FUNCTIONS=[$exports]" \
    -o "$OUT/${name}.js"
}

build_variant passthrough 0
build_variant delay 1
build_variant merger 2
build_variant debug_tone 3
build_variant cutoff 4

echo "WASM artifacts written to $OUT"
