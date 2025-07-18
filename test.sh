#!/bin/bash
# this_file: test.sh

# Comprehensive test runner for Image Batch Processor
# Usage: ./test.sh [options]

set -e

# Default configuration
BUILD_DIR="build"
BUILD_TYPE="Release"
VERBOSE=false
COVERAGE=false
BENCHMARK=false
INTEGRATION=false
UNIT_ONLY=false
PYTHON_ONLY=false
CPP_ONLY=false
TEST_FILTER=""
TEST_TIMEOUT=300

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

Test runner for Image Batch Processor

OPTIONS:
    -h, --help          Show this help message
    -b, --build-dir D   Build directory (default: $BUILD_DIR)
    -t, --type T        Build type (default: $BUILD_TYPE)
    -v, --verbose       Verbose test output
    -c, --coverage      Enable code coverage reporting
    -p, --benchmark     Run performance benchmarks
    -i, --integration   Run integration tests
    -u, --unit-only     Run only unit tests
    -y, --python-only   Run only Python tests
    -x, --cpp-only      Run only C++ tests
    -f, --filter F      Filter tests by pattern
    -T, --timeout N     Test timeout in seconds (default: $TEST_TIMEOUT)

EXAMPLES:
    $0                  # Run all tests
    $0 -u               # Run only unit tests
    $0 -i -v            # Run integration tests with verbose output
    $0 -c               # Run tests with coverage
    $0 -f "*image*"     # Run tests matching pattern
EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -c|--coverage)
            COVERAGE=true
            shift
            ;;
        -p|--benchmark)
            BENCHMARK=true
            shift
            ;;
        -i|--integration)
            INTEGRATION=true
            shift
            ;;
        -u|--unit-only)
            UNIT_ONLY=true
            shift
            ;;
        -y|--python-only)
            PYTHON_ONLY=true
            shift
            ;;
        -x|--cpp-only)
            CPP_ONLY=true
            shift
            ;;
        -f|--filter)
            TEST_FILTER="$2"
            shift 2
            ;;
        -T|--timeout)
            TEST_TIMEOUT="$2"
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

log_info "Running tests on $PLATFORM"
log_info "Build directory: $BUILD_DIR"
log_info "Build type: $BUILD_TYPE"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    log_error "Build directory $BUILD_DIR does not exist"
    log_error "Please run ./build.sh first"
    exit 1
fi

# Test counters
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

# Function to run a test with timeout
run_test() {
    local test_name="$1"
    local test_command="$2"
    local test_dir="${3:-$PWD}"
    
    log_info "Running $test_name..."
    
    if [ "$VERBOSE" = true ]; then
        log_info "Command: $test_command"
        log_info "Directory: $test_dir"
    fi
    
    (
        cd "$test_dir"
        timeout "$TEST_TIMEOUT" bash -c "$test_command"
    )
    
    local exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        log_success "$test_name passed"
        ((TESTS_PASSED++))
        return 0
    elif [ $exit_code -eq 124 ]; then
        log_error "$test_name timed out after $TEST_TIMEOUT seconds"
        ((TESTS_FAILED++))
        return 1
    else
        log_error "$test_name failed with exit code $exit_code"
        ((TESTS_FAILED++))
        return 1
    fi
}

