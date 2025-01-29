
# Image Batch Processor (IBP) Technical Overview

## Overview

The Image Batch Processor (IBP) is a cross-platform, open-source tool written in C++/Qt designed for automatically transforming one or more images based on a list of effects. The primary goal of IBP is to provide a set of filters that aid in achieving animation-specific effects, such as cleaning cels, correcting non-uniform illumination, and keying.

## Project Structure

The project is organized into several key directories and files, each serving a specific purpose:

- **Root Directory:** Contains project-level configuration, build scripts, and documentation.
- **`src/`:** Contains the source code for the main application and its components.
- **`cmake/`:** Contains CMake modules used for finding external dependencies.
- **`doc/`:** Contains documentation, including images used in the README.
- **`resources/`:** Contains application resources such as icons.
- **`plugins/`:** Contains source code for various image filter plugins.

## Root Directory Files

### `.gitignore`

This file specifies intentionally untracked files that Git should ignore. It includes common build artifacts, temporary files, and editor-specific files to keep the repository clean.

### `BUILDING.md`

This file provides instructions for building the project using CMake. It includes specific flags and environment variables for compiling with Qt5, lcms2, and FreeImage libraries on macOS.

### `CMakeLists.txt`

This is the top-level CMake configuration file. It sets the minimum required CMake version, includes the GNUInstallDirs module, defines the project name and version, and includes the `src` subdirectory for further processing.

### `LICENSE`

This file contains the MIT License under which the project is released. It details the permissions, conditions, and limitations of using, modifying, and distributing the software.

### `README.md`

This file provides a general overview of the project, its purpose, development status, and snapshots of its user interface. It also includes a brief introduction to the main developer, Deif Lou.

## CMake Modules (`cmake/modules/`)

### `FindFreeImage.cmake`

This CMake module is used to locate the FreeImage library. It defines:

- `FreeImage_FOUND`: Whether FreeImage was found.
- `FreeImage_INCLUDE_DIRS`: Include directories for FreeImage.
- `FreeImage_LIBRARIES`: Libraries needed to link against FreeImage.

It searches for `FreeImage.h` in common installation directories and sets the appropriate variables.

## Documentation Images (`doc/images/`)

This folder contains images used in the `README.md` to demonstrate the functionality of various filters, such as color correction, illumination correction, and HSL keying. The images are provided in both thumbnail and full-size formats.

## Source Code (`src/`)

### `src/CMakeLists.txt`

This file configures the build for the `src` directory. It sets the C++ standard to C++11, defines the output directory for the build, and includes subdirectories for `ibp` and `plugins`.

### `src/ibp/`

This directory contains the source code for the main application components, organized into subdirectories for different parts of the application.

#### `src/ibp/imagebatchprocessor/`

- **`CMakeLists.txt`**: Configures the build for the `ibp` executable, including linking against necessary libraries and setting target properties such as `AUTOMOC`, `AUTORCC`, and `AUTOUIC`.
- **`imagebatchprocessor.qrc`**:  A Qt resource file that lists icons used by the application's user interface.
- **`main.cpp`**: The entry point of the application. Initializes Qt resources, sets application name and style, handles configuration paths, and creates the main window.
- **`mainwindow.h`**: The header file for the main application window. Defines the `MainWindow` class, including member variables, slots, and methods related to the UI, image processing, and plugin loading.
- **`mainwindow.main.cpp`**: Implements the main functionality of the `MainWindow` class, including loading and unloading the main components, reloading image filter list presets, and handling resize, move, and show events.
- **`mainwindow.toolbar.batch.cpp`**: Implements the toolbar for batch processing.
- **`mainwindow.toolbar.edit.cpp`**: Implements the toolbar for the edit view, including functionalities for loading/saving images, adding/removing filters, and managing filter lists.
- **`mainwindow.ui`**: The Qt Designer UI file defining the layout and widgets of the main window.
- **`mainwindow.view.batch.cpp`**: Implements the batch processing view.
- **`mainwindow.view.edit.cpp`**: Implements the edit view, including image preview, zoom controls, and the image filter list.

#### `src/ibp/imgproc/`

