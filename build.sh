#!/usr/bin/env bash
# Usage: ./build.sh [Debug|Release]
set -euo pipefail

cd "$(dirname "$0")"
BUILD_TYPE="${1:-Debug}"

cmake -S . -B "build/${BUILD_TYPE}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
cmake --build "build/${BUILD_TYPE}" -j"$(nproc)"

echo
echo "Executable: build/${BUILD_TYPE}/SmallObjectAllocator"
