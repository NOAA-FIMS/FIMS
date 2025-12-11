#!/bin/bash

# ------------------------------------------------------------------
# NOAA FIMS User Setup Script
#
# USAGE: Users open their terminal (e.g., Google Cloud Workstations, Git Bash)
# and run:
# bash <(curl -sL https://raw.githubusercontent.com/NOAA-FIMS/FIMS/main/setup_fims.sh)
# ------------------------------------------------------------------

# Exit immediately if any command fails. Essential for reliable scripting.
set -e

# --- DETECT OPERATING SYSTEM ---
# Get the operating system name (Linux, Darwin, etc.)
OS="$(uname -s)"
case "${OS}" in
    # Handles standard Linux and WSL2
    Linux*)     MACHINE=Linux;;
    # Handles macOS
    Darwin*)    MACHINE=Mac;;
    # Windows (Cygwin environment)
    CYGWIN*)    MACHINE=Windows;;
    # Windows (Git Bash / MinGW environment)
    MINGW*)     MACHINE=Windows;;
    # Windows (MSYS environment)
    MSYS*)      MACHINE=Windows;;
    # Catch any other OS
    *)          MACHINE="UNKNOWN:${OS}"
esac

echo ">>> Detected OS: $MACHINE"

# --- CONFIGURATION ---
NON_ROOT_USER=$(whoami)
USER_HOME="$HOME"
# Marker file to check if setup has been completed previously
MARKER_FILE="$USER_HOME/.fims_setup_complete"

# --- Quarto check and installation ---
echo "--- Checking for Quarto CLI ---"
if command -v quarto &> /dev/null; then
    echo ">>> Quarto is already installed ($(quarto --version))."
else
    echo ">>> Quarto not found."
    # Auto-install Quarto only on Linux machines with 'sudo' privileges (like Google Cloud Workstations)
    if [ "$MACHINE" == "Linux" ] && command -v sudo &> /dev/null; then
        echo ">>> Installing Quarto CLI (Latest Release)..."
        # Dynamically fetch the latest version tag from GitHub API
        QUARTO_VER=$(curl -s "https://api.github.com/repos/quarto-dev/quarto-cli/releases/latest" | grep '"tag_name":' | sed -E 's/.*"v([^"]+)".*/\1/')
        
        wget -O "quarto-${QUARTO_VER}-linux-amd64.deb" "https://github.com/quarto-dev/quarto-cli/releases/download/v${QUARTO_VER}/quarto-${QUARTO_VER}-linux-amd64.deb"
        
        sudo dpkg -i "quarto-${QUARTO_VER}-linux-amd64.deb"
        rm "quarto-${QUARTO_VER}-linux-amd64.deb"
        echo ">>> Quarto installed successfully."
    elif [ "$MACHINE" == "Windows" ]; then
        # On Windows, installation is manual due to varying shell environments
        echo ">>> WARNING: On Windows, please download the Quarto installer manually from https://quarto.org/docs/get-started/"
    else
        echo ">>> WARNING: Cannot auto-install Quarto on this OS without sudo. Please install manually."
    fi
fi