# Function to run C++ tests
run_cpp_tests() {
    if [ "$PYTHON_ONLY" = true ]; then
        log_info "Skipping C++ tests (Python only mode)"
        return 0
    fi
    
    log_info "Running C++ tests..."
    
    # Look for test executables
    local test_executables=()
    while IFS= read -r -d '' test_exe; do
        test_executables+=("$test_exe")
    done < <(find "$BUILD_DIR" -name "*test*" -type f -executable -print0 2>/dev/null)
    
    if [ ${#test_executables[@]} -eq 0 ]; then
        log_warning "No C++ test executables found in $BUILD_DIR"
        log_warning "C++ tests may not be built yet"
        ((TESTS_SKIPPED++))
        return 0
    fi
    
    # Run each test executable
    for test_exe in "${test_executables[@]}"; do
        local test_name=$(basename "$test_exe")
        
        if [ -n "$TEST_FILTER" ] && [[ ! "$test_name" =~ $TEST_FILTER ]]; then
            log_info "Skipping $test_name (filtered)"
            ((TESTS_SKIPPED++))
            continue
        fi
        
        local test_args=""
        if [ "$VERBOSE" = true ]; then
            test_args="--verbose"
        fi
        
        run_test "C++ $test_name" "$test_exe $test_args"
    done
}

# Function to run Python tests
run_python_tests() {
    if [ "$CPP_ONLY" = true ]; then
        log_info "Skipping Python tests (C++ only mode)"
        return 0
    fi
    
    log_info "Running Python tests..."
    
    # Check if pytest is available
    if ! command -v pytest >/dev/null 2>&1; then
        log_error "pytest not found. Please install it:"
        log_error "  pip install pytest"
        exit 1
    fi
    
    # Activate virtual environment if it exists
    if [ -d ".venv" ]; then
        source .venv/bin/activate
    fi
    
    # Build pytest arguments
    local pytest_args=()
    
    if [ "$VERBOSE" = true ]; then
        pytest_args+=("-v")
    fi
    
    if [ "$COVERAGE" = true ]; then
        pytest_args+=("--cov=src/imagebatchprocessor" "--cov-report=html" "--cov-report=term")
    fi
    
    if [ "$BENCHMARK" = true ]; then
        pytest_args+=("--benchmark-only")
    fi
    
    if [ "$INTEGRATION" = true ]; then
        pytest_args+=("-m" "integration")
    elif [ "$UNIT_ONLY" = true ]; then
        pytest_args+=("-m" "not integration")
    fi
    
    if [ -n "$TEST_FILTER" ]; then
        pytest_args+=("-k" "$TEST_FILTER")
    fi
    
    # Add timeout
    pytest_args+=("--timeout=$TEST_TIMEOUT")
    
    # Run pytest
    run_test "Python tests" "pytest ${pytest_args[*]} tests/"
}

# Function to run integration tests
run_integration_tests() {
    if [ "$UNIT_ONLY" = true ]; then
        log_info "Skipping integration tests (unit only mode)"
        return 0
    fi
    
    log_info "Running integration tests..."
    
    # Test basic CLI functionality
    local cli_test_commands=(
        "python -m imagebatchprocessor --version"
        "python -m imagebatchprocessor --help"
    )
    
    for cmd in "${cli_test_commands[@]}"; do
        if [ -n "$TEST_FILTER" ] && [[ ! "$cmd" =~ $TEST_FILTER ]]; then
            log_info "Skipping '$cmd' (filtered)"
            ((TESTS_SKIPPED++))
            continue
        fi
        
        run_test "CLI: $cmd" "$cmd"
    done
}

# Function to run benchmarks
run_benchmarks() {
    if [ "$BENCHMARK" = false ]; then
        log_info "Skipping benchmarks (not requested)"
        return 0
    fi
    
    log_info "Running performance benchmarks..."
    
    # Run Python benchmarks
    if [ "$CPP_ONLY" = false ]; then
        run_test "Python benchmarks" "pytest --benchmark-only tests/"
    fi
    
    # TODO: Add C++ benchmarks when available
    log_info "C++ benchmarks not implemented yet"
}

# Function to check code coverage
check_coverage() {
    if [ "$COVERAGE" = false ]; then
        return 0
    fi
    
    log_info "Checking code coverage..."
    
    # Coverage is generated by pytest with --cov
    # Display coverage report
    if [ -f "htmlcov/index.html" ]; then
        log_success "Coverage report generated: htmlcov/index.html"
        
        # Extract coverage percentage if available
        if command -v grep >/dev/null 2>&1; then
            local coverage_pct=$(grep -oP 'pc_cov">\K[0-9]+' htmlcov/index.html | head -1)
            if [ -n "$coverage_pct" ]; then
                log_info "Total coverage: $coverage_pct%"
            fi
        fi
    fi
}

# Function to generate test report
generate_report() {
    log_info "Test Summary:"
    log_info "  Passed: $TESTS_PASSED"
    log_info "  Failed: $TESTS_FAILED"
    log_info "  Skipped: $TESTS_SKIPPED"
    log_info "  Total: $((TESTS_PASSED + TESTS_FAILED + TESTS_SKIPPED))"
    
    if [ $TESTS_FAILED -gt 0 ]; then
        log_error "Some tests failed!"
        return 1
    else
        log_success "All tests passed!"
        return 0
    fi
}

# Main test execution
main() {
    # Run tests based on options
    if [ "$UNIT_ONLY" = false ] && [ "$PYTHON_ONLY" = false ] && [ "$CPP_ONLY" = false ]; then
        # Run all tests
        run_cpp_tests
        run_python_tests
        run_integration_tests
        run_benchmarks
    else
        # Run specific test types
        run_cpp_tests
        run_python_tests
        
        if [ "$INTEGRATION" = true ]; then
            run_integration_tests
        fi
        
        run_benchmarks
    fi
    
    # Check coverage
    check_coverage
    
    # Generate final report
    generate_report
}

# Run main function
main "$@"