# this_file: test.ps1

# Comprehensive test runner for Image Batch Processor (Windows PowerShell)
# Usage: .\test.ps1 [options]

param(
    [switch]$Help,
    [string]$BuildDir = "build",
    [string]$Type = "Release",
    [switch]$Verbose,
    [switch]$Coverage,
    [switch]$Benchmark,
    [switch]$Integration,
    [switch]$UnitOnly,
    [switch]$PythonOnly,
    [switch]$CppOnly,
    [string]$Filter = "",
    [int]$Timeout = 300
)

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
Usage: .\test.ps1 [OPTIONS]

Test runner for Image Batch Processor (Windows)

OPTIONS:
    -Help               Show this help message
    -BuildDir D         Build directory (default: $BuildDir)
    -Type T             Build type (default: $Type)
    -Verbose            Verbose test output
    -Coverage           Enable code coverage reporting
    -Benchmark          Run performance benchmarks
    -Integration        Run integration tests
    -UnitOnly           Run only unit tests
    -PythonOnly         Run only Python tests
    -CppOnly            Run only C++ tests
    -Filter F           Filter tests by pattern
    -Timeout N          Test timeout in seconds (default: $Timeout)

EXAMPLES:
    .\test.ps1                      # Run all tests
    .\test.ps1 -UnitOnly            # Run only unit tests
    .\test.ps1 -Integration -Verbose # Run integration tests with verbose output
    .\test.ps1 -Coverage            # Run tests with coverage
    .\test.ps1 -Filter "*image*"    # Run tests matching pattern
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

Write-Log "Running tests on Windows" "INFO"
Write-Log "Build directory: $BuildDir" "INFO"
Write-Log "Build type: $Type" "INFO"

# Check if build directory exists
if (!(Test-Path $BuildDir)) {
    Write-Log "Build directory $BuildDir does not exist" "ERROR"
    Write-Log "Please run .\build.ps1 first" "ERROR"
    exit 1
}

# Test counters
$script:TestsPassed = 0
$script:TestsFailed = 0
$script:TestsSkipped = 0

# Function to run a test with timeout
function Invoke-Test {
    param(
        [string]$TestName,
        [string]$TestCommand,
        [string]$TestDir = $PWD
    )
    
    Write-Log "Running $TestName..." "INFO"
    
    if ($Verbose) {
        Write-Log "Command: $TestCommand" "INFO"
        Write-Log "Directory: $TestDir" "INFO"
    }
    
    $job = Start-Job -ScriptBlock {
        param($cmd, $dir)
        Set-Location $dir
        Invoke-Expression $cmd
    } -ArgumentList $TestCommand, $TestDir
    
    if (Wait-Job $job -Timeout $Timeout) {
        $result = Receive-Job $job
        $exitCode = $job.State -eq "Completed" -and $job.HasMoreData -eq $false
        
        if ($exitCode) {
            Write-Log "$TestName passed" "SUCCESS"
            $script:TestsPassed++
            Remove-Job $job
            return $true
        } else {
            Write-Log "$TestName failed" "ERROR"
            if ($result) {
                Write-Host $result
            }
            $script:TestsFailed++
            Remove-Job $job
            return $false
        }
    } else {
        Write-Log "$TestName timed out after $Timeout seconds" "ERROR"
        Stop-Job $job
        Remove-Job $job
        $script:TestsFailed++
        return $false
    }
}

# Function to run C++ tests
function Invoke-CppTests {
    if ($PythonOnly) {
        Write-Log "Skipping C++ tests (Python only mode)" "INFO"
        return
    }
    
    Write-Log "Running C++ tests..." "INFO"
    
    # Look for test executables
    $testExecutables = Get-ChildItem -Path $BuildDir -Recurse -Name "*test*.exe" -ErrorAction SilentlyContinue
    
    if ($testExecutables.Count -eq 0) {
        Write-Log "No C++ test executables found in $BuildDir" "WARNING"
        Write-Log "C++ tests may not be built yet" "WARNING"
        $script:TestsSkipped++
        return
    }
    
    # Run each test executable
    foreach ($testExe in $testExecutables) {
        $testName = [System.IO.Path]::GetFileNameWithoutExtension($testExe)
        
        if ($Filter -and $testName -notmatch $Filter) {
            Write-Log "Skipping $testName (filtered)" "INFO"
            $script:TestsSkipped++
            continue
        }
        
        $testArgs = if ($Verbose) { "--verbose" } else { "" }
        $testPath = Join-Path $BuildDir $testExe
        
        Invoke-Test "C++ $testName" "$testPath $testArgs"
    }
}

