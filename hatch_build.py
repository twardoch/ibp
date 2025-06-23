import os
import shutil
import subprocess
import sys
from pathlib import Path

from hatchling.builders.hooks.plugin.interface import BuildHookInterface


class CustomBuildHook(BuildHookInterface):
    def _copy_artifacts(self, build_dir: Path, package_dir: Path):
        self.app.display_info("Copying build artifacts...")

        # Ensure target package_dir and its subdirectories exist
        package_dir.mkdir(parents=True, exist_ok=True)
        (package_dir / "plugins").mkdir(parents=True, exist_ok=True)

        # --- Main Application ---
        app_executable_name = "ibp"  # Corrected name
        if sys.platform == "win32":
            app_executable_name += ".exe"

        # Path based on IBP_OUTPUT_DIRECTORY which is ${CMAKE_BINARY_DIR}/build
        built_app_path = build_dir / "build" / app_executable_name

        if built_app_path.exists():
            target_app_path = package_dir / app_executable_name
            self.app.display_info(f"Copying application {built_app_path} to {target_app_path}")
            shutil.copy2(built_app_path, target_app_path)
            if sys.platform != "win32":
                os.chmod(target_app_path, 0o755)
        else:
            raise FileNotFoundError(f"Application executable not found at {built_app_path}.")

        # --- Core Shared Libraries ---
        # These are the libraries like ibp.misc, ibp.imgproc, etc.
        # They are also in build_dir / "build" /
        core_lib_names = ["ibp.misc", "ibp.plugins", "ibp.imgproc", "ibp.widgets"]
        if sys.platform == "win32":
            lib_prefix = ""
            lib_suffix = ".dll"
        elif sys.platform == "darwin":
            lib_prefix = "lib"
            lib_suffix = ".dylib"
        else: # Linux and other Unix-like
            lib_prefix = "lib"
            lib_suffix = ".so"

        for lib_base_name in core_lib_names:
            full_lib_name = f"{lib_prefix}{lib_base_name}{lib_suffix}"
            built_lib_path = build_dir / "build" / full_lib_name
            if built_lib_path.exists():
                target_lib_path = package_dir / full_lib_name
                self.app.display_info(f"Copying core library {built_lib_path} to {target_lib_path}")
                shutil.copy2(built_lib_path, target_lib_path)
            else:
                self.app.display_warning(f"Core library {full_lib_name} not found at {built_lib_path}")


        # --- Actual Plugin Files (*.so, *.dll from src/plugins/*) ---
        # These are distinct from the ibp.plugins loader library.
        # Plugins are in ${CMAKE_BINARY_DIR}/build/plugins
        plugins_source_dir = build_dir / "build" / "plugins"
        plugins_target_dir = package_dir / "plugins"

        if plugins_source_dir.exists() and plugins_source_dir.is_dir():
            if plugins_target_dir.exists(): # Clean target first if it exists
                shutil.rmtree(plugins_target_dir, ignore_errors=True)
            # Recreate after cleaning or if it didn't exist
            try:
                plugins_target_dir.mkdir(parents=True, exist_ok=False)
            except FileExistsError:
                # Directory was created by another process in the meantime
                pass

            self.app.display_info(f"Copying plugins from {plugins_source_dir} to {plugins_target_dir}")
            for item in plugins_source_dir.iterdir():
                if item.is_file(): # Add more specific checks if needed (e.g. by extension)
                    shutil.copy2(item, plugins_target_dir / item.name)
                elif item.is_dir(): # If plugins can be in subdirectories
                    shutil.copytree(item, plugins_target_dir / item.name, dirs_exist_ok=True)
        else:
            self.app.display_warning(f"Plugins source directory not found or is not a directory: {plugins_source_dir}")


    def initialize(self, version, build_data):
        self.app.display_info("Starting custom CMake build for ImageBatchProcessor (ibp)...")

        root_dir = Path(self.root)
        build_dir = root_dir / "build_hatch"
        # Define package_dir where artifacts will be copied, relative to project root
        package_dir = root_dir / "src" / "imagebatchprocessor"

        if build_dir.exists():
            self.app.display_info(f"Cleaning previous build directory: {build_dir}")
            shutil.rmtree(build_dir)
        build_dir.mkdir(parents=True, exist_ok=True)
        # Ensure package_dir exists for artifact copying
        package_dir.mkdir(parents=True, exist_ok=True)


        cmake_configure_command = [
            "cmake",
            "-S", str(root_dir),
            "-B", str(build_dir),
            "-DCMAKE_BUILD_TYPE=Release",
            "-DIBP_BUILD_PLUGINS=ON", # Ensure plugins are built
        ]
        if sys.platform == "win32":
            # Example: cmake_configure_command.extend(["-G", "Visual Studio 17 2022", "-A", "x64"])
            pass
        # No specific macOS options added for now unless identified as necessary

        self.app.display_info(f"Configuring CMake with: {' '.join(cmake_configure_command)}")
        subprocess.run(cmake_configure_command, cwd=root_dir, check=True)

        cmake_build_command = [
            "cmake",
            "--build", str(build_dir),
            "--config", "Release",
        ]
        num_jobs = os.cpu_count()
        if num_jobs:
            cmake_build_command.extend(["--parallel", str(num_jobs)])

        self.app.display_info(f"Building with CMake: {' '.join(cmake_build_command)}")
        subprocess.run(cmake_build_command, cwd=root_dir, check=True)

        # Copy artifacts after successful build
        self._copy_artifacts(build_dir, package_dir)

        # Hatch will automatically include files from the package_dir (src/imagebatchprocessor)
        # because it's the package source. No need for build_data['force_include']
        # if artifacts are correctly placed within this directory structure.

        self.app.display_info("Custom CMake build step finished.")


    def finalize(self, version, build_data, artifact_path):
        self.app.display_info(f"ImageBatchProcessor (ibp) built. Artifact at: {artifact_path}")

