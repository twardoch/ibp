#!/bin/bash
# this_file: build.sh

# Cross-platform build script for Image Batch Processor
# Usage: ./build.sh [options]

set -e

# Default configuration
BUILD_TYPE="Release"
PARALLEL_JOBS=$(nproc 2>/dev/null || echo 4)
CLEAN_BUILD=false
VERBOSE=false
SKIP_TESTS=false
SKIP_PYTHON=false
CMAKE_GENERATOR=""
BUILD_DIR="build"
INSTALL_PREFIX=""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Print usage
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Build script for Image Batch Processor

OPTIONS:
    -h, --help          Show this help message
    -c, --clean         Clean build directory before building
    -d, --debug         Build in Debug mode (default: Release)
    -j, --jobs N        Number of parallel jobs (default: $PARALLEL_JOBS)
    -v, --verbose       Verbose build output
    -t, --skip-tests    Skip running tests after build
    -p, --skip-python   Skip Python package build
    -g, --generator G   CMake generator (e.g., Ninja, Unix Makefiles)
    -b, --build-dir D   Build directory (default: $BUILD_DIR)
    -i, --install P     Install prefix directory

EXAMPLES:
    $0                  # Build with default settings
    $0 -c -d            # Clean debug build
    $0 -j 8 -v          # Build with 8 jobs, verbose output
    $0 -g Ninja         # Build with Ninja generator
EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -t|--skip-tests)
            SKIP_TESTS=true
            shift
            ;;
        -p|--skip-python)
            SKIP_PYTHON=true
            shift
            ;;
        -g|--generator)
            CMAKE_GENERATOR="$2"
            shift 2
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -i|--install)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        *)
            log_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Detect OS
OS=$(uname -s)
case $OS in
    Linux*)
        PLATFORM="Linux"
        ;;
    Darwin*)
        PLATFORM="macOS"
        ;;
    CYGWIN*|MINGW*|MSYS*)
        PLATFORM="Windows"
        ;;
    *)
        log_error "Unsupported operating system: $OS"
        exit 1
        ;;
esac

log_info "Building Image Batch Processor on $PLATFORM"
log_info "Build type: $BUILD_TYPE"
log_info "Parallel jobs: $PARALLEL_JOBS"
log_info "Build directory: $BUILD_DIR"

# Check for required tools
check_tool() {
    if ! command -v "$1" >/dev/null 2>&1; then
        log_error "$1 is required but not installed"
        exit 1
    fi
}

log_info "Checking required tools..."
check_tool cmake
check_tool git

# Check for Python and uv if not skipping Python build
if [ "$SKIP_PYTHON" = false ]; then
    check_tool python3
    if ! command -v uv >/dev/null 2>&1; then
        log_warning "uv not found, installing..."
        curl -LsSf https://astral.sh/uv/install.sh | sh
        export PATH="$HOME/.cargo/bin:$PATH"
    fi
fi

# Platform-specific dependency checks
case $PLATFORM in
    Linux)
        # Check for required libraries on Linux
        if ! pkg-config --exists Qt5Core Qt5Widgets; then
            log_error "Qt5 development packages not found. Install with:"
            log_error "  Ubuntu/Debian: sudo apt-get install qtbase5-dev libqt5widgets5"
            log_error "  CentOS/RHEL: sudo yum install qt5-qtbase-devel"
            exit 1
        fi
        ;;
    macOS)
        # Check for Homebrew and Qt on macOS
        if ! brew --version >/dev/null 2>&1; then
            log_error "Homebrew is required on macOS"
            exit 1
        fi
        if ! brew list qt@5 >/dev/null 2>&1; then
            log_warning "Qt5 not found, installing..."
            brew install qt@5
        fi
        # Set Qt5 path
        export CMAKE_PREFIX_PATH="/usr/local/opt/qt@5:$CMAKE_PREFIX_PATH"
        ;;
    Windows)
        log_info "Windows build detected - ensure Visual Studio and Qt5 are installed"
        ;;
esac

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    log_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
log_info "Configuring CMake..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DIBP_BUILD_PLUGINS=ON
)

if [ -n "$CMAKE_GENERATOR" ]; then
    CMAKE_ARGS+=(-G "$CMAKE_GENERATOR")
fi

if [ -n "$INSTALL_PREFIX" ]; then
    CMAKE_ARGS+=(-DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX")
fi

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

# Add platform-specific CMake options
case $PLATFORM in
    macOS)
        CMAKE_ARGS+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=10.12)
        ;;
esac

cmake "${CMAKE_ARGS[@]}" ..

# Build
log_info "Building project..."
BUILD_ARGS=(--build . --config "$BUILD_TYPE")

if [ "$VERBOSE" = true ]; then
    BUILD_ARGS+=(--verbose)
fi

BUILD_ARGS+=(--parallel "$PARALLEL_JOBS")

cmake "${BUILD_ARGS[@]}"

# Install if prefix specified
if [ -n "$INSTALL_PREFIX" ]; then
    log_info "Installing to $INSTALL_PREFIX..."
    cmake --install . --config "$BUILD_TYPE"
fi

cd ..

# Run tests if not skipped
if [ "$SKIP_TESTS" = false ]; then
    log_info "Running tests..."
    ./test.sh --build-dir "$BUILD_DIR" --type "$BUILD_TYPE"
fi

# Build Python package if not skipped
if [ "$SKIP_PYTHON" = false ]; then
    log_info "Building Python package..."
    
    # Activate virtual environment if it exists
    if [ -d ".venv" ]; then
        source .venv/bin/activate
    fi
    
    # Install/upgrade build dependencies
    uv pip install --upgrade build hatch hatch-vcs
    
    # Build Python package
    python -m build
    
    log_success "Python package built successfully"
    ls -la dist/
fi

log_success "Build completed successfully!"
log_info "Build artifacts:"
log_info "  C++ binaries: $BUILD_DIR/"
if [ "$SKIP_PYTHON" = false ]; then
    log_info "  Python packages: dist/"
fi