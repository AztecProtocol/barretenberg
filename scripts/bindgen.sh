#!/bin/bash
set -eu

(cd ./cpp && cmake --build --preset wasm)
find ./cpp/src -type f -name "c_bind*.hpp" | grep pedersen_.*/c_bind.hpp | ./scripts/decls_json.py > exports.json
(cd ./ts && yarn ts-node-esm ./src/bindgen/index.ts ../exports.json > ./src/bindings/index.ts)
