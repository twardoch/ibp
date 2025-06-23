import importlib.metadata
import subprocess
import sys
from unittest import mock

import pytest

# Try to import the main package
try:
    import imagebatchprocessor
except ImportError:
    imagebatchprocessor = None # Handle case where package isn't built/installed yet

def test_package_import():
    """
    Tests if the main package can be imported.
    This is a very basic check.
    """
    assert imagebatchprocessor is not None, "Package 'imagebatchprocessor' could not be imported."

def test_package_metadata():
    """
    Tests if package metadata can be read.
    This helps ensure the package is installed correctly.
    """
    try:
        version = importlib.metadata.version("imagebatchprocessor")
        assert version is not None
        # Further checks on version format could be added if hatch-vcs is set up
        # e.g. assert re.match(r"\d+\.\d+\.\d+", version)
    except importlib.metadata.PackageNotFoundError:
        pytest.fail("Package 'imagebatchprocessor' not found by importlib.metadata. Is it installed?")

@pytest.mark.skipif(imagebatchprocessor is None, reason="Package not importable, skipping entry point test")
def test_main_entry_point_exists():
    """
    Checks if the main function, intended as the entry point, exists.
    """
    assert hasattr(imagebatchprocessor, "main"), "The package 'imagebatchprocessor' does not have a 'main' function."

@pytest.mark.skipif(imagebatchprocessor is None, reason="Package not importable, skipping CLI test")
@mock.patch("subprocess.run")
@mock.patch("sys.exit")
@mock.patch("builtins.print")
def test_cli_entry_point_simulation_success(mock_print, mock_exit, mock_subprocess_run):
    """
    Simulates calling the main function as if it were an entry point.
    Mocks subprocess.run to avoid actually running the C++ application.
    """
    # Setup mock for executable path resolution within imagebatchprocessor.main
    # This needs to align with how imagebatchprocessor.main() finds the executable

    # If imagebatchprocessor.main() uses Path(__file__).parent:
    mock_executable_path = mock.MagicMock()
    mock_executable_path.exists.return_value = True

    # The path constructed in imagebatchprocessor.main() needs to be intercepted.
    # This is a bit tricky as it's constructed inside the function.
    # A more robust way would be for main() to have the path injected or use a discoverable location.
    # For now, let's assume imagebatchprocessor.main() can be called and we mock subprocess.run.

    # Simulate a successful run of the C++ application
    mock_subprocess_run.return_value = subprocess.CompletedProcess(args=["ibp"], returncode=0)

    try:
        imagebatchprocessor.main()
    except Exception as e:
        pytest.fail(f"imagebatchprocessor.main() raised an unexpected exception: {e}")

    mock_subprocess_run.assert_called_once() # Check that subprocess.run was called
    # We can add more specific assertions about the call if needed, e.g.,
    # mock_subprocess_run.assert_called_with([mock.ANY], check=True) # mock.ANY because path is dynamic

    mock_exit.assert_not_called() # Ensure sys.exit wasn't called on success path

@pytest.mark.skipif(imagebatchprocessor is None, reason="Package not importable, skipping CLI test")
@mock.patch("subprocess.run")
@mock.patch("sys.exit")
@mock.patch("builtins.print")
def test_cli_entry_point_simulation_app_not_found(mock_print, mock_exit, mock_subprocess_run):
    """
    Simulates the scenario where the C++ application is not found by the Python entry point.
    """
    # This test requires imagebatchprocessor.main() to correctly check for the executable's existence.
    # We need to mock the part of imagebatchprocessor.main that checks `executable_path.exists()`

    # This is difficult to do perfectly without changing imagebatchprocessor.main() to be more testable.
    # One way is to mock Path.exists globally, but that's broad.
    # For now, we'll rely on the structure of imagebatchprocessor.main and ensure it calls sys.exit(1)
    # when the (mocked) executable_path.exists() returns False.

    # To effectively test this, imagebatchprocessor.main() should ideally take the path as an argument
    # or use a helper function for path resolution that can be mocked.

    # Let's assume we can mock Path.exists for the specific path it checks.
    # This requires knowing the exact path string.
    # path_to_mock = Path(imagebatchprocessor.__file__).parent / "ImageBatchProcessor" (or .exe)

    # A simpler, though less precise, approach for this test:
    # We can't easily mock `executable_path.exists()` from outside `main()`.
    # Instead, we'll assume that if `subprocess.run` is *not* called, and `sys.exit(1)` *is* called,
    # it implies the file-not-found path was taken. This is an indirect test.

    # To make this test more direct, we would refactor `imagebatchprocessor.main`.
    # For now, let's assume the 'executable not found' message is printed and sys.exit(1) is called.

    # This mock setup is for a different scenario, let's adjust
    # mock_subprocess_run.side_effect = FileNotFoundError("Mocked: Executable not found by subprocess")

    # To test the "executable not found" logic within imagebatchprocessor.main(), we need to
    # make `executable_path.exists()` return False.
    # The current `imagebatchprocessor.main` uses `Path(__file__).parent / executable_name`.
    # We can try to mock `Path.exists` for this specific instance.

    with mock.patch("pathlib.Path.exists", return_value=False) as mock_path_exists:
        # We need to ensure this mock targets the Path object created inside main() for the executable.
        # This is tricky because the Path object is instantiated within the function.
        # A common pattern is to mock the class `Path` itself if it's imported like `from pathlib import Path`.
        # If it's `import pathlib`, then `pathlib.Path.exists`.

        # Let's assume `from pathlib import Path` is used in `imagebatchprocessor/__init__.py`.
        # And let the name of the executable be determined.
        executable_name = "ibp" # Changed from ImageBatchProcessor
        if sys.platform == "win32":
            executable_name += ".exe"

        # The path that would be checked:
        # expected_checked_path = Path(imagebatchprocessor.__file__).parent / executable_name
        # mock_path_exists.return_value = False # Already set

        imagebatchprocessor.main()

        # Check that it tried to exit with 1
        mock_exit.assert_called_with(1)

        # Check that print was called with an error message (adapt message as needed)
        # This is fragile as it depends on the exact print message.
        args, kwargs = mock_print.call_args_list[-1] # Get the last call to print
        assert "Error: Packaged executable" in args[0]
        assert f"'{executable_name}' not found" in args[0]

        # Ensure subprocess.run was not called because the file was "not found"
        mock_subprocess_run.assert_not_called()