- **`CMakeLists.txt`**: Configures the build for the `ibp.imgproc` shared library, including finding necessary packages like OpenCV and FreeImage.
- **`colorconversion.cpp/h`**: Implements color conversion functions using the Little CMS color management system (lcms2).
- **`freeimage.cpp/h`**: Provides functions for loading and saving images using the FreeImage library, including filter string generation for open/save dialogs.
- **`imagefilter.h`**: Defines the base class `ImageFilter` for all image filters.
- **`imagefilterlist.cpp/h`**: Implements the `ImageFilterList` class, which manages a list of image filters, handles their processing, and supports loading and saving filter lists from/to files.
- **`lut.h`**: Defines lookup tables used in various image processing operations.
- **`lut01.cpp`, `lut02.cpp`, `lut03.cpp`**: Implementations of specific lookup table calculations.
- **`util.cpp/h`**: Provides utility functions for image processing.
- **`pixelblending.cpp/h`**: Implements various pixel blending algorithms.
- **`intensitymapping.cpp/h`**: Implements intensity mapping functions, including generating levels LUTs.
- **`thresholding.cpp/h`**: Implements thresholding algorithms, including adaptive thresholding using integral images.
- **`imagehistogram.cpp/h`**: Implements the `ImageHistogram` class for computing and managing image histograms.

#### `src/ibp/misc/`

- **`CMakeLists.txt`**: Configures the build for the `ibp.misc` shared library.
- **`configurationmanager.cpp/h`**: Implements the `ConfigurationManager` class for managing application settings.
- **`nativeeventfilter.cpp/h`**: Implements a native event filter for handling platform-specific events.
- **`interpolator1D.h`**: Defines the base class for 1D interpolators.
- **`basesplineinterpolator1D.h`**: Defines the base class for spline-based interpolators.
- **`cubicsplineinterpolator1D.cpp/h`**: Implements a cubic spline interpolator.
- **`linearsplineinterpolator1D.cpp/h`**: Implements a linear spline interpolator.
- **`nearestneighborsplineinterpolator1D.cpp/h`**: Implements a nearest neighbor spline interpolator.
- **`probabilitymassfunction.cpp/h`**: Implements the `ProbabilityMassFunction` class for managing probability mass functions.
- **`util.h`**: Provides utility functions for the `misc` module.

#### `src/ibp/plugins/`

- **`CMakeLists.txt`**: Configures the build for plugins, including an option to build plugins.
- **`imagefilterpluginloader.cpp/h`**: Implements the `ImageFilterPluginLoader` class, which is responsible for loading image filter plugins, managing their information, and instantiating filter objects.

#### `src/ibp/widgets/`

- **`CMakeLists.txt`**: Configures the build for the `ibp.widgets` shared library.
- **`resinit.cpp`**: Initializes the Qt resource system for the widgets.
- **`widgets.qrc`**: A Qt resource file that lists resources used by the widgets.
- **`style.cpp`**: Sets the application style, including the dark palette.
- **`colorbutton.cpp/h`**: Implements a custom color button widget.
- **`colorslider.cpp/h`**: Implements a custom color slider widget.
- **`histogramviewer.cpp/h`**: Implements a widget for viewing image histograms.
- **`imageviewer.cpp/h`**: Implements an image viewer widget with zoom and pan capabilities.
- **`toolbuttonex.cpp/h`**: Implements an extended tool button widget.
- **`genericdialog.cpp/h/ui`**: Implements a generic dialog widget.
- **`colorpicker.cpp/h/ui`**: Implements a color picker widget.
- **`colorbox.cpp/h`**: Implements a color box widget.
- **`imagebutton.cpp/h`**: Implements an image button widget.
- **`colorcompositionmodecombobox.cpp/h`**: Implements a combo box for selecting color composition modes.
- **`widgetlist.cpp/h`**: Implements a widget list for managing a list of widgets.
- **`affinetransformationlist.cpp/h/ui`**: Implements a list for managing affine transformations.
- **`rotationgauge.cpp/h`**: Implements a rotation gauge widget.
- **`inputlevelsslider.cpp/h`**: Implements a slider for input levels.
- **`inputlevelswidget.cpp/h/ui`**: Implements a widget for managing input levels.
- **`outputlevelsslider.cpp/h`**: Implements a slider for output levels.
- **`outputlevelswidget.cpp/h/ui`**: Implements a widget for managing output levels.
- **`anchorpositionbox.cpp/h`**: Implements a widget for selecting anchor positions.
- **`curves.cpp/h`**: Implements a curves widget for visualizing and manipulating interpolation curves.
- **`filedialog.cpp/h`**: Provides functions for opening and saving files using platform-specific dialogs.
- **`levelscurvespaintdelegate.cpp/h`**: Implements a custom paint delegate for levels curves.
- **`lumakeyingcurvespaintdelegate.cpp/h`**: Implements a custom paint delegate for luma keying curves.
- **`hslkeyingcurvespaintdelegate.cpp/h`**: Implements a custom paint delegate for HSL keying curves.
- **`hslcolorreplacementcurvespaintdelegate.cpp/h`**: Implements a custom paint delegate for HSL color replacement curves.

