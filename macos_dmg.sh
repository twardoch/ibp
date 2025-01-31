#!/usr/bin/env bash

# Exit on error
set -e

# Configuration
APP_NAME="ImageBatchProcessor"
BUNDLE_NAME="${APP_NAME}.app"
DIST_DIR="dist"

# External library paths
QT_PATH="$(brew --prefix qt@5)"

rm -rf "${DIST_DIR}/${BUNDLE_NAME}.dmg"

# Create DMG
echo "Creating DMG..."
"${QT_PATH}/bin/macdeployqt" "${DIST_DIR}/${BUNDLE_NAME}" \
    -verbose=2 \
    -dmg \
    -hardened-runtime \
    -timestamp \
    -fs=HFS+

echo "Created: ${DIST_DIR}/${BUNDLE_NAME}.dmg"
