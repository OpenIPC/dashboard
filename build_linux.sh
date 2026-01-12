#!/bin/bash

# OpenIPC Dashboard - Linux Build Script

# 1. Check for required tools
if ! command -v cmake &> /dev/null; then
    echo "Error: cmake is required but not installed."
    exit 1
fi

if ! command -v pkg-config &> /dev/null; then
    echo "Error: pkg-config is required but not installed."
    exit 1
fi

# 2. Check for dependencies
# Note: Users need Qt6, libvlc, and potentially others.
# This is just a basic check.

echo "Checking dependencies..."
if pkg-config --exists libvlc; then
    echo " - libvlc found."
else
    echo " - Warning: libvlc not found via pkg-config. Build may fail."
    echo "   On Debian/Ubuntu: sudo apt install libvlc-dev libvlccore-dev"
fi

if pkg-config --exists Qt6Core Qt6Quick; then
    echo " - Qt6 found."
else
    echo " - Warning: Qt6 not found via pkg-config. Make sure it's in your PATH or installed."
    echo "   On Debian/Ubuntu: sudo apt install qt6-base-dev qt6-declarative-dev"
fi

# 3. Create build directory
if [ -d "build_linux" ]; then
    echo "Cleaning previous build..."
    rm -rf build_linux/*
else
    mkdir build_linux
fi

cd build_linux

# 4. Configure
echo "Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 5. Build
echo "Building..."
cmake --build . --config Release -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful! Run ./appOpenIPC-Dashboard"
else
    echo "Build failed."
    exit 1
fi