## Plugins (`src/plugins/`)

The `plugins` directory contains subdirectories for each image filter plugin. Each plugin is built as a separate shared library.

### Plugin Structure

Each plugin follows a similar structure:

- **`CMakeLists.txt`**: Configures the build for the plugin.
- **`filter.h`**: Defines the `Filter` class, inheriting from `ImageFilter`.
- **`filter.cpp`**: Implements the `Filter` class, including the `process` method for applying the filter to an image.
- **`filterwidget.h`**: Defines the `FilterWidget` class, which provides the user interface for the filter.
- **`filterwidget.cpp`**: Implements the `FilterWidget` class.
- **`filterwidget.ui`**: The Qt Designer UI file for the filter widget.
- **`main.cpp`**: Contains the plugin entry point and functions for retrieving plugin information and creating filter instances.

### Example Plugin: `imagefilter_adaptivemanifoldfilter`

This plugin implements an adaptive manifold filter.

- **`filter.h/cpp`**: Defines and implements the `Filter` class, which applies the adaptive manifold filter using OpenCV.
- **`filterwidget.h/cpp/ui`**: Defines and implements the `FilterWidget` class, providing UI elements for configuring the filter parameters.
- **`main.cpp`**: Contains the plugin entry point and the `getIBPPluginInfo` function, which returns information about the plugin.

## Building on macOS

### Prerequisites

```bash
#!/bin/bash

# Script to install prerequisites and build the Image Batch Processor (IBP) project on macOS.

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Install Homebrew (if not already installed) ---
if ! command -v brew &> /dev/null
then
    echo "Homebrew not found. Installing..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
    echo "Homebrew already installed."
fi

# --- Install Required Packages ---
echo "Installing required packages using Homebrew..."
brew update
brew install cmake qt@5 lcms2 freeimage opencv eigen

# Link qt@5 if necessary (it's keg-only)
if ! ls /usr/local/opt/qt@5 > /dev/null; then
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
    -DCMAKE_PREFIX_PATH="/usr/local/opt/qt@5;/usr/local/opt/lcms2;/usr/local/Cellar/freeimage/3.18.0"
    -DCMAKE_SHARED_LINKER_FLAGS="-L/usr/local/Cellar/freeimage/3.18.0/lib -L/usr/local/opt/lcms2/lib -llcms2"
    -DIBP_BUILD_PLUGINS=ON
)

# --- Build the Project ---
echo "Configuring project with CMake..."
cmake . "${CMAKE_OPTIONS[@]}"

echo "Building project with Make..."
make -j$(sysctl -n hw.ncpu)

echo "Build completed successfully!"

# --- Cleanup (Optional) ---
# brew cleanup

echo "Installation and build process finished."
```

### Pre-building

```bash
LDFLAGS="-L/usr/local/opt/qt@5/lib" CPPFLAGS="-I/usr/local/opt/qt@5/include" PKG_CONFIG_PATH="/usr/local/opt/qt@5/lib/pkgconfig" cmake . \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_EXTENSIONS=OFF \
    -DCMAKE_CXX_FLAGS="-std=c++17 -DQT_USE_QSTRINGBUILDER -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x060000 -DQ_COMPILER_STDCXX=201703L" \
    -DCMAKE_EXE_LINKER_FLAGS="-L/usr/local/opt/lcms2/lib -L/usr/local/Cellar/freeimage/3.18.0/lib" \
    -DCMAKE_INCLUDE_PATH="/usr/local/include;/usr/local/Cellar/freeimage/3.18.0/include" \
    -DCMAKE_LIBRARY_PATH="/usr/local/lib;/usr/local/Cellar/freeimage/3.18.0/lib" \
    -DCMAKE_PREFIX_PATH="/usr/local/opt/qt@5;/usr/local/opt/lcms2;/usr/local/Cellar/freeimage/3.18.0" \
    -DCMAKE_SHARED_LINKER_FLAGS="-L/usr/local/Cellar/freeimage/3.18.0/lib -L/usr/local/opt/lcms2/lib -llcms2"
```

### Building

```bash
make -j$(sysctl -n hw.ncpu)
```

### Building details

**Prerequisites for Building on macOS**

This project uses several external libraries and tools, which you'll need to install before attempting to build. Based on the `BUILDING.md` and the CMake command, here's a breakdown of the prerequisites and how to install them:

**1. Package Manager: Homebrew**

Homebrew is a popular package manager for macOS that simplifies the installation of software. It's highly recommended for managing development tools and libraries.

