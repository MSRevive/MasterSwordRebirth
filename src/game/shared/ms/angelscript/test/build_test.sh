#!/bin/bash
# Build script for AngelScript test program

echo "Building AngelScript test program..."

# Create build directory
mkdir -p build_test
cd build_test

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Copy test script to build directory
cp ../../../../../../build/test_angelscript_simple.as .

echo "Build complete!"
echo "Run with: ./test_angelscript"