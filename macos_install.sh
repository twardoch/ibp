#!/usr/bin/env bash

export PKG_CONFIG_PATH="/usr/local/opt/lcms2/lib/pkgconfig:$PKG_CONFIG_PATH"
export PKG_CONFIG_PATH="/usr/local/opt/freeimage/lib/pkgconfig:$PKG_CONFIG_PATH"

LCMS2_PATH=$(brew --prefix lcms2)
FREEIMAGE_PATH=$(brew --prefix freeimage)
OPENCV_PATH="/usr/local/opt/opencv"

# --- Set Environment Variables for CMake ---
export LDFLAGS="-L/usr/local/opt/qt@5/lib"
export CPPFLAGS="-I/usr/local/opt/qt@5/include"
export PKG_CONFIG_PATH="/usr/local/opt/qt@5/lib/pkgconfig"

# --- Set CMake Options ---
CMAKE_OPTIONS=(
    -DCMAKE_CXX_STANDARD=17
    -DCMAKE_CXX_STANDARD_REQUIRED=ON
    -DCMAKE_CXX_EXTENSIONS=OFF
    -DCMAKE_CXX_FLAGS="-std=c++17 -DQT_USE_QSTRINGBUILDER -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x060000 -DQ_COMPILER_STDCXX=201703L -I/usr/local/include"
    -DCMAKE_EXE_LINKER_FLAGS="-L${LCMS2_PATH}/lib -L${FREEIMAGE_PATH}/lib -L${OPENCV_PATH}/lib"
    -DCMAKE_INCLUDE_PATH="/usr/local/include;${FREEIMAGE_PATH}/include;${LCMS2_PATH}/include;${OPENCV_PATH}/include"
    -DCMAKE_LIBRARY_PATH="/usr/local/lib;${FREEIMAGE_PATH}/lib;${OPENCV_PATH}/lib"
    -DCMAKE_PREFIX_PATH="/usr/local/opt/qt@5;${LCMS2_PATH};${FREEIMAGE_PATH};/usr/local/opt/itk;${OPENCV_PATH}/lib/cmake/opencv4"
    -DCMAKE_SHARED_LINKER_FLAGS="-L${FREEIMAGE_PATH}/lib -L${LCMS2_PATH}/lib -llcms2 -L${OPENCV_PATH}/lib"
    -DIBP_BUILD_PLUGINS=ON
)

# --- Build the Project ---
echo "Configuring project with CMake..."
if ! cmake . "${CMAKE_OPTIONS[@]}"; then
    echo "CMake configuration failed. Please check the error messages above."
    echo "Python paths may be incorrect. Please verify Python 3.13 installation."
    exit 1
fi

echo "Building project with Make..."
if ! make -j$(sysctl -n hw.ncpu); then
    echo "Build failed. Please check the error messages above."
    exit 1
fi

echo "Build completed successfully!"

# --- Cleanup (Optional) ---
# brew cleanup

echo "Installation and build process finished."
