#!/bin/bash
set -e

echo "=== SandBoxX Build Script ==="
echo "Configuring build directory..."

mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

echo ""
echo "=== Build Complete ==="
echo "SandBoxX binary available at: build/sandboxx"
echo "Example binaries available at: build/hello, build/cpu_bomb, build/memory_bomb, build/file_test, build/hostname_test"
