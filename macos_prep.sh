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
