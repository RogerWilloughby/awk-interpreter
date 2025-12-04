#!/bin/bash
# Build script for AWK Interpreter
# Usage: ./scripts/build_all.sh [debug|release]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_TYPE="${1:-Release}"

echo "========================================"
echo "AWK Interpreter Build Script"
echo "Build type: $BUILD_TYPE"
echo "========================================"

cd "$PROJECT_DIR"

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Configure
echo ""
echo "Configuring..."
cmake -B build \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_TESTS=ON \
    -DAWK_ENABLE_LTO=ON

# Build
echo ""
echo "Building..."
cmake --build build --config "$BUILD_TYPE" -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Run tests
echo ""
echo "Running unit tests..."
./build/bin/awk_tests

# Run integration tests
echo ""
echo "Running integration tests..."
cd integration_tests
chmod +x run_tests.sh
./run_tests.sh
cd ..

echo ""
echo "========================================"
echo "Build complete!"
echo ""
echo "Executable: build/bin/awk"
echo "Library: build/lib/libawk.a"
echo "========================================"
