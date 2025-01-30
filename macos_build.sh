#!/usr/bin/env bash

# Exit on error
set -e

export PKG_CONFIG_PATH="/usr/local/opt/lcms2/lib/pkgconfig:$PKG_CONFIG_PATH"
export PKG_CONFIG_PATH="/usr/local/opt/freeimage/lib/pkgconfig:$PKG_CONFIG_PATH"

# Store the original directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

LCMS2_PATH=$(brew --prefix lcms2)
FREEIMAGE_PATH=$(brew --prefix freeimage)
OPENCV_PATH="$(brew --prefix opencv)"
QT_PATH="$(brew --prefix qt@5)"

# --- Set Environment Variables for CMake ---
export LDFLAGS="-L${QT_PATH}/lib"
export CPPFLAGS="-I${QT_PATH}/include"
export PKG_CONFIG_PATH="${QT_PATH}/lib/pkgconfig"

# --- Set CMake Options ---
CMAKE_OPTIONS=(
    -G "Unix Makefiles"
    -B "${BUILD_DIR}"
    -S "${SCRIPT_DIR}"
    -DCMAKE_CXX_STANDARD=17
    -DCMAKE_CXX_STANDARD_REQUIRED=ON
    -DCMAKE_CXX_EXTENSIONS=OFF
    -DCMAKE_CXX_FLAGS="-std=c++17 -DQT_USE_QSTRINGBUILDER -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x060000 -DQ_COMPILER_STDCXX=201703L -I/usr/local/include"
    -DCMAKE_EXE_LINKER_FLAGS="-L${LCMS2_PATH}/lib -L${FREEIMAGE_PATH}/lib -L${OPENCV_PATH}/lib"
    -DCMAKE_INCLUDE_PATH="/usr/local/include;${FREEIMAGE_PATH}/include;${LCMS2_PATH}/include;${OPENCV_PATH}/include"
    -DCMAKE_LIBRARY_PATH="/usr/local/lib;${FREEIMAGE_PATH}/lib;${OPENCV_PATH}/lib"
    -DCMAKE_PREFIX_PATH="${QT_PATH};${LCMS2_PATH};${FREEIMAGE_PATH};/usr/local/opt/itk;${OPENCV_PATH}/lib/cmake/opencv4"
    -DCMAKE_SHARED_LINKER_FLAGS="-L${FREEIMAGE_PATH}/lib -L${LCMS2_PATH}/lib -llcms2 -L${OPENCV_PATH}/lib"
    -DCMAKE_INSTALL_RPATH="@loader_path/../lib;@executable_path/../Frameworks"
    -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
    -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON
    -DCMAKE_SKIP_BUILD_RPATH=FALSE
    -DCMAKE_BUILD_WITH_INSTALL_RPATH=FALSE
    -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE
    -DIBP_BUILD_PLUGINS=ON
)

# Clean build directory
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# --- Build the Project ---
echo "Configuring project with CMake..."
if ! cmake "${CMAKE_OPTIONS[@]}"; then
    echo "CMake configuration failed. Please check the error messages above."
    exit 1
fi

echo "Building project with Make..."
if ! cmake --build "${BUILD_DIR}" -j$(sysctl -n hw.ncpu); then
    echo "Build failed. Please check the error messages above."
    exit 1
fi

echo "Build completed successfully!"

# --- Cleanup (Optional) ---
# brew cleanup

echo "Installation and build process finished."
