#!/usr/bin/env bash

# Exit on error
set -e

# Configuration
APP_NAME="ImageBatchProcessor"
APP_VERSION="2.0.1"
BUNDLE_NAME="${APP_NAME}.app"
DMG_NAME="${APP_NAME}-${APP_VERSION}.dmg"
DIST_DIR="dist"
BUILD_DIR="build/build"

# External library paths
QT_PATH="/usr/local/opt/qt@5"
OPENCV_PATH="/usr/local/opt/opencv@4"
FREEIMAGE_PATH="$(brew --prefix freeimage)"
LCMS2_PATH="$(brew --prefix lcms2)"
OPENBLAS_PATH="$(brew --prefix openblas)"

# Detect OpenCV version
OPENCV_VERSION=$(pkg-config --modversion opencv4 || echo "4.8")
OPENCV_MAJOR_MINOR=$(echo $OPENCV_VERSION | cut -d. -f1,2)

# Array to track which libraries have been copied
COPIED_LIBS=()

# Function to check if a library has been copied
has_been_copied() {
    local lib_path="$1"
    local lib_name=$(basename "$lib_path")
    for copied_lib in "${COPIED_LIBS[@]}"; do
        if [[ "$copied_lib" == "$lib_name" ]]; then
            return 0
        fi
    done
    return 1
}

# Function to mark a library as copied
mark_as_copied() {
    local lib_path="$1"
    local lib_name=$(basename "$lib_path")
    COPIED_LIBS+=("$lib_name")
}

# Function to ensure directory permissions
ensure_dir_permissions() {
    local dir="$1"
    if [ ! -w "$dir" ]; then
        sudo chmod -R u+w "$dir"
    fi
}

# Function to copy files safely
safe_copy() {
    local src="$1"
    local dest="$2"
    local dest_dir="$(dirname "$dest")"

    echo "safe_copy: Copying $src to $dest"
    echo "safe_copy: Destination directory is $dest_dir"

    # Create destination directory if it doesn't exist
    mkdir -p "$dest_dir"

    # Copy the file
    cp -R "$src" "$dest"

    if [ -f "$dest" ]; then
        # Make sure the copied file is writable
        chmod u+w "$dest"
        echo "safe_copy: File copied successfully"
    else
        echo "safe_copy: Failed to copy file"
        return 1
    fi
}

# Function to add RPATH if it doesn't already exist
add_rpath_if_missing() {
    local binary="$1"
    local rpath="$2"

    # Check if RPATH already exists
    if ! otool -l "$binary" | grep -A2 LC_RPATH | grep -q "path $rpath"; then
        install_name_tool -add_rpath "$rpath" "$binary" 2>/dev/null || true
    fi
}

# Function to copy library and its dependencies
copy_library() {
    local src="$1"
    local dest_dir="$2"
    local lib_name=$(basename "$src")

    ensure_dir_permissions "$dest_dir"

    # Copy the main library file
    safe_copy "$src" "$dest_dir/$lib_name"

    # Create necessary symlinks
    if [[ "$lib_name" =~ ([^.]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)\.dylib ]]; then
        local name="${BASH_REMATCH[1]}"
        local major="${BASH_REMATCH[2]}"
        local minor="${BASH_REMATCH[3]}"

        (cd "$dest_dir" && ln -sf "$lib_name" "${name}.${major}.${minor}.dylib")
        (cd "$dest_dir" && ln -sf "$lib_name" "${name}.${major}.dylib")
        (cd "$dest_dir" && ln -sf "$lib_name" "${name}.dylib")
    fi

    # Fix the library ID and dependencies
    install_name_tool -id "@rpath/$lib_name" "$dest_dir/$lib_name"
}

