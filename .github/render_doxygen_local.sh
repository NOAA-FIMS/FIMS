#!/usr/bin/env bash
set -euo pipefail

# Render Doxygen docs locally the same way as .github/workflows/build-doxygen.yml
#
# Usage:
#   bash .github/render_doxygen_local.sh
#
# Output:
#   build/html (and whatever your CMake/Doxygen config produces)

# ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# cd "$ROOT_DIR"

EXTERNAL_DOCS_DIR="${EXTERNAL_DOCS_DIR:-external-docs}"
EXTERNAL_DOCS_REPO="${EXTERNAL_DOCS_REPO:-https://github.com/noaa-fims/noaa-fims.github.io.git}"
EXTERNAL_DOCS_REF="${EXTERNAL_DOCS_REF:-gh-pages}"

DOCS_PATH="$EXTERNAL_DOCS_DIR/site_libs/bootstrap"

# 1) Fetch/update external docs repo (source of bootstrap CSS filenames)
if [[ -d "$EXTERNAL_DOCS_DIR/.git" ]]; then
  echo "==> Updating external docs repo in: $EXTERNAL_DOCS_DIR"
  git -C "$EXTERNAL_DOCS_DIR" fetch --quiet origin
  git -C "$EXTERNAL_DOCS_DIR" checkout --quiet "$EXTERNAL_DOCS_REF"
  git -C "$EXTERNAL_DOCS_DIR" pull --quiet --ff-only origin "$EXTERNAL_DOCS_REF"
else
  echo "==> Cloning external docs repo to: $EXTERNAL_DOCS_DIR"
  git clone --quiet --depth 1 --branch "$EXTERNAL_DOCS_REF" "$EXTERNAL_DOCS_REPO" "$EXTERNAL_DOCS_DIR"
fi

if [[ ! -d "$DOCS_PATH" ]]; then
  echo "ERROR: Expected bootstrap directory not found: $DOCS_PATH" >&2
  exit 1
fi

# 2) Find latest light + dark bootstrap css filenames in the external repo
BOOTSTRAP_LIGHT="$(ls -1 "$DOCS_PATH" | sort | grep -E '^bootstrap-[0-9a-f]+\.min\.css$' | grep -v 'dark' | head -n 1 || true)"
BOOTSTRAP_DARK="$(ls -1 "$DOCS_PATH" | sort | grep -E '^bootstrap-dark-[0-9a-f]+\.min\.css$' | head -n 1 || true)"

if [[ -z "$BOOTSTRAP_LIGHT" || -z "$BOOTSTRAP_DARK" ]]; then
  echo "ERROR: Could not find both light and dark bootstrap CSS files in: $DOCS_PATH" >&2
  echo "Found light: '$BOOTSTRAP_LIGHT'" >&2
  echo "Found dark:  '$BOOTSTRAP_DARK'" >&2
  exit 1
fi

echo "==> Bootstrap (light): $BOOTSTRAP_LIGHT"
echo "==> Bootstrap (dark) : $BOOTSTRAP_DARK"

# 3) Update header.html in-place in assets/, using a temp copy like the workflow
tmp_header="$(mktemp -t fims-header.XXXXXX.html)"
cp "assets/header.html" "$tmp_header"

OLD_LIGHT="$(grep -oE 'bootstrap-[0-9a-f]+\.min\.css' "$tmp_header" | grep -v 'dark' | head -n 1 || true)"
OLD_DARK="$(grep -oE 'bootstrap-dark-[0-9a-f]+\.min\.css' "$tmp_header" | head -n 1 || true)"

if [[ -z "$OLD_LIGHT" || -z "$OLD_DARK" ]]; then
  echo "ERROR: Could not find old hard-coded bootstrap filenames in assets/header.html" >&2
  echo "Old light: '$OLD_LIGHT'" >&2
  echo "Old dark:  '$OLD_DARK'" >&2
  rm -f "$tmp_header"
  exit 1
fi

# Replace both (separately to avoid cross-matching)
# Use BSD/macOS compatible sed if needed
if sed --version >/dev/null 2>&1; then
  # GNU sed
  sed -i "s/${OLD_LIGHT}/${BOOTSTRAP_LIGHT}/g" "$tmp_header"
  sed -i "s/${OLD_DARK}/${BOOTSTRAP_DARK}/g" "$tmp_header"
else
  # BSD sed (macOS)
  sed -i '' "s/${OLD_LIGHT}/${BOOTSTRAP_LIGHT}/g" "$tmp_header"
  sed -i '' "s/${OLD_DARK}/${BOOTSTRAP_DARK}/g" "$tmp_header"
fi

if ! cmp -s "$tmp_header" "assets/header.html"; then
  echo "==> Updating assets/header.html (bootstrap hash changed)"
  cp "$tmp_header" "assets/header.html"
else
  echo "==> assets/header.html already up-to-date"
fi
rm -f "$tmp_header"

# 4) Copy assets/* to repo root (same as workflow)
echo "==> Copying assets/* into repo root"
cp -R assets/* ./

# 5) Build doxygen docs via CMake/Ninja (same flags as workflow)
echo "==> Configuring CMake"
cmake -S . -B build -G Ninja -DDOXYGEN_WARN_AS_ERROR=FAIL_ON_WARNINGS

echo "==> Building (this runs Doxygen)"
cmake --build build

echo "==> Done."
echo "Output should be under: $ROOT_DIR/build/html"