# Main execution block for testing the script directly (not used by Hatch)
if __name__ == "__main__":
    print("Running hatch_build.py directly for testing artifact copying...")

    class MockApp:
        def __init__(self): self.verbosity = 1
        def display_info(self, msg): print(f"INFO: {msg}")
        def display_warning(self, msg): print(f"WARN: {msg}")
        def display_error(self, msg): print(f"ERROR: {msg}")

    # Test is run from project root (where hatch_build.py is)
    test_root_dir = Path.cwd()

    # Simulate paths as they would be in the hook
    # build_dir for the hook means self.root / "build_hatch"
    # package_dir for the hook means self.root / "src" / "imagebatchprocessor"

    # For testing _copy_artifacts, we create temporary mock build and package directories
    mock_build_dir_for_test = test_root_dir / "build_hatch_test_artifacts"
    mock_package_dir_for_test = test_root_dir / "src_imagebatchprocessor_test_artifacts"

    hook = CustomBuildHook(root=str(test_root_dir), config={}, build_config={}, context=None, app=MockApp())

    try:
        # Clean up before test
        if mock_build_dir_for_test.exists(): shutil.rmtree(mock_build_dir_for_test)
        if mock_package_dir_for_test.exists(): shutil.rmtree(mock_package_dir_for_test)

        # Create structure expected by _copy_artifacts under mock_build_dir_for_test
        # This is what CMake build would produce in build_dir/build/
        cmake_output_dir = mock_build_dir_for_test / "build"
        cmake_output_plugins_dir = cmake_output_dir / "plugins"
        cmake_output_plugins_dir.mkdir(parents=True, exist_ok=True)

        # Create dummy executable in the "CMake output" location
        exe_name = "ibp.exe" if sys.platform == "win32" else "ibp"
        with open(cmake_output_dir / exe_name, "w") as f: f.write("dummy exe content")

        # Create dummy core libraries
        core_lib_names_test = ["ibp.misc", "ibp.plugins", "ibp.imgproc", "ibp.widgets"]
        if sys.platform == "win32":
            test_lib_prefix = ""
            test_lib_suffix = ".dll"
        elif sys.platform == "darwin":
            test_lib_prefix = "lib"
            test_lib_suffix = ".dylib"
        else: # Linux
            test_lib_prefix = "lib"
            test_lib_suffix = ".so"
        for lib_base_name_test in core_lib_names_test:
            with open(cmake_output_dir / f"{test_lib_prefix}{lib_base_name_test}{test_lib_suffix}", "w") as f:
                f.write(f"dummy {lib_base_name_test} content")

        # Create dummy actual plugin file in the "CMake output plugins" location
        with open(cmake_output_plugins_dir / "dummy_plugin.so", "w") as f: f.write("dummy plugin content")

        # Call _copy_artifacts with the mock directories
        hook._copy_artifacts(mock_build_dir_for_test, mock_package_dir_for_test)

        # Assert that artifacts were copied to the mock package directory
        assert (mock_package_dir_for_test / exe_name).exists(), \
            f"Executable {exe_name} not found in mock package dir."
        for lib_base_name_test in core_lib_names_test:
            assert (mock_package_dir_for_test / f"{test_lib_prefix}{lib_base_name_test}{test_lib_suffix}").exists(), \
                f"Core library {lib_base_name_test} not found in mock package dir."
        assert (mock_package_dir_for_test / "plugins" / "dummy_plugin.so").exists(), \
            "Dummy plugin not found in mock package plugins dir."

        print("Artifact copying logic test successful.")

    except Exception as e:
        print(f"Error during build script test: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
    finally:
        # Clean up dummy dirs after test
        if mock_build_dir_for_test.exists(): shutil.rmtree(mock_build_dir_for_test)
        if mock_package_dir_for_test.exists(): shutil.rmtree(mock_package_dir_for_test)
        print("Test cleanup finished.")

# Note: The class name is CustomBuildHook.
# pyproject.toml should have:
# [tool.hatch.build.targets.wheel.hooks.custom]
# path = "hatch_build.py:CustomBuildHook"
# This was already set.
