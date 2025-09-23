#!/usr/bin/env bash
# Usage: scripts/upx-compress.sh <dist_dir>
set -euo pipefail
DIST=${1:-dist}
echo "Compressing artifacts in ${DIST} with UPX..."
if [ ! -d "${DIST}" ]; then
  echo "Dist directory not found: ${DIST}" >&2
  exit 1
fi

find "${DIST}" -type f \( -perm /111 -o -name '*.AppImage' -o -name '*.bin' \) | while read -r file; do
  echo "Processing: $file"
  # Try to compress; ignore non-compressible
  if upx --version >/dev/null 2>&1; then
    upx -9 --best "$file" || echo "UPX failed on $file, skipping"
  else
    echo "UPX not installed, skipping"
  fi
done

echo "UPX compression finished."
