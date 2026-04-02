#!/usr/bin/env bash
set -euo pipefail

# Render Doxygen docs locally the same way as .github/workflows/build-doxygen.yml
#
# Usage:
#   bash .github/render_doxygen_local.sh
#
# Output:
#   doxygen/ (and whatever your CMake/Doxygen config produces)

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

# 4) Copy doxygen/* to repo root (same as workflow)
echo "==> Copying doxygen/* into repo root"
shopt -s nullglob
COPIED_ITEMS=(doxygen/*)
declare -A PREEXISTING_ROOT_ITEMS
for item in "${COPIED_ITEMS[@]}"; do
	base_name="$(basename "$item")"
	if [[ -e "./$base_name" ]]; then
		PREEXISTING_ROOT_ITEMS["$base_name"]=1
	fi
done

if (( ${#COPIED_ITEMS[@]} > 0 )); then
	cp -R "${COPIED_ITEMS[@]}" ./
fi
cp FIMS_hexlogo.png ./doxygen/

# 5) Build doxygen docs via CMake/Ninja (same flags as workflow)
echo "==> Configuring CMake"
cmake -S . -B build -G Ninja -DDOXYGEN_WARN_AS_ERROR=FAIL_ON_WARNINGS

echo "==> Building (this runs Doxygen)"
cmake --build build

# 6) Clean up files copied from doxygen/ to repo root
if (( ${#COPIED_ITEMS[@]} > 0 )); then
	echo "==> Cleaning copied files from repo root"
	for item in "${COPIED_ITEMS[@]}"; do
		base_name="$(basename "$item")"
		if [[ -z "${PREEXISTING_ROOT_ITEMS[$base_name]+x}" ]]; then
			rm -rf "./$base_name"
		fi
	done
fi

echo "==> Done."
echo "Output should be under: $ROOT_DIR/doxygen"