# Function to copy Qt framework
copy_qt_framework() {
    local framework_path="$1"
    local framework_name="$2"
    local framework_dest="${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/${framework_name}.framework"

    echo "Copying Qt framework: ${framework_name}"

    # Create framework directories
    mkdir -p "$framework_dest/Versions/5/Resources"

    # Copy framework binary
    cp "${framework_path}/Versions/5/${framework_name}" "${framework_dest}/Versions/5/"
    chmod u+w "${framework_dest}/Versions/5/${framework_name}"

    # Copy resources if they exist
    if [ -d "${framework_path}/Versions/5/Resources" ]; then
        cp -R "${framework_path}/Versions/5/Resources/." "${framework_dest}/Versions/5/Resources/"
    fi

    # Create symlinks
    (cd "${framework_dest}" && ln -sfh "Versions/Current/${framework_name}" "${framework_name}")
    (cd "${framework_dest}" && ln -sfh "Versions/Current/Resources" "Resources")
    (cd "${framework_dest}/Versions" && ln -sfh "5" "Current")

    # Fix framework ID and dependencies
    install_name_tool -id "@rpath/${framework_name}.framework/Versions/5/${framework_name}" \
        "${framework_dest}/Versions/5/${framework_name}"

    # Fix dependencies between Qt frameworks
    for dep in "${QT_FRAMEWORKS[@]}"; do
        install_name_tool -change \
            "${QT_PATH}/lib/${dep}.framework/Versions/5/${dep}" \
            "@rpath/${dep}.framework/Versions/5/${dep}" \
            "${framework_dest}/Versions/5/${framework_name}" || true
    done
}

