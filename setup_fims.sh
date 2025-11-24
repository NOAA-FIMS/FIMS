#!/bin/bash

# ------------------------------------------------------------------
# NOAA FIMS User Setup Script
# Description: Automates the installation of Quarto, system dependencies, and R # packages for the FIMS project.
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
    sudo apt-get install -y --no-install-recommends build-essential libcairo2-dev libcurl4-openssl-dev libpoppler-cpp-dev libssl-dev libxml2-dev git wget perl xz-utils r-base libharfbuzz-dev libfribidi-dev libfreetype6-dev libpng-dev libtiff5-dev libjpeg-dev libwebp-dev liblapack-dev libblas-dev
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
# Ensure file ends with a newline before appending
[ -n "$(tail -c1 "$R_PROF")" ] && echo "" >> "$R_PROF"

if ! grep -Fq "noaa-fisheries-integrated-toolbox" "$R_PROF"; then
    echo "options(repos = c(NOAA = 'https://noaa-fisheries-integrated-toolbox.r-universe.dev', CRAN = 'https://packagemanager.posit.co/cran/latest'))" >> "$R_PROF"
fi

# --- R Package Installation ---
echo "--- Checking R Packages ---"

# This R code block performs the installation AND a final verification
R_CODE="
# Define local library path
target_lib <- Sys.getenv('R_LIBS_USER', unset = file.path('~', 'R', paste0('lib-fims-', getRversion())))
if (!dir.exists(target_lib)) dir.create(target_lib, recursive = TRUE)
.libPaths(c(target_lib, .libPaths()))

# Package requirements
required_pkgs <- c('languageserver', 'renv', 'FIMS', 'stockplotr')
repos_config <- c(
    NOAA = 'https://noaa-fisheries-integrated-toolbox.r-universe.dev',
    CRAN = 'https://packagemanager.posit.co/cran/latest'
)

# Primary Installation via {pak}
if (!requireNamespace('pak', quietly = TRUE)) install.packages('pak', repos = repos_config['CRAN'])

message('>>> Attempting installation via {pak}...')
tryCatch({
    pak::pkg_install(required_pkgs, ask = FALSE)
}, error = function(e) {
    message('>>> {pak} encountered an issue. Falling back to base install.packages()...')
})

# Secondary/Fallback Installation via base R
current_installed <- installed.packages(lib.loc = target_lib)[, 'Package']
missing_pkgs <- setdiff(required_pkgs, current_installed)

if (length(missing_pkgs) > 0) {
    message('>>> Installing missing packages: ', paste(missing_pkgs, collapse = ', '))
    install.packages(missing_pkgs, lib = target_lib, repos = repos_config, INSTALL_opts = '--no-lock')
}

# Development Tools (VS Code Support)
if (Sys.getenv('TERM_PROGRAM') == 'vscode') {
    if (!'httpgd' %in% installed.packages()[, 'Package']) {
        if (!requireNamespace('remotes', quietly = TRUE)) install.packages('remotes')
        remotes::install_github('nx10/httpgd')
    }
}

# Final Validation
final_check <- setdiff(required_pkgs, installed.packages(lib.loc = target_lib)[, 'Package'])
if (length(final_check) > 0) {
    stop('Critical Error: The following packages failed to install: ', paste(final_check, collapse = ', '))
} else {
    message('>>> All R packages successfully verified.')
    fims_version <- packageVersion('FIMS')
    message('>>> FIMS version: ', fims_version)
    message('>>> Installed at: ', target_lib)
}
"

# Create temporary R script
R_TEMP_FILE=$(mktemp)
# Rename it to have an .R extension so Rscript is happy
mv "$R_TEMP_FILE" "${R_TEMP_FILE}.R"
R_TEMP_FILE="${R_TEMP_FILE}.R"

cleanup() {
  rm "$R_TEMP_FILE"
}
trap cleanup EXIT

echo "$R_CODE" > "$R_TEMP_FILE"

# Execute with System Requirements disabled to avoid libnode-dev conflicts
export PKG_SYSREQS=false

if ! Rscript "$R_TEMP_FILE"; then
    echo "------------------------------------------------------------------"
    echo "!!! ERROR: R Setup failed. See errors above. !!!"
    echo "------------------------------------------------------------------"
    exit 1
fi

# --- Post-Installation Config (VS Code Plotting) ---
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
echo "* FIMS Setup Complete!"
echo "* Platform: $MACHINE"
echo "* All R packages and system dependencies are verified."
echo "* Run 'renv::init()' in the R console to create reproducible environments
        for your R projects. Learn more about renv:
        https://rstudio.github.io/renv/articles/renv.html"
echo "******************************************************************"