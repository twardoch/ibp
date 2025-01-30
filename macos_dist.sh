#!/usr/bin/env bash

# Exit on error
set -e

# Configuration
APP_NAME="ImageBatchProcessor"
APP_VERSION="2.0.1"
BUNDLE_NAME="${APP_NAME}.app"
DMG_NAME="${APP_NAME}-${APP_VERSION}.dmg"
DIST_DIR="dist"

# External library paths
QT_PATH="$(brew --prefix qt@5)"
OPENCV_PATH="$(brew --prefix opencv)"
FREEIMAGE_PATH="$(brew --prefix freeimage)"
LCMS2_PATH="$(brew --prefix lcms2)"

# Ensure we're in the right directory
cd "$(dirname "$0")"

# Create dist directory and clean up any existing bundle
mkdir -p "${DIST_DIR}"
rm -rf "${DIST_DIR}/${BUNDLE_NAME}"
rm -f "${DIST_DIR}/${DMG_NAME}"

# Create the basic bundle structure
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/"{MacOS,Resources,Frameworks,PlugIns}

# Copy the main executable
cp build/ibp "${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"

# Copy all IBP libraries
for lib in build/libibp.*.dylib; do
    if [[ -L "$lib" ]]; then
        # Skip symlinks
        continue
    fi
    cp "$lib" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/"
done

# Copy plugins
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters"
cp build/plugins/libibp.imagefilter.*.dylib "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters/"

# Copy settings
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/settings"
cp build/settings/* "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/settings/"

# Copy required Qt frameworks
qtlibs=(
    "QtCore"
    "QtGui"
    "QtWidgets"
)

for lib in "${qtlibs[@]}"; do
    cp -R "${QT_PATH}/lib/${lib}.framework" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/"
done

# Copy OpenCV libraries
opencv_libs=(
    "libopencv_core.4.9.dylib"
    "libopencv_imgproc.4.9.dylib"
    "libopencv_imgcodecs.4.9.dylib"
)

for lib in "${opencv_libs[@]}"; do
    cp "${OPENCV_PATH}/lib/${lib}" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/"
    cp "${OPENCV_PATH}/lib/${lib%.*.*}"* "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/"
done

# Copy FreeImage and LCMS2
cp "${FREEIMAGE_PATH}/lib/libfreeimage.dylib" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/"
cp "${LCMS2_PATH}/lib/liblcms2.dylib" "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/"

# Create Info.plist
cat >"${DIST_DIR}/${BUNDLE_NAME}/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>${APP_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>com.github.twardoch.ibp</string>
    <key>CFBundleName</key>
    <string>${APP_NAME}</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>${APP_VERSION}</string>
    <key>CFBundleVersion</key>
    <string>${APP_VERSION}</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.13</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF

# Fix library paths in main executable
EXECUTABLE="${DIST_DIR}/${BUNDLE_NAME}/Contents/MacOS/${APP_NAME}"

# Function to fix library paths
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
            new_path="@executable_path/../Frameworks/${framework_name}.framework/${framework_name}"
        else
            new_path="@executable_path/../Frameworks/${lib_name}"
        fi

        echo "  Changing $current_path -> $new_path"
        install_name_tool -change "$current_path" "$new_path" "$binary"
    done
}

# Fix paths in main executable
fix_library_paths "$EXECUTABLE"

# Fix paths in core libraries
find "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks" -type f -name "*.dylib" | while read -r lib; do
    fix_library_paths "$lib"
    install_name_tool -id "@executable_path/../Frameworks/$(basename "$lib")" "$lib"
done

# Fix paths in plugins
find "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns" -type f -name "*.dylib" | while read -r plugin; do
    fix_library_paths "$plugin"
    install_name_tool -id "@executable_path/../PlugIns/imagefilters/$(basename "$plugin")" "$plugin"
done

# Fix Qt frameworks
for framework in "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/"*.framework/Versions/5/*; do
    if [ -f "$framework" ] && [ -x "$framework" ]; then
        fix_library_paths "$framework"
    fi
done

# Create DMG
hdiutil create -volname "${APP_NAME}" -srcfolder "${DIST_DIR}/${BUNDLE_NAME}" -ov -format UDZO "${DIST_DIR}/${DMG_NAME}"

echo "Packaging complete!"
echo "Bundle created at: ${DIST_DIR}/${BUNDLE_NAME}"
echo "DMG created at: ${DIST_DIR}/${DMG_NAME}"
