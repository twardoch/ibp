#!/usr/bin/env bash

# Exit on error
set -e

# Configuration
APP_NAME="ImageBatchProcessor"
APP_VERSION="2.0.1"
BUNDLE_NAME="${APP_NAME}.app"
DIST_DIR="dist"
BUILD_DIR="build/build"

# External library paths
QT_PATH="$(brew --prefix qt@5)"
OPENCV_PATH="$(brew --prefix opencv)"
FREEIMAGE_PATH="$(brew --prefix freeimage)"
LCMS2_PATH="$(brew --prefix lcms2)"
OPENBLAS_PATH="$(brew --prefix openblas)"

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
if [ -f "${BUILD_DIR}/ibp-0.1.0" ]; then
    cp "${BUILD_DIR}/ibp-0.1.0" "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
    chmod +x "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
elif [ -L "${BUILD_DIR}/ibp" ] && [ -f "${BUILD_DIR}/$(readlink ${BUILD_DIR}/ibp)" ]; then
    actual_exec="${BUILD_DIR}/$(readlink ${BUILD_DIR}/ibp)"
    cp "$actual_exec" "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
    chmod +x "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"
elif [ -f "${BUILD_DIR}/ibp" ]; then
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
opencv_libs=(
    "libopencv_core.4.11.0.dylib"
    "libopencv_imgproc.4.11.0.dylib"
    "libopencv_imgcodecs.4.11.0.dylib"
    "libopencv_photo.4.11.0.dylib"
    "libopencv_ximgproc.4.11.0.dylib"
    "libopencv_xphoto.4.11.0.dylib"
)

for lib in "${opencv_libs[@]}"; do
    lib_path="${OPENCV_PATH}/lib/${lib}"
    if [ -f "$lib_path" ]; then
        copy_additional_lib "$lib_path"
        # Create symlinks for .411.dylib versions
        base_name=$(echo "$lib" | sed 's/\.4\.11\.0\.dylib/.411.dylib/')
        (cd "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" && ln -sf "$lib" "$base_name")
    else
        echo "Error: Could not find OpenCV library ${lib}"
        exit 1
    fi
done

# Copy additional dependencies
echo "Copying additional dependencies..."
additional_libs=(
    "${OPENBLAS_PATH}/lib/libopenblasp-r0.3.29.dylib"
    "${LCMS2_PATH}/lib/liblcms2.2.dylib"
    "${FREEIMAGE_PATH}/lib/libfreeimage.3.18.0.dylib"
    "${FREEIMAGE_PATH}/lib/libfreeimageplus.3.18.0.dylib"
)

for lib_path in "${additional_libs[@]}"; do
    if [ -f "$lib_path" ]; then
        copy_additional_lib "$lib_path"
        # Create symlinks for OpenBLAS
        if [[ "$lib_path" == *"libopenblasp-r0.3.29.dylib" ]]; then
            (cd "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" && ln -sf "$(basename "$lib_path")" "libopenblas.0.dylib")
            (cd "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" && ln -sf "libopenblas.0.dylib" "libopenblas.dylib")
        fi
    else
        echo "Error: Could not find library $lib_path"
        exit 1
    fi
done

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
cp "${BUILD_DIR}/settings/"* "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/settings/"

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

# Create DMG
echo "Creating DMG..."
"${QT_PATH}/bin/macdeployqt" "${DIST_DIR}/${BUNDLE_NAME}" -dmg

echo "Packaging complete!"
