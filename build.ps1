# this_file: build.ps1

# Cross-platform build script for Image Batch Processor (Windows PowerShell)
# Usage: .\build.ps1 [options]

param(
    [switch]$Help,
    [switch]$Clean,
    [switch]$Debug,
    [int]$Jobs = 4,
    [switch]$Verbose,
    [switch]$SkipTests,
    [switch]$SkipPython,
    [string]$Generator = "",
    [string]$BuildDir = "build",
    [string]$InstallPrefix = ""
)

# Default configuration
$BuildType = if ($Debug) { "Debug" } else { "Release" }

# Colors for output
$Red = [ConsoleColor]::Red
$Green = [ConsoleColor]::Green
$Yellow = [ConsoleColor]::Yellow
$Blue = [ConsoleColor]::Blue

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $color = switch ($Level) {
        "ERROR" { $Red }
        "SUCCESS" { $Green }
        "WARNING" { $Yellow }
        "INFO" { $Blue }
        default { $Blue }
    }
    Write-Host "[$Level] $Message" -ForegroundColor $color
}

function Show-Usage {
    Write-Host @"
Usage: .\build.ps1 [OPTIONS]

Build script for Image Batch Processor (Windows)

OPTIONS:
    -Help               Show this help message
    -Clean              Clean build directory before building
    -Debug              Build in Debug mode (default: Release)
    -Jobs N             Number of parallel jobs (default: 4)
    -Verbose            Verbose build output
    -SkipTests          Skip running tests after build
    -SkipPython         Skip Python package build
    -Generator G        CMake generator (e.g., "Visual Studio 17 2022")
    -BuildDir D         Build directory (default: build)
    -InstallPrefix P    Install prefix directory

EXAMPLES:
    .\build.ps1                     # Build with default settings
    .\build.ps1 -Clean -Debug       # Clean debug build
    .\build.ps1 -Jobs 8 -Verbose    # Build with 8 jobs, verbose output
    .\build.ps1 -Generator "Ninja"  # Build with Ninja generator
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

Write-Log "Building Image Batch Processor on Windows" "INFO"
Write-Log "Build type: $BuildType" "INFO"
Write-Log "Parallel jobs: $Jobs" "INFO"
Write-Log "Build directory: $BuildDir" "INFO"

# Check for required tools
function Test-Tool {
    param([string]$Tool)
    if (!(Get-Command $Tool -ErrorAction SilentlyContinue)) {
        Write-Log "$Tool is required but not installed" "ERROR"
        exit 1
    }
}

Write-Log "Checking required tools..." "INFO"
Test-Tool "cmake"
Test-Tool "git"

# Check for Python and uv if not skipping Python build
if (!$SkipPython) {
    Test-Tool "python"
    if (!(Get-Command "uv" -ErrorAction SilentlyContinue)) {
        Write-Log "uv not found, please install it manually" "WARNING"
        Write-Log "Download from: https://github.com/astral-sh/uv/releases" "WARNING"
    }
}

# Check for Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($vsPath) {
        Write-Log "Found Visual Studio at: $vsPath" "INFO"
    } else {
        Write-Log "Visual Studio with C++ tools not found" "ERROR"
        exit 1
    }
} else {
    Write-Log "Visual Studio installer not found" "ERROR"
    exit 1
}

# Clean build directory if requested
if ($Clean) {
    Write-Log "Cleaning build directory..." "INFO"
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }
}

# Create build directory
if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Set-Location $BuildDir

# Configure CMake
Write-Log "Configuring CMake..." "INFO"
$cmakeArgs = @(
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DIBP_BUILD_PLUGINS=ON"
)

if ($Generator) {
    $cmakeArgs += "-G", $Generator
}

if ($InstallPrefix) {
    $cmakeArgs += "-DCMAKE_INSTALL_PREFIX=$InstallPrefix"
}

if ($Verbose) {
    $cmakeArgs += "-DCMAKE_VERBOSE_MAKEFILE=ON"
}

& cmake $cmakeArgs ..
if ($LASTEXITCODE -ne 0) {
    Write-Log "CMake configuration failed" "ERROR"
    exit 1
}

# Build
Write-Log "Building project..." "INFO"
$buildArgs = @(
    "--build", ".",
    "--config", $BuildType,
    "--parallel", $Jobs
)

if ($Verbose) {
    $buildArgs += "--verbose"
}

& cmake $buildArgs
if ($LASTEXITCODE -ne 0) {
    Write-Log "Build failed" "ERROR"
    exit 1
}

# Install if prefix specified
if ($InstallPrefix) {
    Write-Log "Installing to $InstallPrefix..." "INFO"
    & cmake --install . --config $BuildType
    if ($LASTEXITCODE -ne 0) {
        Write-Log "Installation failed" "ERROR"
        exit 1
    }
}

Set-Location ..

# Run tests if not skipped
if (!$SkipTests) {
    Write-Log "Running tests..." "INFO"
    & .\test.ps1 -BuildDir $BuildDir -Type $BuildType
    if ($LASTEXITCODE -ne 0) {
        Write-Log "Tests failed" "ERROR"
        exit 1
    }
}

# Build Python package if not skipped
if (!$SkipPython) {
    Write-Log "Building Python package..." "INFO"
    
    # Activate virtual environment if it exists
    if (Test-Path ".venv\Scripts\Activate.ps1") {
        & .venv\Scripts\Activate.ps1
    }
    
    # Install/upgrade build dependencies
    & uv pip install --upgrade build hatch hatch-vcs
    if ($LASTEXITCODE -ne 0) {
        Write-Log "Failed to install build dependencies" "ERROR"
        exit 1
    }
    
    # Build Python package
    & python -m build
    if ($LASTEXITCODE -ne 0) {
        Write-Log "Python package build failed" "ERROR"
        exit 1
    }
    
    Write-Log "Python package built successfully" "SUCCESS"
    Get-ChildItem -Path "dist" | Format-Table Name, Length, LastWriteTime
}

Write-Log "Build completed successfully!" "SUCCESS"
Write-Log "Build artifacts:" "INFO"
Write-Log "  C++ binaries: $BuildDir\" "INFO"
if (!$SkipPython) {
    Write-Log "  Python packages: dist\" "INFO"
}