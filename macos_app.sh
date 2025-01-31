#!/usr/bin/env bash

# Exit on error
set -e

# Configuration
APP_NAME="ImageBatchProcessor"
APP_VERSION="$(cat VERSION.TXT)"
BUILD_DIR="build/build"
BUNDLE_NAME="${APP_NAME}.app"
DIST_DIR="dist"

# External library paths
FREEIMAGE_PATH="$(brew --prefix freeimage)"
LCMS2_PATH="$(brew --prefix lcms2)"
OPENBLAS_PATH="$(brew --prefix openblas)"
OPENCV_PATH="$(brew --prefix opencv@4)"
OPENJPEG_PATH="$(brew --prefix openjpeg)"
QT_PATH="$(brew --prefix qt@5)"
WEBP_PATH="$(brew --prefix webp)"

# Find OpenCV libraries dynamically
mapfile -t opencv_libs < <(find "${OPENCV_PATH}/lib" -type f -iname "libopencv_*.dylib")

# Find additional libraries dynamically
additional_libs=(
    $(find "${FREEIMAGE_PATH}/lib" -type f -iname "libfreeimage*.dylib")
    $(find "${LCMS2_PATH}/lib" -type f -iname "liblcms2*.dylib")
    $(find "${OPENBLAS_PATH}/lib" -type f -iname "libopenblas*.dylib")
    $(find "${OPENJPEG_PATH}/lib" -type f -iname "libopenjp2*.dylib")
    $(find "${WEBP_PATH}/lib" -type f -iname "libwebp*.dylib")
    $(find "${WEBP_PATH}/lib" -type f -iname "libsharpyuv*.dylib")
)

# Clean up any .DS_Store files
find . -name ".DS_Store" -delete

# Create dist directory and clean up any existing bundle
echo "Creating distribution directory..."
mkdir -p "${DIST_DIR}"
rm -rf "${DIST_DIR}/${BUNDLE_NAME}"

# Create basic bundle structure
echo "Creating bundle structure..."
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/"{MacOS,Resources,Frameworks,PlugIns}

# Copy the main executable
# First try to find versioned executable
versioned_exec=$(find "${BUILD_DIR}" -maxdepth 1 -type f -name "ibp-*" | sort -V | tail -n1)

if [ -n "$versioned_exec" ]; then
    # Found versioned executable
    cp "$versioned_exec" "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
    chmod +x "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
elif [ -L "${BUILD_DIR}/ibp" ] && [ -f "${BUILD_DIR}/$(readlink ${BUILD_DIR}/ibp)" ]; then
    # Found symbolic link to executable
    actual_exec="${BUILD_DIR}/$(readlink ${BUILD_DIR}/ibp)"
    cp "$actual_exec" "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
    chmod +x "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
elif [ -f "${BUILD_DIR}/ibp" ]; then
    # Found plain executable
    cp "${BUILD_DIR}/ibp" "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
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
    cp "$lib" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/$(basename "$lib")"
done

# Run macdeployqt first
echo "Running macdeployqt..."
"${QT_PATH}/bin/macdeployqt" "${DIST_DIR}/${BUNDLE_NAME}" \
    -verbose=2 \
    -always-overwrite

# Function to copy and fix additional library
copy_additional_lib() {
    local src="$1"
    local dest="${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/$(basename "$src")"

    echo "Copying $src to $dest"

    # Copy the library
    cp -R "$src" "$dest"
    chmod u+w "$dest"

    # Fix the library ID
    install_name_tool -id "@rpath/$(basename "$src")" "$dest"

    # Fix dependencies
    otool -L "$dest" | tail -n +2 | while read -r line; do
        local dep_path=$(echo "$line" | awk '{print $1}')
        if [[ "$dep_path" != /System/* && "$dep_path" != /usr/lib/* ]]; then
            local dep_name=$(basename "$dep_path")
            install_name_tool -change "$dep_path" "@rpath/$dep_name" "$dest"
        fi
    done
}

# Copy OpenCV libraries
echo "Copying OpenCV libraries..."

for lib in "${opencv_libs[@]}"; do
    if [ -f "$lib" ]; then
        copy_additional_lib "$lib"
        # Create symlinks for version-specific .dylib files
        # Extract version number and create appropriate symlink
        lib_basename=$(basename "$lib")
        if [[ "$lib_basename" =~ libopencv_.*\.([0-9]+\.[0-9]+)\.[0-9]+\.dylib ]]; then
            version="${BASH_REMATCH[1]}"
            base_name=$(echo "$lib_basename" | sed "s/\.$version\.[0-9]\+\.dylib/.$version.dylib/")
            (cd "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" && ln -sf "$lib_basename" "$base_name")
        fi
    else
        echo "Error: Could not find OpenCV library ${lib}"
        exit 1
    fi
done

# Function to set correct permissions
set_permissions() {
    local target="$1"
    if [ -d "$target" ]; then
        chmod -R u+w "$target"
    elif [ -f "$target" ]; then
        chmod u+w "$target"
    fi
}

# Copy additional dependencies
echo "Copying additional dependencies..."

for lib_path in "${additional_libs[@]}"; do
    if [ -f "$lib_path" ]; then
        copy_additional_lib "$lib_path"
        # Create symlinks for OpenBLAS
        if [[ "$(basename "$lib_path")" =~ libopenblasp-r([0-9]+\.[0-9]+\.[0-9]+)\.dylib ]]; then
            openblas_lib="$(basename "$lib_path")"
            (cd "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" && ln -sf "$openblas_lib" "libopenblas.0.dylib")
            (cd "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" && ln -sf "libopenblas.0.dylib" "libopenblas.dylib")
        fi
    else
        echo "Error: Could not find library $lib_path"
        exit 1
    fi
done

# Set permissions on the entire bundle
echo "Setting permissions..."
set_permissions "${DIST_DIR}/${BUNDLE_NAME}"

# Copy custom plugins
echo "Copying image filter plugins..."
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters"
for plugin in "${BUILD_DIR}/plugins/"libibp.imagefilter.*.dylib; do
    cp "$plugin" "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters/"
    chmod 755 "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters/$(basename "$plugin")"
    # Fix plugin paths
    plugin_dest="${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters/$(basename "$plugin")"
    install_name_tool -id "@rpath/$(basename "$plugin")" "$plugin_dest"
    # Add rpath to find frameworks
    install_name_tool -add_rpath "@executable_path/../Frameworks" "$plugin_dest"
done

# Copy settings
echo "Copying settings..."
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/settings"
if [ -n "$(ls -A "${BUILD_DIR}/settings/" 2>/dev/null)" ]; then
    cp "${BUILD_DIR}/settings/"* "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/settings/"
fi

# Copy application icon
echo "Copying application icon..."
cp "doc/icon/ibp.icns" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/icon.icns"

# Create qt.conf
cat >"${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/qt.conf" <<EOF
[Paths]
Plugins = PlugIns
Libraries = Frameworks
EOF

# Update Info.plist
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
</dict>
</plist>
EOF

echo "Created: ${DIST_DIR}/${BUNDLE_NAME}.app"
