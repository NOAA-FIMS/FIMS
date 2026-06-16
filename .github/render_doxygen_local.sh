#!/usr/bin/env bash
set -euo pipefail

# Render Doxygen docs locally the same way as .github/workflows/build-doxygen.yml
#
# Usage:
#   bash .github/render_doxygen_local.sh
#
# Output:
#   doxygen/html (and whatever your CMake/Doxygen config produces)

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

# 4) Build doxygen docs via CMake/Ninja (same flags as workflow)
echo "==> Configuring CMake"
cmake -S . -B build -G Ninja -DDOXYGEN_WARN_AS_ERROR=FAIL_ON_WARNINGS

echo "==> Building (this runs Doxygen)"
cmake --build build

echo "==> Done."
echo "Output should be under: $ROOT_DIR/doxygen/html"