# Function to run Python tests
function Invoke-PythonTests {
    if ($CppOnly) {
        Write-Log "Skipping Python tests (C++ only mode)" "INFO"
        return
    }
    
    Write-Log "Running Python tests..." "INFO"
    
    # Check if pytest is available
    if (!(Get-Command "pytest" -ErrorAction SilentlyContinue)) {
        Write-Log "pytest not found. Please install it:" "ERROR"
        Write-Log "  pip install pytest" "ERROR"
        exit 1
    }
    
    # Activate virtual environment if it exists
    if (Test-Path ".venv\Scripts\Activate.ps1") {
        & .venv\Scripts\Activate.ps1
    }
    
    # Build pytest arguments
    $pytestArgs = @()
    
    if ($Verbose) {
        $pytestArgs += "-v"
    }
    
    if ($Coverage) {
        $pytestArgs += "--cov=src/imagebatchprocessor", "--cov-report=html", "--cov-report=term"
    }
    
    if ($Benchmark) {
        $pytestArgs += "--benchmark-only"
    }
    
    if ($Integration) {
        $pytestArgs += "-m", "integration"
    } elseif ($UnitOnly) {
        $pytestArgs += "-m", "not integration"
    }
    
    if ($Filter) {
        $pytestArgs += "-k", $Filter
    }
    
    # Add timeout
    $pytestArgs += "--timeout=$Timeout"
    
    # Run pytest
    $pytestCommand = "pytest $($pytestArgs -join ' ') tests/"
    Invoke-Test "Python tests" $pytestCommand
}

# Function to run integration tests
function Invoke-IntegrationTests {
    if ($UnitOnly) {
        Write-Log "Skipping integration tests (unit only mode)" "INFO"
        return
    }
    
    Write-Log "Running integration tests..." "INFO"
    
    # Test basic CLI functionality
    $cliTestCommands = @(
        "python -m imagebatchprocessor --version",
        "python -m imagebatchprocessor --help"
    )
    
    foreach ($cmd in $cliTestCommands) {
        if ($Filter -and $cmd -notmatch $Filter) {
            Write-Log "Skipping '$cmd' (filtered)" "INFO"
            $script:TestsSkipped++
            continue
        }
        
        Invoke-Test "CLI: $cmd" $cmd
    }
}

# Function to run benchmarks
function Invoke-Benchmarks {
    if (!$Benchmark) {
        Write-Log "Skipping benchmarks (not requested)" "INFO"
        return
    }
    
    Write-Log "Running performance benchmarks..." "INFO"
    
    # Run Python benchmarks
    if (!$CppOnly) {
        Invoke-Test "Python benchmarks" "pytest --benchmark-only tests/"
    }
    
    # TODO: Add C++ benchmarks when available
    Write-Log "C++ benchmarks not implemented yet" "INFO"
}

# Function to check code coverage
function Test-Coverage {
    if (!$Coverage) {
        return
    }
    
    Write-Log "Checking code coverage..." "INFO"
    
    # Coverage is generated by pytest with --cov
    # Display coverage report
    if (Test-Path "htmlcov\index.html") {
        Write-Log "Coverage report generated: htmlcov\index.html" "SUCCESS"
        
        # Extract coverage percentage if available
        $coverageContent = Get-Content "htmlcov\index.html" -Raw -ErrorAction SilentlyContinue
        if ($coverageContent -match 'pc_cov">(\d+)') {
            $coveragePct = $matches[1]
            Write-Log "Total coverage: $coveragePct%" "INFO"
        }
    }
}

# Function to generate test report
function Write-TestReport {
    Write-Log "Test Summary:" "INFO"
    Write-Log "  Passed: $script:TestsPassed" "INFO"
    Write-Log "  Failed: $script:TestsFailed" "INFO"
    Write-Log "  Skipped: $script:TestsSkipped" "INFO"
    Write-Log "  Total: $($script:TestsPassed + $script:TestsFailed + $script:TestsSkipped)" "INFO"
    
    if ($script:TestsFailed -gt 0) {
        Write-Log "Some tests failed!" "ERROR"
        return $false
    } else {
        Write-Log "All tests passed!" "SUCCESS"
        return $true
    }
}

# Main test execution
function Main {
    # Run tests based on options
    if (!$UnitOnly -and !$PythonOnly -and !$CppOnly) {
        # Run all tests
        Invoke-CppTests
        Invoke-PythonTests
        Invoke-IntegrationTests
        Invoke-Benchmarks
    } else {
        # Run specific test types
        Invoke-CppTests
        Invoke-PythonTests
        
        if ($Integration) {
            Invoke-IntegrationTests
        }
        
        Invoke-Benchmarks
    }
    
    # Check coverage
    Test-Coverage
    
    # Generate final report
    $success = Write-TestReport
    
    if (!$success) {
        exit 1
    }
}

# Run main function
Main