# --- SYSTEM DEPENDENCIES (LINUX ONLY) ---
if [ "$MACHINE" == "Linux" ]; then
    echo "--- [Linux] Checking System Dependencies via apt-get ---"

    # If on WSL, the clock often lags, breaking compilation. We attempt to sync it.
    if grep -q "Microsoft" /proc/version 2>/dev/null; then
        echo ">>> WSL2 detected. Attempting to sync system clock..."
        if command -v sudo &> /dev/null; then
            sudo hwclock -s 2>/dev/null || true
        fi
    fi
    
    # Check for sudo privileges
    if command -v sudo &> /dev/null; then
        # Set environment to prevent interactive prompts during apt-get
        export DEBIAN_FRONTEND=noninteractive
        sudo apt-get update
        
        # Dependencies for {languageserver}: libcurl4-openssl-dev libssl-dev libxml2-dev
        # Dependencies for TinyTeX logic: perl xz-utils
        # Dependencies for {pdftools} which is required by {asar}: libpoppler-cpp-dev
        # Dependencies for Cairo which is needed by gdtools, {flextable}, {asar}, and {stockplotr}
        DEPS="build-essential libcairo2-dev libcurl4-openssl-dev libpoppler-cpp-dev libssl-dev libxml2-dev git wget nano perl xz-utils"
        
        # Only add 'r-base' to the install list if R is not already present
        if command -v R &> /dev/null; then
            echo ">>> R is already installed ($(R --version | head -n 1)). Skipping 'r-base' install."
        else
            echo ">>> R not found. Adding 'r-base' to install list."
            DEPS="$DEPS r-base"
        fi

        # Install dependencies
        sudo apt-get install -y --no-install-recommends $DEPS
        
        # Cleanup
        sudo rm -rf /var/lib/apt/lists/*
    else
        echo ">>> WARNING: 'sudo' not found. Assuming system deps are pre-installed."
    fi
    
    echo ">>> System dependencies verified."

elif [ "$MACHINE" == "Windows" ]; then
    echo ">>> NOTE: On Windows, you must manually install:"
    echo "    1. R (https://cloud.r-project.org/)"
    echo "    2. RTools (required for compiling packages)"
    echo ">>> Proceeding to R package installation..."
fi

# --- Setup CRAN mirror ---
echo "--- Configuring R Environment ---"
# Setup CRAN mirror to Posit Package Manager (PPM)
# PPM provides system-specific BINARY packages for Linux, Mac, and Windows, 
# significantly speeding up installs and preventing compilation errors.
if [ ! -f "$USER_HOME/.Rprofile" ] || ! grep -Fq "options(repos" "$USER_HOME/.Rprofile"; then
    # Use PPM for Linux, Mac, and Windows (MINGW/MSYS/CYGWIN detection)
    # The default 'latest' PPM URL provides system-specific binaries.
    if [ "$MACHINE" != "UNKNOWN:${OS}" ]; then
        echo "options(repos = c(CRAN = 'https://packagemanager.posit.co/cran/latest'))" >> "$USER_HOME/.Rprofile"
        echo ">>> Configured R to use Posit Package Manager (PPM)."
    else
        # Fallback to standard CRAN for unknown or unsupported systems
        echo "options(repos = c(CRAN = 'https://cloud.r-project.org'))" >> "$USER_HOME/.Rprofile"
        echo ">>> Configured R to use standard CRAN mirror."
    fi
fi

# --- R PACKAGE INSTALLATION ---

if [ -f "$MARKER_FILE" ]; then
    echo "--- R Packages already installed (Marker found). Skipping. ---"
else
    echo "--- Installing R Packages (This may take time) ---"
    
    # Check if 'Rscript' is available in the path
    if ! command -v Rscript &> /dev/null; then
        echo "!!! ERROR: R is not installed or not in your PATH. !!!"
        echo "Please install R and try again."
        exit 1
    fi

    # Define the R code as a variable to be executed (Note: NO leading spaces here)
    R_CODE="
# Setup Library Path
lib_loc <- Sys.getenv('R_LIBS_USER', unset = file.path('~', 'R', paste0('lib-fims-', getRversion())))
dir.create(lib_loc, recursive = TRUE, showWarnings = FALSE)
.libPaths(c(lib_loc, .libPaths()))

# Define Base Packages
pkgs <- c(
    'languageserver',
    'FIMS',
    'asar',
    'stockplotr',
    'remotes',
    'renv',
    'rlang',
    'tinytex'
)

# Define the custom repositories for R-Universe packages (FIMS, asar, stockplotr)
repos <- c(
    'https://noaa-fisheries-integrated-toolbox.r-universe.dev',
    CRAN = 'https://packagemanager.posit.co/cran/latest',
    'https://cloud.r-project.org'
)

install.packages(pkgs, repos = repos)

# Check for VS Code and add httpgd
if (Sys.getenv('TERM_PROGRAM') == 'vscode') {
    message('>>> VS Code environment detected: Adding httpgd to install list.')
    remotes::install_github('nx10/httpgd')
}

# TinyTex Binary Installation
if (require('tinytex', lib.loc = lib_loc, quietly = TRUE)) {
    if (!tinytex::is_tinytex()) {
        message('>>> TinyTeX binary not found. Installing now...')
        tinytex::install_tinytex(force = TRUE)
    } else {
        message('>>> TinyTeX binary is already present.')
    }
}
"
    # --- Execute R Code based on OS ---
    Rscript -e "$R_CODE"

    # Check the exit code of Rscript (or cmd)
    R_EXIT_CODE=$?
    if [ $R_EXIT_CODE -eq 0 ]; then
        # Only mark complete if Rscript ran without error
        touch "$MARKER_FILE"
        echo ">>> Setup Complete! Marker created."
        
        # Set up httpgd
        if [[ "$TERM_PROGRAM" == "vscode" ]]; then
            echo ">>> VS Code detected: Configuring httpgd graphics device in .Rprofile..."
            
            # Check if we already added the httpgd config to avoid duplicates
            if ! grep -Fq "httpgd" "$USER_HOME/.Rprofile"; then
                cat <<EOT >> "$USER_HOME/.Rprofile"

# VS Code Configuration (Added by FIMS setup for httpgd plotting)
# This code runs whenever R starts in a VS Code interactive session.
if (interactive() && Sys.getenv("TERM_PROGRAM") == "vscode") {
    if (requireNamespace("httpgd", quietly = TRUE)) {
        options(vsc.plot = FALSE)
        # Setting the device option is what tells R to use httpgd automatically
        options(device = function(...) {
            httpgd::hgd()
        })
    }
}
EOT
                echo ">>> .Rprofile updated successfully."
            fi
        fi
    else
        # Exit with error status and display warning
        echo "!!! ERROR: R package installation failed (Exit Code: $R_EXIT_CODE). !!!"
        echo "!!! FIMS Setup is NOT complete. Please review the log for errors. !!!"
        exit $R_EXIT_CODE # Propagate the failure status
    fi    
fi

echo ""
echo "******************************************************************"
echo "* FIMS Setup Complete!"
echo "* Library: (R System Default Location)"
echo "* Note: Packages were installed to R's default library path."
echo "* You can confirm the exact path by running '.libPaths()' in the R console."
echo "* Run 'renv::init()' in the R console to create reproducible environments
        for your R projects. Learn more about renv:
        https://rstudio.github.io/renv/articles/renv.html"
echo "******************************************************************"
