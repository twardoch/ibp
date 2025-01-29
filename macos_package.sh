#!/usr/bin/env bash

# Exit on error
# set -e

# Configuration
APP_NAME="ImageBatchProcessor"
APP_VERSION="1.0.0"
BUNDLE_NAME="${APP_NAME}.app"
DMG_NAME="${APP_NAME}-${APP_VERSION}.dmg"
DIST_DIR="dist"

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

# Copy core libraries
cp build/libibp.*.dylib "${DIST_DIR}/${BUNDLE_NAME}/Contents/Frameworks/"

# Copy plugins
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters"
cp build/plugins/libibp.imagefilter.*.dylib "${DIST_DIR}/${BUNDLE_NAME}/Contents/PlugIns/imagefilters/"

# Copy settings
mkdir -p "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/settings"
cp build/settings/* "${DIST_DIR}/${BUNDLE_NAME}/Contents/Resources/settings/"

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
for lib in "${DIST_DIR}/${BUNDLE_NAME}"/Contents/Frameworks/libibp.*.dylib; do
    lib_name=$(basename "$lib")
    install_name_tool -change "@rpath/$lib_name" "@executable_path/../Frameworks/$lib_name" "$EXECUTABLE"
done

# Fix library paths in core libraries
for lib in "${DIST_DIR}/${BUNDLE_NAME}"/Contents/Frameworks/libibp.*.dylib; do
    for dep in "${DIST_DIR}/${BUNDLE_NAME}"/Contents/Frameworks/libibp.*.dylib; do
        dep_name=$(basename "$dep")
        install_name_tool -change "@rpath/$dep_name" "@executable_path/../Frameworks/$dep_name" "$lib"
    done
    install_name_tool -id "@executable_path/../Frameworks/$(basename "$lib")" "$lib"
done

# Fix library paths in plugins
for plugin in "${DIST_DIR}/${BUNDLE_NAME}"/Contents/PlugIns/imagefilters/libibp.imagefilter.*.dylib; do
    for lib in "${DIST_DIR}/${BUNDLE_NAME}"/Contents/Frameworks/libibp.*.dylib; do
        lib_name=$(basename "$lib")
        install_name_tool -change "@rpath/$lib_name" "@executable_path/../Frameworks/$lib_name" "$plugin"
    done
    install_name_tool -id "@executable_path/../PlugIns/imagefilters/$(basename "$plugin")" "$plugin"
done

# Create DMG
hdiutil create -volname "${APP_NAME}" -srcfolder "${DIST_DIR}/${BUNDLE_NAME}" -ov -format UDZO "${DIST_DIR}/${DMG_NAME}"

echo "Packaging complete!"
echo "Bundle created at: ${DIST_DIR}/${BUNDLE_NAME}"
echo "DMG created at: ${DIST_DIR}/${DMG_NAME}"
