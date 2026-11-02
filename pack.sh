#!/bin/bash
# Pack the built LowerThirdsPlus plugin into a distributable zip and DMG.
# Run from repo root after building (e.g. ./build.sh ... or CI build).
# Output: LowerThirdsPlus-v1.0-macOS.zip and optionally LowerThirdsPlus-v1.0-macOS.dmg

set -e

VERSION="${1:-1.0}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Find built plugin
PLUGIN=
for candidate in build/Release/LowerThirdsPlus.plugin build/LowerThirdsPlus.plugin build/*/LowerThirdsPlus.plugin; do
  if [ -d "$candidate" ]; then
    PLUGIN="$candidate"
    break
  fi
done

if [ -z "$PLUGIN" ] || [ ! -d "$PLUGIN" ]; then
  echo "Error: Built plugin not found. Build first with ./build.sh <OBS_PATH>"
  echo "Looked in: build/Release/, build/, build/*/"
  exit 1
fi

echo "Using plugin: $PLUGIN"

PACK_NAME="LowerThirdsPlus-v${VERSION}-macOS"
STAGING="pack_staging/${PACK_NAME}"
rm -rf "$STAGING"
mkdir -p "$STAGING"

# Copy plugin
cp -R "$PLUGIN" "$STAGING/LowerThirdsPlus.plugin"

# Copy Distribution docs and installer
for f in README.md INSTALL.md INSTALL.command USER_GUIDE.md QUICK_REFERENCE.md HOW_TO_INSTALL_ON_MACOS.md; do
  if [ -f "Distribution/$f" ]; then
    cp "Distribution/$f" "$STAGING/"
  fi
done
chmod +x "$STAGING/INSTALL.command"

# Create zip
ZIP_NAME="${PACK_NAME}.zip"
rm -f "$ZIP_NAME"
ditto -c -k --sequesterRsrc --keepParent "$STAGING" "$ZIP_NAME"
echo "Created: $ZIP_NAME"

# Create DMG if hdiutil available
DMG_NAME="${PACK_NAME}.dmg"
if command -v hdiutil &>/dev/null; then
  rm -f "$DMG_NAME"
  hdiutil create -volname "LowerThirdsPlus ${VERSION}" -srcfolder "$STAGING" -ov -format UDZO "$DMG_NAME"
  echo "Created: $DMG_NAME"
else
  echo "Skipping DMG (hdiutil not found)"
fi

# Cleanup
rm -rf pack_staging

echo "Done. Distributables: $ZIP_NAME" $([ -f "$DMG_NAME" ] && echo "$DMG_NAME")
