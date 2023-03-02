#!/bin/bash
set -eu

# 1) Install emscripten.
EMSCRIPTEN_VERSION="3.1.32"

if [ ! -d emsdk ]; then
  echo "Cloning emscripten emsdk..."
  git clone https://github.com/emscripten-core/emsdk.git
fi
pushd emsdk >/dev/null
# ensure we use a working commit
git checkout 17f6a2e &>/dev/null
echo "Activating Emscripten version $EMSCRIPTEN_VERSION"
./emsdk install "$EMSCRIPTEN_VERSION" >/dev/null
./emsdk activate "$EMSCRIPTEN_VERSION" >/dev/null # Activate the desired version
export EMSDK_QUIET=1
source ./emsdk_env.sh
popd >/dev/null

# 2) Build WASM.
mkdir -p build-wasm 
pushd build-wasm >/dev/null
emcmake cmake ..
cmake --build . --parallel --target barretenberg.wasm
popd >/dev/null