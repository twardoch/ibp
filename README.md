# Image Batch Processor (IBP)

![](docs/assets/img/ibp-sm.png) <!-- Existing image, seems appropriate -->

Image Batch Processor (IBP) is a cross-platform, open-source tool written in C++/Qt for applying a series of image processing effects to one or more images in an automated fashion. This project modernizes its packaging and development setup using Python, Hatch, uv, Ruff, MyPy, and GitHub Actions.

<!-- Optional: Add badges here once CI is reliably passing, e.g.,
[![CI/CD Pipeline](https://github.com/YOUR_USERNAME/YOUR_REPOSITORY/actions/workflows/ci_cd.yml/badge.svg)](https://github.com/YOUR_USERNAME/YOUR_REPOSITORY/actions/workflows/ci_cd.yml)
[![PyPI version](https://badge.fury.io/py/imagebatchprocessor.svg)](https://badge.fury.io/py/imagebatchprocessor)
-->

## Rationale

The core C++ application provides powerful image processing capabilities. This project aims to:
*   Modernize its build and packaging process using contemporary Python tooling.
*   Simplify installation for end-users via Python package managers.
*   Improve developer experience with standardized linting, formatting, and testing.
*   Automate CI/CD using GitHub Actions for robust development and releases.
*   Ensure continued maintainability and cross-platform compatibility.

## Features

*   **Batch Processing:** Apply a sequence of filters to multiple images.
*   **Plugin Architecture:** Extend functionality with various image processing plugins. (Details on existing plugins can be found in the original documentation).
*   **GUI and CLI:** Interactive graphical user interface and command-line operation.
*   **Cross-Platform:** Designed to run on Linux, macOS, and Windows.

## Installation

This project is packaged as a Python package that includes the compiled C++ application.

**Prerequisites:**
*   Python 3.8+
*   `uv` (recommended Python package installer and virtual environment manager)

**Steps:**

1.  **Install `uv`:**
    ```bash
    curl -LsSf https://astral.sh/uv/install.sh | sh
    ```
    (On Windows, you might need to download the installer from the `uv` releases page or use `pip install uv` if pip is already available).

2.  **Create a virtual environment (recommended):**
    ```bash
    uv venv .venv  # Creates a virtual environment named .venv
    source .venv/bin/activate  # On Linux/macOS
    # .venv\Scripts\activate  # On Windows
    ```

3.  **Install Image Batch Processor:**
    From PyPI (once published):
    ```bash
    uv pip install imagebatchprocessor
    ```
    Or, to install directly from the source repository (e.g., after cloning):
    ```bash
    uv pip install .
    ```

This will install the `imagebatchprocessor` command-line tool and the underlying C++ application.

## Usage

### Graphical User Interface (GUI)

To launch the GUI, simply run:
```bash
imagebatchprocessor
```
This will open the main application window, allowing you to load images, define a list of filters, and process them.

### Command-Line Interface (CLI)

The application also supports command-line operations.

*   **Show version:**
    ```bash
    imagebatchprocessor --version
    ```

*   **Show help:**
    ```bash
    imagebatchprocessor --help
    ```
    This will display available options for CLI processing, such as specifying input/output files and filter lists.

*   **Basic CLI Processing Example:**
    (Assuming you have an input image `input.jpg` and a filter list file `filters.ifl`)
    ```bash
    imagebatchprocessor -i input.jpg -o output.png -l filters.ifl
    ```
    This command would apply the filters defined in `filters.ifl` to `input.jpg` and save the result as `output.png`. Refer to `imagebatchprocessor --help` and the original application documentation for details on filter list file format and available CLI options.

## Codebase Structure

### Overview

The project combines a C++ core application with Python-based tooling for building, packaging, and developer utilities.
*   **C++ Core (`src/`):** Contains the Qt-based GUI application, image processing logic, and plugin system.
    *   `src/ibp/`: Main application libraries (misc, imgproc, widgets, plugin loader).
    *   `src/plugins/`: Source code for individual image processing plugins.
*   **Python Packaging (`pyproject.toml`, `hatch_build.py`):**
    *   `pyproject.toml`: Defines the Python package, dependencies, and build system (Hatch).
    *   `hatch_build.py`: A build hook for Hatch that orchestrates the CMake build of the C++ application and copies artifacts into the Python package structure.
    *   `src/imagebatchprocessor/`: The Python package itself, which includes a simple `main()` entry point to launch the C++ application and will contain the packaged C++ binaries and plugins.

### Directory Layout

```
.
├── .github/workflows/      # GitHub Actions CI/CD workflows
├── .gitignore
├── .pre-commit-config.yaml # Pre-commit hook configurations
├── CMakeLists.txt          # Root CMake file for the C++ project
├── LICENSE                 # Project license (MIT)
├── README.md               # This file
├── VERSION.TXT             # Original version file (C++ app might still use this)
├── build/                  # Original CMake build directory (not used by Hatch builds)
├── cmake/                  # Custom CMake modules (e.g., FindFreeImage.cmake)
├── dist/                   # Distribution artifacts (wheels, sdist) built by Hatch
├── hatch_build.py          # Hatch build hook for CMake integration
├── plugins_*.py            # Original utility scripts (may be deprecated or integrated)
├── pyproject.toml          # Python project definition and build configuration
├── src/
│   ├── CMakeLists.txt      # Top-level CMake for all C++ sources
│   ├── ibp/                # Core C++ libraries for the application
│   │   ├── imagebatchprocessor/ # Main GUI application C++ code
│   │   ├── imgproc/        # Image processing C++ library
│   │   ├── misc/           # Miscellaneous C++ utilities
│   │   ├── plugins/        # C++ Plugin loader library
│   │   └── widgets/        # Custom Qt widgets C++ code
│   ├── imagebatchprocessor/  # Python package source
│   │   ├── __init__.py     # Python package entry point
│   │   └── (compiled C++ artifacts will be placed here by hatch_build.py)
│   └── plugins/            # Source code for individual C++ plugins
└── tests/
    ├── __init__.py
    └── test_packaging.py   # Python tests for packaging and basic CLI functionality
```

### Build System

1.  **Hatch:** The primary build frontend for the Python package. It's defined in `pyproject.toml`.
2.  **`hatch_build.py`:** This custom Hatch build hook is triggered when `hatch build` (or `uv pip install .`) is run.
    *   It invokes `cmake` to configure and build the C++ application located in `src/`.
    *   It ensures C++ build dependencies (like Qt, OpenCV, FreeImage) are found by CMake.
    *   After a successful C++ build, it copies the main executable (`ibp`), core shared libraries (`libibp.*.so/dll/dylib`), and plugin shared objects (`imagefilter_*.so/dll/dylib`) into the `src/imagebatchprocessor/` directory.
3.  **CMake:** Used to build the C++ application and its plugins from the `src/` directory. `CMakeLists.txt` files define targets and dependencies.
4.  **Python Package:** The `src/imagebatchprocessor` directory becomes the Python package, containing the C++ artifacts and a Python entry point (`__init__.py`) that knows how to launch the `ibp` executable.

### Plugin System

The C++ application uses a plugin system:
*   Core plugin functionality is managed by the `ibp.plugins` C++ library.
*   Individual plugins (e.g., `imagefilter_addnoise`) are compiled into separate shared libraries.
*   These plugin libraries are loaded at runtime by the application from a designated `plugins` subdirectory (which is packaged within `src/imagebatchprocessor/plugins/`).

## Contributing

We welcome contributions! Please follow these guidelines:

### Setting up a Development Environment

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/YOUR_USERNAME/imagebatchprocessor.git # Replace with actual repo URL
    cd imagebatchprocessor
    ```

2.  **Install C++ Dependencies:**
    *   **Linux:** `sudo apt-get install build-essential cmake qtbase5-dev libqt5widgets5 libopencv-dev libfreeimage-dev` (or equivalent for your distribution).
    *   **macOS:** `brew install cmake qt5 opencv freeimage`. You might need to set `CMAKE_PREFIX_PATH` for Qt5, e.g., `export CMAKE_PREFIX_PATH=$(brew --prefix qt@5):$(brew --prefix opencv):$(brew --prefix freeimage)`
    *   **Windows:** Requires Visual Studio (Community Edition with C++ workload is fine), CMake, Qt5, OpenCV, and FreeImage. Using a package manager like `vcpkg` is highly recommended for managing these C++ dependencies.
        *   With `vcpkg`, after installing it, you might run:
            ```bash
            vcpkg install qt5-base opencv4 freeimage --triplet x64-windows
            ```
            Then, when configuring CMake, you'd point it to the vcpkg toolchain file:
            `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake`

3.  **Install Python tools using `uv`:**
    ```bash
    uv venv .venv
    source .venv/bin/activate  # Or .venv\Scripts\activate on Windows
    uv pip install -e ".[dev]" # Install in editable mode with dev dependencies
    ```

4.  **Set up pre-commit hooks:**
    ```bash
    pre-commit install
    ```

### Building Locally

*   **To build the Python package (which includes the C++ application):**
    ```bash
    hatch build
    ```
    This will produce a wheel and sdist in the `dist/` directory. The `hatch_build.py` script handles the CMake build.

*   **For iterative C++ development:**
    You might want to run CMake and your C++ build tools directly for faster iteration on C++ code.
    1.  Create a build directory: `mkdir build_cpp && cd build_cpp`
    2.  Configure CMake (adjust paths and options as needed, especially `CMAKE_PREFIX_PATH` or `CMAKE_TOOLCHAIN_FILE` for dependencies):
        ```bash
        cmake .. -DCMAKE_BUILD_TYPE=Debug -DIBP_BUILD_PLUGINS=ON # Add -DCMAKE_TOOLCHAIN_FILE for vcpkg if used
        ```
    3.  Build:
        ```bash
        cmake --build . --config Debug --parallel
        ```
    The executables and libraries will be in `build_cpp/build/` (or as configured by `IBP_OUTPUT_DIRECTORY`).

### Running Tests

Ensure your virtual environment is active and development dependencies are installed.
```bash
pytest
```
To run only integration tests (which execute the C++ app):
```bash
pytest -m integration
```

### Coding Conventions

*   **Python:** Code is formatted and linted using `Ruff`. Pre-commit hooks will enforce this. Configuration is in `pyproject.toml`.
*   **C++:**
    *   The project uses C++11.
    *   (If specific C++ style guides or linters like clang-format are introduced, document them here.)

### Submitting Changes

1.  Create a feature branch.
2.  Make your changes. Ensure pre-commit checks pass.
3.  Add tests for new functionality.
4.  Push your branch and open a Pull Request against the main development branch.

### How to Add a New C++ Plugin (High-Level)

1.  **Create a new plugin directory:** Copy an existing plugin directory from `src/plugins/imagefilter_exampleplugin` (if one exists, otherwise adapt from an existing plugin) to `src/plugins/imagefilter_myNewPlugin`.
2.  **Rename files and update CMakeLists.txt:** Inside your new plugin directory, rename `.cpp`, `.h`, `.ui`, `.yaml` files and update `src/plugins/imagefilter_myNewPlugin/CMakeLists.txt` with the new target name.
3.  **Implement plugin logic:** Modify the C++ source files (`filter.cpp`, `filter.h`, `filterwidget.cpp`, `filterwidget.h`, `main.cpp`) to implement your new filter's functionality and its GUI.
4.  **Update plugin metadata:** Edit `filter.yaml` with the details of your new plugin.
5.  **Register the plugin:** Add your new plugin's subdirectory to `src/plugins/CMakeLists.txt` (e.g., `add_subdirectory(imagefilter_myNewPlugin)`).
6.  Rebuild the project. Your new plugin should now be available in the application.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---
*Original Image Batch Processor documentation and further details on its C++ functionalities can be found at [https://twardoch.github.io/ibp/](https://twardoch.github.io/ibp/).*