- **Installation:**
If you don't have Homebrew installed, open your terminal and run the following command:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

This command downloads and executes the Homebrew installation script. Follow the on-screen instructions to complete the installation. You may need to add it to your `$PATH` if it's not added automatically. Usually, this will be indicated in the output after running the command.

- **Verification:**
After installation, verify it's working correctly by running:

```bash
brew doctor
```

This command checks your Homebrew installation and reports any potential issues.

**2. Build System: CMake**

CMake is a cross-platform build system generator. It doesn't build the project directly but generates build files for other build systems (like Make in this case).

- **Installation with Homebrew:**

```bash
brew install cmake
```

- **Verification:**

```bash
cmake --version
```

**3. Libraries**

- **Qt5:** This project requires Qt 5 for its user interface and core functionalities.
- **Installation:**
```bash
brew install qt@5
brew link --force qt@5
```
-  Homebrew has changed how it handles Qt versions, and `qt@5` is now "keg-only", which means it's not automatically linked into `/usr/local`. You might need to force-link it as shown above, or adjust your `PATH` and other relevant environment variables. The `brew info qt@5` command can provide further guidance.
- **Verification:**
```bash
qmake -v
```
(Ensure that it's picking up the Qt 5 version, not another Qt version if you have others installed)

- **lcms2 (Little CMS Color Management Library):** This library is used for color management.
- **Installation:**
```bash
brew install lcms2
```
- **Verification:**
```bash
pkg-config --modversion lcms2
```

- **FreeImage:** This library is used for image loading and saving.
- **Installation:**
```bash
brew install freeimage
```
- **Verification:**
```bash
pkg-config --modversion freeimage
```

- **OpenCV:** This library is used for image processing.
- **Installation:**
```bash
brew install opencv
```
- **Verification:**
```bash
pkg-config --modversion opencv4
```

- **Eigen3:** This library is used for linear algebra operations, especially in the `misc` directory of your `src`.
- **Installation:**
```bash
brew install eigen
```
- **Verification:**
There isn't a direct `pkg-config` check for Eigen, but you can check if the headers are present, e.g.:
```bash
ls /usr/local/include/eigen3/Eigen
```

- **ITK (Insight Segmentation and Registration Toolkit):** Used in the `imagefilter_itkn4iihc` plugin.
- **Installation:**
```bash
brew install itk
```
- **Verification:**
```bash
pkg-config --modversion itk
```

- **Tina Vision Library:**
- You'll need to install this library manually, as it's not typically available through Homebrew. You need to get it from the [Tina Vision website](http://www.tina-vision.net/) and follow their installation instructions.
- Make sure to install the correct version that's compatible with your other dependencies.
- After installation, make sure that the paths you provide to CMake for `TINA_LIBS_INCLUDE_DIR` and `TINA_LIBS_LIB_DIR` point to the correct locations of the Tina headers and libraries.

**4. Other Dependencies (Potentially):**

- The project's `CMakeLists.txt` files might reveal additional dependencies that are not explicitly mentioned in the `BUILDING.md`. You might encounter errors during the CMake configuration step if any of these are missing.

**5. Development Environment (Optional but Recommended):**
 - **Xcode**: Install Xcode from the Mac App Store. This will provide you with the necessary compilers (Clang), build tools, and SDKs for macOS. After installation, open Xcode at least once to accept the license agreement. You may also need to install the command-line tools with `xcode-select --install`

**Putting it All Together**

Once you have these prerequisites installed, you should be able to follow the instructions in your `BUILDING.md` to build the project. The CMake command you provided sets a lot of environment variables and flags to ensure that CMake finds the correct libraries and uses the desired C++ standard.

**Troubleshooting**

- If you encounter errors during the `cmake` configuration step, carefully examine the error messages. They often indicate missing dependencies or incorrect paths.
- Use `brew doctor` and `brew info <package>` to troubleshoot issues with your Homebrew installation or specific packages.
- Make sure that the environment variables like `LDFLAGS`, `CPPFLAGS`, and `PKG_CONFIG_PATH` are correctly set in your shell before running CMake.
- Verify that you are using a compatible compiler. In your CMake command, you specify `-DCMAKE_CXX_STANDARD=17`, which means you need a compiler that supports C++17. Xcode's Clang compiler should be able to handle this.
- If you have multiple versions of a library installed, the paths you provide to CMake must point to the correct versions.

By following these steps, you should be well-equipped to build the Image Batch Processor project on macOS. Remember to consult the official documentation of Homebrew, CMake, and the individual libraries if you encounter any issues.

