# Image Batch Processor (IBP)

![](docs/assets/img/ibp-sm.png)
<!-- TODO: Consider adding an animated GIF showcasing the GUI in action -->

[![CI/CD Pipeline](https://github.com/twardoch/imagebatchprocessor/actions/workflows/ci_cd.yml/badge.svg)](https://github.com/twardoch/imagebatchprocessor/actions/workflows/ci_cd.yml)
[![PyPI version](https://badge.fury.io/py/imagebatchprocessor.svg)](https://badge.fury.io/py/imagebatchprocessor)

## What is Image Batch Processor (IBP)?

Image Batch Processor (IBP) is a powerful, cross-platform, open-source tool designed for automating image processing tasks. Written in C++ with a user-friendly Qt-based graphical interface and a versatile command-line interface, IBP allows you to apply a sequence of image manipulation effects (filters) to one or many images efficiently. Whether you need to resize a hundred photos, apply artistic effects to a collection, or perform complex image adjustments for research, IBP provides a flexible and extensible solution.

This project modernizes the original IBP's packaging and development setup using Python, Hatch, uv, Ruff, MyPy, and GitHub Actions.

## Key Features

*   **Batch Processing:** Apply a defined sequence of filters to multiple images automatically.
*   **Sequential Filters & Effects:** Chain multiple image processing operations to create complex visual results.
*   **Extensible Plugin Architecture:** A vast array of built-in plugins provide diverse functionalities (color correction, geometric transformations, blurs, sharpening, denoising, keying, etc.). Easily extend IBP by creating new C++ plugins.
*   **Real-time Preview (GUI):** Interactively adjust filter parameters and see their effects in real-time within the graphical user interface.
*   **Dual Interface (GUI & CLI):** Work visually with the intuitive GUI or leverage the command-line interface for scripting, automation, and headless operation.
*   **Cross-Platform:** Runs on Linux, macOS, and Windows.
*   **Save/Load Filter Sequences:** Store your customized filter chains as `.ifl` files (INI format) for easy reuse and sharing.
*   **Modernized Python Packaging:** Simplified installation and development workflow using current Python best practices.

## Who is it for?

IBP is designed for a diverse range of users, including:

*   **Photographers & Designers:** Quickly apply consistent adjustments, watermarks, or effects to batches of photos or design assets.
*   **Researchers & Scientists:** Automate image processing pipelines for experiments, data analysis, and visualization.
*   **Web Developers & Content Creators:** Optimize images for web use, create thumbnails, or batch-convert formats.
*   **Hobbyists & Enthusiasts:** Experiment with a wide array of image filters and create custom image processing workflows without needing to write code.
*   **Software Developers:** Integrate IBP's processing capabilities into other applications or extend its functionality by creating new plugins.

## Why is it useful?

IBP offers several key advantages:

*   **Automation & Efficiency:** Save significant time and effort by processing multiple images with a predefined set of filters, eliminating repetitive manual work.
*   **Customizable Workflows:** Define and save sequences of image filters (as `.ifl` files) to create reusable processing pipelines tailored to your specific needs.
*   **Extensibility through Plugins:** A rich plugin architecture allows for a wide variety of image operations, from basic adjustments (brightness, contrast, resize) to advanced filtering (denoising, edge detection, color manipulation) and even specialized scientific image processing. New plugins can be developed to add unique functionalities.
*   **Cross-Platform Compatibility:** Run IBP on Windows, macOS, and Linux, ensuring consistent results across different operating systems.
*   **Dual Interface (GUI & CLI):** Use the intuitive graphical interface for interactive work and real-time previews, or leverage the command-line interface for scripting and integration into automated workflows.
*   **Open Source:** Free to use, modify, and distribute (MIT License).
*   **Powerful Underpinnings:** Leverages established libraries like Qt for the interface, OpenCV and FreeImage for robust image processing capabilities.

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
    From PyPI:
    ```bash
    uv pip install imagebatchprocessor
    ```
    Or, to install directly from the source repository (e.g., after cloning):
    ```bash
    uv pip install .
    ```

This will install the `imagebatchprocessor` command-line tool and the underlying C++ application with its plugins.

## Usage

### Graphical User Interface (GUI)

To launch the GUI, simply run:
```bash
imagebatchprocessor
```
This will open the main application window, allowing you to load images, define a list of filters from the available plugins, adjust their parameters with real-time preview, and process them individually or in batch.

### Command-Line Interface (CLI)

The application also supports command-line operations for automated processing.

*   **Show version:**
    ```bash
    imagebatchprocessor --version
    ```

*   **Show help:**
    ```bash
    imagebatchprocessor --help
    ```
    This will display available options for CLI processing, such as specifying input/output files/directories and filter list files.

*   **Basic CLI Processing Example:**
    (Assuming you have an input image `input.jpg` and a filter list file `filters.ifl`)
    ```bash
    imagebatchprocessor -i input.jpg -o output.png -l filters.ifl
    ```
    This command applies the filters defined in `filters.ifl` to `input.jpg` and saves the result as `output.png`.

*   **Example: Processing multiple files (conceptual - check `--help` for exact syntax if supported for multiple inputs directly):**
    While direct globbing for input files in one command might depend on shell expansion, you can typically script this:
    ```bash
    # Example for bash/zsh
    mkdir -p processed_images
    for img in *.jpg; do
      imagebatchprocessor -i "$img" -o "processed_images/processed_$img" -l my_effects.ifl
    done
    ```
    Refer to `imagebatchprocessor --help` and the original application documentation for details on filter list file format (`.ifl`) and all available CLI options.

---

## Technical Deep-Dive: How It Works

Image Batch Processor (IBP) combines a robust C++ core for image processing and GUI management with modern Python tooling for building and packaging.

### Core Application (C++/Qt)

The heart of IBP is a C++ application built using the [Qt framework](https://www.qt.io/). Qt provides the cross-platform capabilities for the graphical user interface (GUI), event handling, file system interaction, and more.

*   **Main Window (`MainWindow`):** The central class (`src/ibp/imagebatchprocessor/mainwindow.h/cpp`) manages the user interface, including the image viewing areas, filter list display, toolbars, and dialogs. It orchestrates user interactions, such as loading images, adding/removing filters, and initiating processing.
*   **Event Handling:** Qt's signal and slot mechanism is used extensively for communication between UI elements and backend logic.
*   **Resource Management:** Application resources like icons are managed via Qt's resource system (`.qrc` files).

### Image Processing Engine

The image processing capabilities are primarily handled by the `src/ibp/imgproc/` library.

*   **`ImageFilter` Interface:** The cornerstone of the processing engine is the abstract base class `ImageFilter` (`src/ibp/imgproc/imagefilter.h`). Each specific image operation (e.g., blur, resize, color adjustment) is implemented as a concrete class derived from `ImageFilter`. Key methods include:
    *   `process(const QImage &inputImage)`: Applies the filter to the input image and returns the processed `QImage`.
    *   `clone()`: Creates a copy of the filter instance.
    *   `info()`: Returns metadata about the filter (name, description).
    *   `loadParameters(QSettings &s)` and `saveParameters(QSettings &s)`: Handle serialization of filter settings to/from `.ifl` files (which use an INI format via `QSettings`).
    *   `widget(QWidget *parent)`: Returns a Qt widget (if any) that provides a GUI for configuring the filter's parameters.
*   **`ImageFilterList`:** This class (`src/ibp/imgproc/imagefilterlist.h/cpp`) manages an ordered list of `ImageFilter` pointers. It is responsible for:
    *   Sequentially applying each filter in the list to an image.
    *   Loading and saving filter configurations (sequences of filters and their parameters) from/to `.ifl` files.
    *   Managing the processing pipeline, often executing it in a separate thread (`QThread`) to keep the GUI responsive.
*   **Underlying Libraries:** IBP leverages powerful third-party libraries for image manipulation:
    *   **OpenCV:** Used for a wide range of image processing algorithms (e.g., blurs, denoising, feature detection, morphological operations) within many plugins.
    *   **FreeImage:** Used for loading and saving a broad variety of image file formats.
    *   **Little CMS (lcms2):** Used for color management operations.
    *   **Eigen3:** Used for numerical computations, particularly in filters involving complex calculations like curves.

### Plugin Architecture

IBP's functionality is highly extensible through a plugin system, managed by the `src/ibp/plugins/` C++ library (not to be confused with `src/plugins/` which contains the actual plugin source code).

*   **`ImageFilterPluginLoader`:** This class (`src/ibp/plugins/imagefilterpluginloader.h/cpp`) is responsible for discovering and loading plugins at runtime.
    *   Plugins are compiled as shared libraries (e.g., `.dll` on Windows, `.so` on Linux, `.dylib` on macOS).
    *   The loader scans a designated `plugins` directory (packaged alongside the main application) for these shared libraries.
    *   It uses `QLibrary` to load each valid plugin and interact with it.
*   **Plugin Structure:** Each individual plugin (located in its own subdirectory within `src/plugins/imagefilter_*`) typically consists of:
    *   A `Filter` class inheriting from `ibp::imgproc::ImageFilter`, implementing the actual processing logic.
    *   A `FilterWidget` class (optional) inheriting from `QWidget`, providing the GUI for parameter adjustment. This is often designed using Qt Designer (`.ui` file).
    *   A `main.cpp` file that includes C-style exported functions (e.g., `getIBPPluginInfo`, `instantiateImageFilter`) which `ImageFilterPluginLoader` uses to get plugin metadata and create filter instances.
    *   A `filter.yaml` file (generated by `plugins_scan.py` during development) containing metadata such as the plugin's ID, name, description, and parameter details. This YAML file is used by `plugins_convert.py` to generate example `.ifl` files for documentation.
*   **Dynamic Loading:** This architecture allows new image processing capabilities to be added to IBP simply by dropping a new plugin library into the `plugins` directory, without needing to recompile the main application.

### Build System (CMake & Hatch)

The project uses a two-tiered build system:

*   **CMake:** The C++ core application and all its plugins are built using CMake (`CMakeLists.txt` files). CMake handles finding C++ dependencies (Qt, OpenCV, FreeImage, etc.) and compiling the C++ code into executables and shared libraries.
*   **Hatch (with `hatch_build.py`):** The Python packaging layer, defined by `pyproject.toml`, uses Hatch as the build frontend.
    *   A custom build hook (`hatch_build.py`) orchestrates the CMake build process when the Python package is built (e.g., via `uv pip install .`).
    *   This script invokes CMake to compile the C++ components and then copies the necessary artifacts (the `ibp` executable, core `ibp.*` libraries, and individual `imagefilter_*.so/dll/dylib` plugin files) into the correct locations within the Python package structure (`src/imagebatchprocessor/`).

### CLI vs. GUI Operation

IBP can be run as a GUI application or as a command-line tool.

*   **GUI Mode:** When launched without specific CLI arguments (or via `imagebatchprocessor` after installation), the Qt-based `MainWindow` is displayed, offering an interactive experience.
*   **CLI Mode:** The `main.cpp` (`src/ibp/imagebatchprocessor/main.cpp`) uses Qt's `QCommandLineParser` to handle command-line arguments. If CLI arguments for processing are provided (e.g., input file, output file, filter list), the application bypasses the full GUI display and directly calls methods within `MainWindow` (like `applyFiltersAndSave`) to perform the image processing tasks and then exits. This allows for scripting and automation of batch jobs.

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
├── cmake/                  # Custom CMake modules (e.g., FindFreeImage.cmake)
├── dist/                   # Distribution artifacts (wheels, sdist) built by Hatch
├── hatch_build.py          # Hatch build hook for CMake integration
├── plugins_*.py            # Utility scripts for plugin management (scan, convert)
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
│   └── plugins/            # Source code for individual C++ plugins (e.g., imagefilter_addnoise)
└── tests/
    ├── __init__.py
    └── test_packaging.py   # Python tests for packaging and basic CLI functionality
```

## Contributing

We warmly welcome contributions to Image Batch Processor! Whether you're fixing a bug, adding a new feature or plugin, or improving documentation, your help is appreciated.

### Setting up a Development Environment

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/twardoch/imagebatchprocessor.git # Or your fork
    cd imagebatchprocessor
    ```

2.  **Install C++ Dependencies:**
    *   **Linux:** `sudo apt-get install build-essential cmake qtbase5-dev libqt5widgets5 libopencv-dev libfreeimage-dev` (or equivalent for your distribution like `qt5-qtbase-devel`, `opencv-devel`, `freeimage-devel`).
    *   **macOS:** `brew install cmake qt@5 opencv freeimage`. You will likely need to set `CMAKE_PREFIX_PATH`. For example:
        ```bash
        export CMAKE_PREFIX_PATH=$(brew --prefix qt@5):$(brew --prefix opencv):$(brew --prefix freeimage)
        ```
    *   **Windows:** Requires Visual Studio (Community Edition with C++ workload is fine), CMake, Qt5, OpenCV, and FreeImage. Using a package manager like `vcpkg` is highly recommended:
        *   With `vcpkg` (after installing it):
            ```bash
            vcpkg install qt5-base opencv4 freeimage --triplet x64-windows
            ```
            Then, when configuring CMake, point it to the vcpkg toolchain file using the `-DCMAKE_TOOLCHAIN_FILE` option.

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
    2.  Configure CMake (adjust paths and options as needed, especially `CMAKE_PREFIX_PATH` for dependencies on macOS/Linux if not found automatically, or `CMAKE_TOOLCHAIN_FILE` for vcpkg on Windows):
        ```bash
        # Example for Linux/macOS (adjust CMAKE_PREFIX_PATH if needed)
        cmake .. -DCMAKE_BUILD_TYPE=Debug -DIBP_BUILD_PLUGINS=ON
        # Example for Windows with vcpkg
        # cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake -DIBP_BUILD_PLUGINS=ON
        ```
    3.  Build:
        ```bash
        cmake --build . --config Debug --parallel
        ```
    The executables and libraries will be in `build_cpp/build/` (or as configured by `IBP_OUTPUT_DIRECTORY` in CMake).

### Coding Conventions

Adhering to consistent coding conventions helps maintain code quality and readability.

**Python:**

*   Code is automatically formatted using **Ruff** and type-checked using **MyPy**.
*   Pre-commit hooks are configured (`.pre-commit-config.yaml`) to run these checks automatically before each commit. Please ensure they pass.
*   Configuration for Ruff and MyPy can be found in `pyproject.toml`.

**C++:**

*   **C++ Standard:** The project primarily uses **C++11**. New code should adhere to this standard.
*   **Style Guide:** While a strict `clang-format` style isn't enforced via pre-commit hooks yet, contributions should aim for consistency with the existing codebase. As a general guideline, consider principles from common styles like the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) or [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html), particularly regarding naming conventions, formatting, and readability.
    *   **Naming:**
        *   Classes: `PascalCase` (e.g., `ImageFilterList`)
        *   Functions/Methods: `camelCase` (e.g., `loadImageFile`) or `PascalCase` for Qt slots.
        *   Member variables: `mCamelCase` (e.g., `mViewEditInputImage`) is prevalent.
        *   Constants: `UPPER_SNAKE_CASE` (e.g., `DEFAULT_TIMEOUT`).
    *   **Formatting:** Use spaces for indentation (typically 4). Keep lines reasonably short (e.g., under 100-120 characters).
*   **Qt Conventions:**
    *   Follow Qt naming conventions for signals and slots (e.g., slots often start with `on_objectName_signalName`).
    *   Use `Q_OBJECT` macro for classes defining signals or slots.
    *   Utilize Qt's container classes (e.g., `QString`, `QList`, `QHash`) where appropriate.
    *   Leverage Qt's parent-child ownership model for `QObject` memory management.
*   **Error Handling:**
    *   Use exceptions for truly exceptional circumstances.
    *   For functions where failure is common, return boolean success/failure flags and provide error details via output parameters or Qt's logging (`qWarning`, `qDebug`).
*   **Memory Management:**
    *   For Qt objects (`QObject` derived), rely on Qt's parent-child ownership system.
    *   For non-Qt objects, use RAII. Employ smart pointers (`std::unique_ptr`, `std::shared_ptr`) where appropriate. Avoid raw `new` and `delete` where alternatives exist.
*   **Includes:** Include headers precisely. Prefer forward declarations in header files when possible to reduce compilation dependencies.
*   **Comments:** Write clear and concise comments. Doxygen-style comments are encouraged for public APIs.

### Plugin Development

To add a new C++ image filter plugin:
1.  **Directory & Files:**
    *   Create a new directory `src/plugins/imagefilter_mypluginname/`.
    *   Copy the structure from an existing simple plugin (e.g., `imagefilter_identity` or `imagefilter_invert`) or use it as a template. This includes:
        *   `CMakeLists.txt` (update target names, link necessary libraries like OpenCV if used)
        *   `filter.h` & `filter.cpp` (your `ImageFilter` derivative)
        *   `filterwidget.h` & `filterwidget.cpp` & `filterwidget.ui` (for the GUI, optional)
        *   `main.cpp` (plugin registration boilerplate)
2.  **Implement `ImageFilter`:**
    *   In `filter.h/.cpp`, define your class inheriting from `ibp::imgproc::ImageFilter`.
    *   Implement all pure virtual methods: `clone()`, `info()`, `process()`, `loadParameters()`, `saveParameters()`, `widget()`.
    *   The `process()` method is where your core image manipulation logic resides.
3.  **Implement `FilterWidget` (Optional):**
    *   If your filter has configurable parameters, create a widget inheriting from `QWidget`.
    *   Design its UI in `filterwidget.ui` using Qt Designer.
    *   Connect signals from your widget's controls to slots in your `Filter` class to update parameters, and emit `parametersChanged()` from the `Filter` class.
4.  **Plugin Registration (`main.cpp`):**
    *   Ensure `getIBPPluginInfo()` returns correct metadata (name, description). It usually calls `QObject::tr()` for internationalization.
    *   Ensure `instantiateImageFilter()` creates an instance of your new filter.
5.  **Register in CMake:**
    *   Add `add_subdirectory(imagefilter_mypluginname)` to `src/plugins/CMakeLists.txt`.
6.  **Metadata (`filter.yaml`):**
    *   After initial C++ implementation, run `uv run plugins_scan.py` from the project root. This script will inspect your plugin's source and generate/update a `filter.yaml` file in your plugin's directory. Review this YAML file. It is used by `plugins_convert.py` to create example `.ifl` files (used for documentation/testing).
7.  **Build & Test:** Rebuild the project and test your plugin thoroughly in the IBP application.

### Commit Messages

*   Please follow [Conventional Commits](https://www.conventionalcommits.org/) guidelines.
    *   Example: `feat: add gaussian blur filter with adjustable radius`
    *   Example: `fix(core): correct memory leak in plugin loader`
    *   Example: `docs: update README with new build instructions`

### Running Tests

*   **Python:** Ensure your virtual environment is active and development dependencies are installed.
    ```bash
    pytest
    ```
    To run only integration tests (which execute the C++ app):
    ```bash
    pytest -m integration
    ```
*   **C++:** Currently, there isn't a dedicated C++ test suite integrated. Contributions in this area (e.g., adding Google Test or a similar framework for unit testing core libraries and plugins) would be highly valuable. When adding new C++ functionality, consider its testability.

### Submitting Changes

1.  Create a feature branch from the main development branch.
2.  Make your changes. Ensure pre-commit checks pass for any Python code.
3.  Add tests for new functionality if applicable (especially Python tests for CLI or packaging changes).
4.  Push your branch to your fork and open a Pull Request against the main repository's development branch.
5.  Clearly describe your changes in the Pull Request.

## Available Plugins (Overview)

IBP comes with a rich set of over 40 plugins, offering a wide range of image processing capabilities, including:

*   **Color Adjustments:** Levels, Curves, Brightness/Contrast, Color Balance, Hue/Saturation, Grayscale, Desaturate, Invert, etc.
*   **Geometric Operations:** Resize, Resample, Flip, Rotate, Auto Trim.
*   **Blurring & Sharpening:** Gaussian Blur, Box Blur, Median Blur, Bilateral Filter, Unsharp Mask.
*   **Denoising:** NLM Denoising, DCT Denoising, TV Denoising, Guided Filter.
*   **Keying & Masking:** Luma Keyer, HSL Keyer.
*   **Illumination Correction:** Various IIHC filters (Inpainting, Low Pass, Morphological, ITK N4).
*   **Morphological Operations:** Dilation, Erosion, Opening, Closing.
*   **Artistic & Effects:** Add Noise, Color Layer, Texture Layer.
*   **And many more...**

For a detailed list of plugins, their parameters, and example images, please refer to the [original IBP documentation site](https://twardoch.github.io/ibp/plugins/). Example filter list files (`.ifl`) for each plugin can also be found in the `docs/plugins/` directory of this repository.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

Image Batch Processor was originally created by Deif Lou. This version includes modernization of the build system, packaging, and ongoing maintenance by Adam Twardoch, along with contributions from the open-source community.

## Roadmap / Future Enhancements (Ideas)

*   More comprehensive C++ unit and integration test suite (e.g., using Google Test).
*   Exploration of newer C++ standards (C++17/20) where beneficial.
*   Continuous integration testing for Windows and various Linux distributions.
*   UI/UX refinements.
*   Your great ideas and contributions!

---
*For more detailed information on specific plugins and older documentation, please visit the original [Image Batch Processor documentation site](https://twardoch.github.io/ibp/).*
```
