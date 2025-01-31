---
layout: default
title: 03 Building on macOS
---

# [∞](#building-on-macos) Building on macOS

- Run `./macos_prep.sh` to install prerequisites 
- Run `./macos_build.sh` to build the app into `./build/`
- Run `./macos_app.sh` to build an .app into `./dist/`
- Run `./macos_dmg.sh` to build a .dmg into `./dist/`


## [∞](#prerequisites) Prerequisites

```bash
##!/bin/bash

## [∞](#script-to-install-prerequisites-and-build-the-image-batch-processor-ibp-project-on-macos) Script to install prerequisites and build the Image Batch Processor (IBP) project on macOS.

## [∞](#exit-immediately-if-a-command-exits-with-a-non-zero-status) Exit immediately if a command exits with a non-zero status.
set -e

## [∞](#----install-homebrew-if-not-already-installed----) --- Install Homebrew (if not already installed) ---
if ! command -v brew &> /dev/null
then
    echo "Homebrew not found. Installing..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
    echo "Homebrew already installed."
fi

## [∞](#----install-required-packages----) --- Install Required Packages ---
echo "Installing required packages using Homebrew..."
brew update
brew install cmake qt@5 lcms2 freeimage opencv eigen

## [∞](#link-qt5-if-necessary-its-keg-only) Link qt@5 if necessary (it's keg-only)
if ! ls /usr/local/opt/qt@5 > /dev/null; then
  echo "Linking qt@5..."
  brew link --force qt@5
fi

## [∞](#----set-environment-variables-for-cmake----) --- Set Environment Variables for CMake ---
export LDFLAGS="-L/usr/local/opt/qt@5/lib"
export CPPFLAGS="-I/usr/local/opt/qt@5/include"
export PKG_CONFIG_PATH="/usr/local/opt/qt@5/lib/pkgconfig"

## [∞](#----set-cmake-options----) --- Set CMake Options ---
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

## [∞](#----build-the-project----) --- Build the Project ---
echo "Configuring project with CMake..."
cmake . "${CMAKE_OPTIONS[@]}"

echo "Building project with Make..."
make -j$(sysctl -n hw.ncpu)

echo "Build completed successfully!"

## [∞](#----cleanup-optional----) --- Cleanup (Optional) ---
## [∞](#brew-cleanup) brew cleanup

echo "Installation and build process finished."
```

## [∞](#pre-building) Pre-building

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

## [∞](#building) Building

```bash
make -j$(sysctl -n hw.ncpu)
```

## [∞](#building-details) Building details

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

