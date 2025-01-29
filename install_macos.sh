#!/usr/bin/env bash

# Script to install prerequisites and build the Image Batch Processor (IBP) project on macOS.

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Helper Functions ---
detect_python() {
    local python_path=$(uv python list --no-python-downloads | grep "3.13" | awk '{print $2}')
    if [ -n "$python_path" ] && [ -x "$python_path" ]; then
        echo "Python 3.13 found at: $python_path"
        return 0
    fi
    return 1
}

install_python() {
    echo "Installing Python 3.13 using uv..."
    if ! uv python install 3.13; then
        echo "Error: Failed to install Python 3.13"
        return 1
    fi

    # Verify Python installation
    if ! detect_python >/dev/null; then
        echo "Error: Python 3.13 installation verification failed"
        return 1
    fi
    echo "Python 3.13 installed successfully"
    return 0
}

get_python_root_dir() {
    local python_path=$(uv python list --no-python-downloads | grep "3.13" | awk '{print $2}')
    if [ -n "$python_path" ]; then
        echo "${python_path%/bin/python*}"
    else
        echo "Error: Could not determine Python root directory"
        return 1
    fi
}

verify_python_installation() {
    local python_root="$1"
    local python_exec="$2"

    if [ ! -d "$python_root" ]; then
        echo "Error: Python root directory not found: $python_root"
        return 1
    fi

    if [ ! -x "$python_exec" ]; then
        echo "Error: Python executable not found or not executable: $python_exec"
        return 1
    fi

    return 0
}

# --- Install Homebrew (if not already installed) ---
if ! command -v brew &>/dev/null; then
    echo "Homebrew not found. Installing..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
    echo "Homebrew already installed."
fi

# Check if VTK is installed
if ! brew list vtk &>/dev/null; then
    echo "VTK not found. Installing..."
    wget https://raw.githubusercontent.com/Homebrew/homebrew-core/d16947c4bc97fdbec17ca158713f3609b701a4a4/Formula/vtk.rb
    brew install vtk.rb
    rm vtk.rb
else
    echo "VTK already installed."
fi

## REMOVE ITK BUILDING

rm -f src/plugins/imagefilter_itkn4iihc/CMakeLists.txt

# --- Install Required Packages ---
echo "Installing required packages using Homebrew..."
brew update
brew install cmake qt@5 lcms2 freeimage opencv eigen itk

brew install --skip-link uv

echo "Checking for Python 3.13..."
if ! detect_python; then
    echo "Python 3.13 not found. Installing..."
    if ! install_python; then
        echo "Python installation failed. Exiting."
        exit 1
    fi
fi

# Get Python installation directory
PYTHON_ROOT_DIR=$(get_python_root_dir) || {
    echo "Failed to determine Python root directory"
    exit 1
}
PYTHON_EXECUTABLE="$PYTHON_ROOT_DIR/bin/python3.13"

# Verify Python installation
if ! verify_python_installation "$PYTHON_ROOT_DIR" "$PYTHON_EXECUTABLE"; then
    echo "Python installation verification failed"
    exit 1
fi

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
    -DCMAKE_PREFIX_PATH="/usr/local/opt/qt@5;/usr/local/opt/lcms2;/usr/local/Cellar/freeimage/3.18.0;/usr/local/opt/itk"
    -DCMAKE_SHARED_LINKER_FLAGS="-L/usr/local/Cellar/freeimage/3.18.0/lib -L/usr/local/opt/lcms2/lib -llcms2"
    -DIBP_BUILD_PLUGINS=ON
    -DPython3_ROOT_DIR="$PYTHON_ROOT_DIR"
    -DPython3_EXECUTABLE="$PYTHON_EXECUTABLE"
    -DPython3_FIND_STRATEGY=LOCATION
    -DPython3_FIND_REGISTRY=NEVER
    -DPython3_FIND_FRAMEWORK=NEVER
    -DPython_FIND_FRAMEWORK=NEVER
    -DPython_FIND_REGISTRY=NEVER
)

# --- Build the Project ---
echo "Configuring project with CMake..."
echo "Configuring CMake with Python paths:"
echo "Python Root Dir: $PYTHON_ROOT_DIR"
echo "Python Executable: $PYTHON_EXECUTABLE"
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