# Function to fix library paths for a single binary
fix_library_paths() {
    local binary="$1"
    local current_path
    local new_path

    echo "Fixing paths for: $binary"

    # Get all dependencies
    otool -L "$binary" | tail -n +2 | while read -r line; do
        current_path=$(echo "$line" | awk '{print $1}')

        # Skip system libraries
        if [[ "$current_path" == /usr/lib/* || "$current_path" == /System/* ]]; then
            continue
        fi

        # Extract the library name
        local lib_name=$(basename "$current_path")

        # Handle Qt frameworks specially
        if [[ "$current_path" == *".framework"* ]]; then
            framework_name=$(echo "$current_path" | awk -F'.framework' '{print $1}' | awk -F'/' '{print $NF}')
            new_path="@rpath/${framework_name}.framework/Versions/5/${framework_name}"
        else
            new_path="@rpath/${lib_name}"
        fi

        echo "  Changing $current_path -> $new_path"
        install_name_tool -change "$current_path" "$new_path" "$binary" || true
    done

    # Add rpath for bundled frameworks if not already present
    add_rpath_if_missing "$binary" "@executable_path/../Frameworks"
}

# Function to copy Qt plugins
copy_qt_plugins() {
    local plugins_dest="${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns"

    # Create plugin directories
    mkdir -p "${plugins_dest}/platforms"
    mkdir -p "${plugins_dest}/imageformats"
    mkdir -p "${plugins_dest}/styles"

    # Add rpath for bundled frameworks
    add_rpath_if_missing "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}" "@executable_path/../Frameworks"

    # Define plugin paths
    local qt_plugins=(
        "platforms/libqcocoa.dylib"
        "imageformats/libqgif.dylib"
        "imageformats/libqico.dylib"
        "imageformats/libqjpeg.dylib"
        "imageformats/libqsvg.dylib"
        "imageformats/libqtiff.dylib"
        "styles/libqmacstyle.dylib"
    )

    # Copy each plugin
    for plugin in "${qt_plugins[@]}"; do
        local plugin_src="${QT_PATH}/plugins/${plugin}"
        local plugin_dest="${plugins_dest}/${plugin}"
        local plugin_dir=$(dirname "${plugins_dest}/${plugin}")

        if [ -f "$plugin_src" ]; then
            echo "Copying Qt plugin: ${plugin}"
            mkdir -p "$plugin_dir"
            safe_copy "$plugin_src" "$plugin_dest"
            chmod 755 "$plugin_dest"

            # Fix plugin paths
            fix_qt_plugin "$plugin_dest"

            # Remove old rpaths
            install_name_tool -delete_rpath "@loader_path/.." "$plugin_dest" 2>/dev/null || true
            install_name_tool -delete_rpath "@loader_path/../.." "$plugin_dest" 2>/dev/null || true
            install_name_tool -delete_rpath "@loader_path/../../.." "$plugin_dest" 2>/dev/null || true
            install_name_tool -delete_rpath "/usr/local/opt/qt@5/lib" "$plugin_dest" 2>/dev/null || true
        else
            echo "Warning: Qt plugin not found: ${plugin}"
        fi
    done
}

# Function to copy and fix additional library
copy_additional_lib() {
    local lib_path="$1"
    local lib_name=$(basename "$lib_path")

    # Check if library was already copied
    if has_been_copied "$lib_path"; then
        echo "Library $lib_name was already copied, skipping..."
        return 0
    fi

    local dest_path="${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/${lib_name}"

    echo "Copying additional library: $lib_path"

    # Get the actual file that the symlink points to
    if [ -L "$lib_path" ]; then
        local real_lib=$(readlink -f "$lib_path")
        if [ -f "$real_lib" ]; then
            echo "Found actual library at: $real_lib"
            safe_copy "$real_lib" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/$(basename "$real_lib")"
            mark_as_copied "$real_lib"

            # Create symlink if needed
            if [ "$(basename "$real_lib")" != "$lib_name" ]; then
                (cd "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" && ln -sf "$(basename "$real_lib")" "$lib_name")
            fi

            # Fix the library ID
            install_name_tool -id "@executable_path/../Frameworks/$(basename "$real_lib")" \
                "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/$(basename "$real_lib")"
        else
            echo "Error: Could not find actual library for $lib_path"
            return 1
        fi
    else
        safe_copy "$lib_path" "$dest_path"
        install_name_tool -id "@executable_path/../Frameworks/$lib_name" "$dest_path"
        mark_as_copied "$lib_path"
    fi
}

# Function to check Qt dependencies
check_qt_dependencies() {
    local binary="$1"
    local qt_lib_path="${QT_PATH}/lib"

    # Check if the binary links to Qt libraries outside the bundle
    otool -L "$binary" | grep -E "${qt_lib_path}|@rpath" | while read -r line; do
        local lib_path=$(echo "$line" | awk '{print $1}')

        # Check if it's a Qt library path that needs fixing
        if [[ "$lib_path" == *"${qt_lib_path}"* ]]; then
            local framework_name=$(basename "${lib_path%%.framework*}")
            echo "Warning: $binary links to external Qt framework: $lib_path"
            echo "  Should be: @rpath/${framework_name}.framework/Versions/5/${framework_name}"
        fi
    done
}

# Function to fix Qt plugin paths
fix_qt_plugin() {
    local plugin_path="$1"
    local plugin_name=$(basename "$plugin_path")

    echo "Fixing Qt plugin: ${plugin_name}"

    # Fix plugin ID
    install_name_tool -id "@rpath/PlugIns/platforms/${plugin_name}" "$plugin_path"

    # Fix Qt framework dependencies
    for framework in "${QT_FRAMEWORKS[@]}"; do
        # Fix references to Cellar path
        install_name_tool -change \
            "/usr/local/Cellar/qt@5/5.15.16/lib/${framework}.framework/Versions/5/${framework}" \
            "@rpath/${framework}.framework/Versions/5/${framework}" \
            "$plugin_path" || true

        # Fix references to opt path
        install_name_tool -change \
            "${QT_PATH}/lib/${framework}.framework/Versions/5/${framework}" \
            "@rpath/${framework}.framework/Versions/5/${framework}" \
            "$plugin_path" || true
    done

    # Add rpath for frameworks
    add_rpath_if_missing "$plugin_path" "@executable_path/../Frameworks"

    echo "Finished fixing Qt plugin: ${plugin_name}"
}

# Ensure we're in the right directory
cd "$(dirname "$0")"

# Clean up any .DS_Store files in the source directory
find . -name ".DS_Store" -delete

# Create dist directory and clean up any existing bundle
echo "Creating distribution directory..."
mkdir -p "${DIST_DIR}"
rm -rf "${DIST_DIR}/${BUNDLE_NAME}"
rm -f "${DIST_DIR}/${DMG_NAME}"

# Create the basic bundle structure
echo "Creating bundle structure..."
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/"{MacOS,Resources,Frameworks,PlugIns}

# Ensure all directories are writable
echo "Setting directory permissions..."
for dir in \
    "${DIST_DIR}" \
    "${DIST_DIR}/${BUNDLE_NAME}" \
    "${DIST_DIR}/${BUNDLE_NAME}/Contents" \
    "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS" \
    "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources" \
    "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" \
    "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns"; do
    ensure_dir_permissions "$dir"
done

# Copy the main executable
if [ -f "${BUILD_DIR}/ibp-0.1.0" ]; then
    safe_copy "${BUILD_DIR}/ibp-0.1.0" "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
    chmod +x "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
    # Remove build directory rpath and add framework rpath
    install_name_tool -delete_rpath "/Users/adam/Developer/vcs/github.twardoch/pub/ibp/build/build" "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
    # Add explicit rpath commands for main executable
    add_rpath_if_missing "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}" "@executable_path/../Frameworks"
elif [ -L "${BUILD_DIR}/ibp" ] && [ -f "${BUILD_DIR}/$(readlink ${BUILD_DIR}/ibp)" ]; then
    actual_exec="${BUILD_DIR}/$(readlink ${BUILD_DIR}/ibp)"
    safe_copy "$actual_exec" "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
    chmod +x "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
elif [ -f "${BUILD_DIR}/ibp" ]; then
    safe_copy "${BUILD_DIR}/ibp" "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
    chmod +x "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
else
    echo "Error: Could not find executable in ${BUILD_DIR}"
    exit 1
fi

# Copy all IBP libraries
for lib in "${BUILD_DIR}"/libibp.*.dylib; do
    if [[ -L "$lib" ]]; then
        continue
    fi
    safe_copy "$lib" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/$(basename "$lib")"
done

# Copy plugins
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters"
for plugin in "${BUILD_DIR}/plugins/"libibp.imagefilter.*.dylib; do
    safe_copy "$plugin" "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters/$(basename "$plugin")"
done

# Copy settings to the correct location (only in Resources)
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/settings"
for setting in "${BUILD_DIR}/settings/"*; do
    if [ -f "$setting" ]; then
        safe_copy "$setting" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/settings/$(basename "$setting")"
    fi
done

# Remove incorrect settings directory in MacOS if it exists
rm -rf "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/settings"

# Copy Qt frameworks
echo "Copying Qt frameworks..."
QT_FRAMEWORKS=("QtCore" "QtGui" "QtWidgets" "QtDBus" "QtPrintSupport")

for framework in "${QT_FRAMEWORKS[@]}"; do
    framework_path="${QT_PATH}/lib/${framework}.framework"
    if [ -d "$framework_path" ]; then
        copy_qt_framework "$framework_path" "$framework"
    else
        echo "Error: Qt framework not found: ${framework_path}"
        exit 1
    fi
done

# Additional Qt dependencies check
echo "Checking additional Qt dependencies..."
for framework in "${QT_FRAMEWORKS[@]}"; do
    framework_binary="${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/${framework}.framework/Versions/5/${framework}"
    if [ -f "$framework_binary" ]; then
        echo "Checking dependencies for ${framework}..."
        otool -L "$framework_binary" | grep -E "/usr/local/(opt|Cellar)" || true
    fi
done

# Copy Qt plugins
echo "Copying Qt plugins..."
copy_qt_plugins

# Copy OpenCV libraries with better error handling
echo "Copying OpenCV libraries..."
OPENCV_CELLAR="/usr/local/Cellar/opencv/4.11.0"
opencv_libs=(
    "libopencv_core.411.dylib"
    "libopencv_imgproc.411.dylib"
    "libopencv_imgcodecs.411.dylib"
    "libopencv_photo.411.dylib"
    "libopencv_ximgproc.411.dylib"
    "libopencv_xphoto.411.dylib"
)

for lib in "${opencv_libs[@]}"; do
    lib_path="${OPENCV_CELLAR}/lib/${lib}"
    if [ ! -f "$lib_path" ]; then
        echo "Warning: OpenCV library not found at ${lib_path}"
        # Try alternate paths
        alternate_paths=(
            "/usr/local/lib/${lib}"
            "/usr/local/opt/opencv/lib/${lib}"
            "/usr/local/opt/opencv@4/lib/${lib}"
        )

        lib_found=0
        for alt_path in "${alternate_paths[@]}"; do
            if [ -f "$alt_path" ] || [ -L "$alt_path" ]; then
                echo "Found OpenCV library at alternate path: $alt_path"
                lib_path="$alt_path"
                lib_found=1
                break
            fi
        done

        if [ $lib_found -eq 0 ]; then
            echo "Error: Could not find OpenCV library ${lib}"
            exit 1
        fi
    fi

    # Get the actual file that the symlink points to
    actual_lib=$(readlink -f "$lib_path")
    base_lib=$(basename "$lib")
    base_name="${base_lib%.dylib}"

    echo "Copying OpenCV library from $actual_lib"

    # Copy the actual library file
    safe_copy "$actual_lib" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/$(basename "$actual_lib")"

    # Create the necessary symlinks
    (cd "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" &&
        ln -sf "$(basename "$actual_lib")" "$base_lib" &&
        ln -sf "$base_lib" "${base_name%.*}.dylib")

    # Fix the library ID and dependencies
    install_name_tool -id "@executable_path/../Frameworks/$(basename "$actual_lib")" \
        "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/$(basename "$actual_lib")"
done

# Copy additional dependencies
echo "Copying additional dependencies..."
additional_libs=(
    "/usr/local/opt/pcre2/lib/libpcre2-16.0.dylib"
    "/usr/local/opt/pcre2/lib/libpcre2-8.0.dylib"
    "/usr/local/opt/zstd/lib/libzstd.1.dylib"
    "/usr/local/opt/glib/lib/libgthread-2.0.0.dylib"
    "/usr/local/opt/glib/lib/libglib-2.0.0.dylib"
    "/usr/local/opt/gettext/lib/libintl.8.dylib"
    "/usr/local/opt/libpng/lib/libpng16.16.dylib"
    "/usr/local/opt/md4c/lib/libmd4c.0.dylib"
    "/usr/local/opt/little-cms2/lib/liblcms2.2.dylib"
    "/usr/local/opt/freetype/lib/libfreetype.6.dylib"
    "${OPENBLAS_PATH}/lib/libopenblas.0.dylib"
)

for lib_path in "${additional_libs[@]}"; do
    if [ -f "$lib_path" ] || [ -L "$lib_path" ]; then
        copy_additional_lib "$lib_path"
    else
        echo "Warning: Additional library not found: $lib_path"
        # Try alternate paths in Cellar
        cellar_path=$(echo "$lib_path" | sed 's|/usr/local/opt/|/usr/local/Cellar/|' | sed 's|/lib/|/*/lib/|')
        found_lib=$(ls $cellar_path 2>/dev/null | head -n 1)
        if [ -n "$found_lib" ]; then
            echo "Found library at alternate path: $found_lib"
            copy_additional_lib "$found_lib"
        else
            echo "Error: Could not find library $lib_path"
            exit 1
        fi
    fi
done

# Fix dependencies for glib and related libraries
echo "Fixing dependencies for additional libraries..."
frameworks_dir="${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks"
for lib in "$frameworks_dir"/lib*.dylib; do
    if [ -f "$lib" ]; then
        # Fix the library ID
        install_name_tool -id "@rpath/$(basename "$lib")" "$lib"

        # Fix dependencies
        for dep in "${additional_libs[@]}"; do
            dep_name=$(basename "$dep")
            install_name_tool -change "$dep" "@rpath/$dep_name" "$lib" || true
            install_name_tool -change "/usr/local/Cellar/$(basename "${dep%/lib/*}")/*/lib/$dep_name" "@rpath/$dep_name" "$lib" || true
        done
    fi
done

# Copy FreeImage libraries
echo "Copying FreeImage libraries..."
FREEIMAGE_LIBS=(
    "libfreeimage.3.18.0.dylib"
    "libfreeimageplus.3.18.0.dylib"
)

for lib in "${FREEIMAGE_LIBS[@]}"; do
    src_lib="${FREEIMAGE_PATH}/lib/${lib}"
    dst_lib="${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/${lib}"

    if [ -f "$src_lib" ]; then
        echo "Copying ${lib}..."
        safe_copy "$src_lib" "$dst_lib"
        chmod 755 "$dst_lib"

        # Create symlinks
        base_name="${lib%.3.18.0.dylib}"
        (cd "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" &&
            ln -sf "${lib}" "${base_name}.3.dylib" &&
            ln -sf "${base_name}.3.dylib" "${base_name}.dylib")

        # Fix the library ID and dependencies
        install_name_tool -id "@rpath/${lib}" "$dst_lib"
    else
        echo "Error: ${lib} not found at ${src_lib}"
        exit 1
    fi
done

# Fix library paths for all binaries
echo "Fixing library paths for all binaries..."
# Verify all Qt dependencies are properly bundled
echo "Verifying Qt dependencies..."
find "${DIST_DIR}/${BUNDLE_NAME}/Contents" -type f \( -name "*.dylib" -o -name "${APP_NAME}" \) | while read file; do
    check_qt_dependencies "$file"
done

# Fix library paths for all binaries
echo "Fixing library paths for all binaries..."
find "${DIST_DIR}/${BUNDLE_NAME}/Contents" -type f \( -name "*.dylib" -o -name "${APP_NAME}" \) | while read file; do
    fix_library_paths "$file"
done

# Create qt.conf
cat >"${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/qt.conf" <<EOF
[Paths]
Plugins = PlugIns
Libraries = Frameworks
EOF

# Create Info.plist
cat >"${DIST_DIR}/${BUNDLE_NAME}/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>${APP_NAME}</string>
    <key>CFBundleIconFile</key>
    <string>icon.icns</string>
    <key>CFBundleIdentifier</key>
    <string>com.twardoch.imagebatchprocessor</string>
    <key>CFBundleName</key>
    <string>ImageBatchProcessor</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>${APP_VERSION}</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.13</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <true/>
</dict>
EOF

echo "This is the bundle contents:"
tree "${DIST_DIR}/${BUNDLE_NAME}"

# Create DMG
echo "Creating DMG..."
DMG_TEMP_PATH="${DIST_DIR}/temp.dmg"

# Create temporary DMG
hdiutil create -srcfolder "${DIST_DIR}/${BUNDLE_NAME}" -volname "Image Batch Processor" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW "${DMG_TEMP_PATH}"

# Convert temporary DMG to compressed DMG
hdiutil convert "${DMG_TEMP_PATH}" -format UDZO -imagekey zlib-level=9 -o "${DIST_DIR}/${DMG_NAME}"

# Clean up
rm -f "${DMG_TEMP_PATH}"

echo "Packaging complete!"
