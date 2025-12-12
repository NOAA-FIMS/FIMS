#!/bin/bash

# ------------------------------------------------------------------
# NOAA FIMS User Setup Script
# ------------------------------------------------------------------

set -e  # Exit immediately if a command fails

# --- DETECT SYSTEM ---
OS="$(uname -s)"
ARCH="$(uname -m)" 
case "${OS}" in
    Linux*)   MACHINE=Linux;;
    Darwin*)  MACHINE=Mac;;
    CYGWIN*|MINGW*|MSYS*) MACHINE=Windows;;
    *)        MACHINE="UNKNOWN:${OS}"
esac

echo ">>> Detected OS: $MACHINE ($ARCH)"

# --- QUARTO INSTALLATION ---
if command -v quarto &> /dev/null; then
    echo ">>> Quarto is already installed ($(quarto --version))."
else
    if [ "$MACHINE" == "Linux" ] && command -v sudo &> /dev/null; then
        echo ">>> Installing Quarto for Linux..."
        Q_ARCH="amd64"
        if [[ "$ARCH" == "aarch64" || "$ARCH" == "arm64" ]]; then Q_ARCH="arm64"; fi
        QUARTO_VER=$(curl -s "https://api.github.com/repos/quarto-dev/quarto-cli/releases/latest" | grep '"tag_name":' | sed -E 's/.*"v([^"]+)".*/\1/')
        wget -q -O "quarto.deb" "https://github.com/quarto-dev/quarto-cli/releases/download/v${QUARTO_VER}/quarto-${QUARTO_VER}-linux-${Q_ARCH}.deb"
        sudo dpkg -i "quarto.deb" && rm "quarto.deb"
    elif [ "$MACHINE" == "Mac" ] && command -v brew &> /dev/null; then
        echo ">>> Installing Quarto via Homebrew..."
        brew install --cask quarto
    fi
fi

# --- SYSTEM DEPENDENCIES ---
if [ "$MACHINE" == "Linux" ] && command -v sudo &> /dev/null; then
    echo ">>> Syncing Linux dependencies..."
    sudo apt-get update -qq
    sudo apt-get install -y --no-install-recommends build-essential libcairo2-dev libcurl4-openssl-dev libpoppler-cpp-dev libssl-dev libxml2-dev git wget perl xz-utils r-base
elif [ "$MACHINE" == "Mac" ]; then
    if ! xcode-select -p &>/dev/null; then
        echo ">>> Xcode CLI Tools not found. Starting installation..."
        xcode-select --install
        echo "!!! Action Required: Complete the Xcode popup and then rerun this script. !!!"
        exit 0
    fi
    if command -v brew &> /dev/null; then
        brew install openssl libxml2 curl cairo pkg-config poppler
    fi
fi

# --- R CONFIGURATION (.Rprofile) ---
R_PROF="$HOME/.Rprofile"
touch "$R_PROF"
[ -n "$(tail -c1 "$R_PROF")" ] && echo "" >> "$R_PROF"

if ! grep -Fq "packagemanager.posit.co" "$R_PROF"; then
    echo "options(repos = c(CRAN = 'https://packagemanager.posit.co/cran/latest'))" >> "$R_PROF"
fi

# --- R INSTALLATION WITH ERROR HANDLING ---
echo "--- Checking R Packages ---"

# This R code block performs the installation AND a final verification
R_CODE="
# Set Library Path
lib_loc <- Sys.getenv('R_LIBS_USER', unset = file.path('~', 'R', paste0('lib-fims-', getRversion())))
if (!dir.exists(lib_loc)) dir.create(lib_loc, recursive = TRUE)
.libPaths(c(lib_loc, .libPaths()))

# Define Requirements
pkgs <- c('languageserver', 'FIMS', 'asar', 'stockplotr', 'remotes', 'renv', 'rlang', 'tinytex')
repos <- c(
    'https://noaa-fisheries-integrated-toolbox.r-universe.dev',
    CRAN = 'https://packagemanager.posit.co/cran/latest'
)

# Install missing packages
installed_before <- installed.packages(lib.loc = lib_loc)[, 'Package']
missing <- setdiff(pkgs, installed_before)

if (length(missing) > 0) {
    message('>>> Installing missing: ', paste(missing, collapse = ', '))
    install.packages(missing, lib = lib_loc, repos = repos)
}

# Handle VS Code httpgd
if (Sys.getenv('TERM_PROGRAM') == 'vscode' && !'httpgd' %in% installed.packages()[, 'Package']) {
    remotes::install_github('nx10/httpgd', upgrade = 'never')
}

# TinyTeX check
if (require('tinytex', quietly = TRUE) && !tinytex::is_tinytex()) {
    tinytex::install_tinytex(force = TRUE)
}

# FINAL CRITICAL VERIFICATION
# This is where we force the script to fail if a package didn't install
final_installed <- installed.packages(lib.loc = lib_loc)[, 'Package']
failed <- setdiff(pkgs, final_installed)

if (length(failed) > 0) {
    # This stop() command causes Rscript to exit with Code 1
    stop('\n!!! FAILED TO INSTALL PACKAGES: ', paste(failed, collapse = ', '), ' !!!\n')
} else {
    message('>>> All R packages verified.')
}
"

# Execute R and catch failure
if ! Rscript -e "$R_CODE"; then
    echo "------------------------------------------------------------------"
    echo "!!! ERROR: R Setup failed. See errors above. !!!"
    echo "------------------------------------------------------------------"
    exit 1
fi

# --- FINAL VS CODE PLOTTING CONFIG ---
if [[ "$TERM_PROGRAM" == "vscode" ]] && ! grep -Fq "httpgd::hgd()" "$R_PROF"; then
    cat <<'EOT' >> "$R_PROF"
if (interactive() && Sys.getenv("TERM_PROGRAM") == "vscode") {
    if (requireNamespace("httpgd", quietly = TRUE)) {
        options(vsc.plot = FALSE)
    }
}
EOT
fi

echo ""
echo "******************************************************************"
echo "* FIMS Setup Complete for $MACHINE!"
echo "* All R packages and system dependencies are verified."
echo "******************************************************************"