# It would also be good to have a test that uses `python -m imagebatchprocessor`
# or the console script `imagebatchprocessor` via subprocess,
# but that requires the package to be built and installed,
# which is more of an integration test for later.
# For now, these unit tests for the Python part are a good start.

def find_executable_script():
    """Tries to find the installed 'imagebatchprocessor' script."""
    # This could be more robust, e.g. checking sys.prefix or user bin paths
    # if shutil.which is not sufficient on all platforms/CI environments.
    import shutil
    return shutil.which("imagebatchprocessor")

@pytest.mark.integration
def test_cli_application_version():
    """
    Tests running the packaged application's --version command.
    This is an integration test that actually runs the compiled C++ application.
    """
    executable_script = find_executable_script()

    # Fallback to `python -m imagebatchprocessor` if script isn't found easily by `shutil.which`
    # This is often more reliable, especially in CI or complex environments.
    command_to_run = []
    if executable_script:
        command_to_run = [executable_script, "--version"]
        print(f"Found executable script: {executable_script}")
    else:
        # This assumes that `python -m imagebatchprocessor --version` will pass args to the C++ app
        # which our `imagebatchprocessor.main()` does.
        command_to_run = [sys.executable, "-m", "imagebatchprocessor", "--version"]
        print(f"Executable script not found by shutil.which, trying: {' '.join(command_to_run)}")


    try:
        result = subprocess.run(command_to_run, capture_output=True, text=True, check=False, timeout=30)
    except FileNotFoundError:
        pytest.fail(f"Failed to run command: {' '.join(command_to_run)}. Executable not found or not in PATH.")
    except subprocess.TimeoutExpired:
        pytest.fail(f"Command {' '.join(command_to_run)} timed out.")

    print(f"--- Running command: {' '.join(command_to_run)} ---")
    print(f"Return code: {result.returncode}")
    print(f"Stdout:\n{result.stdout}")
    print(f"Stderr:\n{result.stderr}")
    print("--- End of command output ---")

    assert result.returncode == 0, \
        f"Command {' '.join(command_to_run)} failed with exit code {result.returncode}.\nStderr: {result.stderr}"

    # QCommandLineParser usually prints version to stdout for --version.
    # The version in main.cpp is hardcoded as "1.0.0" via a.setApplicationVersion("1.0.0");
    # The name is "Image Batch Processor".
    # Output of --version for QCommandLineParser is typically "<AppName> <version>"
    assert "Image Batch Processor 1.0.0" in result.stdout or "ibp 1.0.0" in result.stdout, \
        f"Expected version string not found in stdout. Got: {result.stdout}"

@pytest.mark.integration
def test_cli_application_help():
    """
    Tests running the packaged application's --help command.
    """
    executable_script = find_executable_script()
    command_to_run = []
    if executable_script:
        command_to_run = [executable_script, "--help"]
    else:
        command_to_run = [sys.executable, "-m", "imagebatchprocessor", "--help"]

    try:
        result = subprocess.run(command_to_run, capture_output=True, text=True, check=False, timeout=30)
    except FileNotFoundError:
        pytest.fail(f"Failed to run command: {' '.join(command_to_run)}. Executable not found or not in PATH.")
    except subprocess.TimeoutExpired:
        pytest.fail(f"Command {' '.join(command_to_run)} timed out.")

    print(f"--- Running command: {' '.join(command_to_run)} ---")
    print(f"Return code: {result.returncode}")
    print(f"Stdout:\n{result.stdout}")
    print(f"Stderr:\n{result.stderr}")
    print("--- End of command output ---")

    assert result.returncode == 0, \
        f"Command {' '.join(command_to_run)} failed with exit code {result.returncode}.\nStderr: {result.stderr}"
    # Check for some common help text elements.
    # QCommandLineParser output for --help includes "Usage:", "Options:", "Arguments:"
    assert "Usage:" in result.stdout, "Help output should contain 'Usage:'"
    assert "Options:" in result.stdout, "Help output should contain 'Options:'"
    # The application description is "Image Batch Processor"
    assert "Image Batch Processor" in result.stdout, "Help output should contain application description"

# To run integration tests: pytest -m integration
# To run all tests: pytest
# Ensure the package is built and installed (`uv pip install .`) in the test environment.
