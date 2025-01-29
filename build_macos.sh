#!/usr/bin/env bash

# Script to install prerequisites and build the Image Batch Processor (IBP) project on macOS.

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Install Homebrew (if not already installed) ---
if ! command -v brew &>/dev/null; then
    echo "Homebrew not found. Installing..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
    echo "Homebrew already installed."
fi

# --- Install Required Packages ---
echo "Installing required packages using Homebrew..."
brew update
brew install cmake qt@5 lcms2 freeimage opencv eigen

# Link qt@5 if necessary (it's keg-only)
if ! ls /usr/local/opt/qt@5 >/dev/null; then
    echo "Linking qt@5..."
    brew link --force qt@5
fi

# --- Set Environment Variables for CMake ---
export LDFLAGS="-L/usr/local/opt/qt@5/lib"
export CPPFLAGS="-I/usr/local/opt/qt@5/include"
export PKG_CONFIG_PATH="/usr/local/opt/qt@5/lib/pkgconfig"

# --- Set CMake Options ---
CMAKE_OPTIONS=(
    -DCMAKE_CXX_STANDARD=17
    -DCMAKE_CXX_STANDARD_REQUIRED=ON
    -DCMAKE_CXX_EXTENSIONS=OFF
    -DCMAKE_CXX_FLAGS="-std=c++17 -DQT_USE_QSTRINGBUILDER -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x060000 -DQ_COMPILER_STDCXX=201703L"
    -DCMAKE_EXE_LINKER_FLAGS="-L/usr/local/opt/lcms2/lib -L/usr/local/Cellar/freeimage/3.18.0/lib"
    -DCMAKE_INCLUDE_PATH="/usr/local/include;/usr/local/Cellar/freeimage/3.18.0/include"
    -DCMAKE_LIBRARY_PATH="/usr/local/lib;/usr/local/Cellar/freeimage/3.18.0/lib"
    -DCMAKE_PREFIX_PATH="/usr/local/opt/qt@5;/usr/local/opt/lcms2;/usr/local/Cellar/freeimage/3.18.0"
    -DCMAKE_SHARED_LINKER_FLAGS="-L/usr/local/Cellar/freeimage/3.18.0/lib -L/usr/local/opt/lcms2/lib -llcms2"
    -DIBP_BUILD_PLUGINS=ON
)

# --- Build the Project ---
echo "Configuring project with CMake..."
cmake . "${CMAKE_OPTIONS[@]}"

echo "Building project with Make..."
make -j$(sysctl -n hw.ncpu)

echo "Build completed successfully!"

# --- Cleanup (Optional) ---
# brew cleanup

echo "Installation and build process finished."
