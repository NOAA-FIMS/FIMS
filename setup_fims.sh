#!/bin/bash

# ------------------------------------------------------------------
# NOAA FIMS User Setup Script
# Description: Automates installation of Quarto, system dependencies,
# R configuration, and R packages for the FIMS project.
# ------------------------------------------------------------------

set -e

NOAA_REPO="https://noaa-fisheries-integrated-toolbox.r-universe.dev"
CRAN_REPO="https://packagemanager.posit.co/cran/latest"
R_MIN_VERSION="4.1.0"

info() {
    echo ">>> $*"
}

warn() {
    echo "!!! $*"
}

die() {
    warn "$*"
    exit 1
}

command_exists() {
    command -v "$1" >/dev/null 2>&1
}

version_at_least() {
    # Returns success when $1 >= $2. Uses sort -V when available and falls
    # back to R for macOS/BSD environments.
    if command_exists sort && sort -V </dev/null >/dev/null 2>&1; then
        [ "$(printf '%s\n%s\n' "$2" "$1" | sort -V | head -n1)" = "$2" ]
    elif command_exists Rscript; then
        Rscript --vanilla -e "quit(status = numeric_version('$1') < numeric_version('$2'))" >/dev/null 2>&1
    else
        return 1
    fi
}

install_with_brew() {
    local packages=("$@")
    local missing=()

    for package in "${packages[@]}"; do
        local installed_name="${package##*/}"
        if ! brew list --formula "$installed_name" >/dev/null 2>&1; then
            missing+=("$package")
        fi
    done

    if [ "${#missing[@]}" -eq 0 ]; then
        info "Homebrew system dependencies are already installed."
        return 0
    fi

    info "Installing Homebrew dependencies: ${missing[*]}"
    brew install --formula "${missing[@]}"
}

install_brew_cask() {
    local cask="$1"
    local installed_name="${cask##*/}"

    if brew list --cask "$installed_name" >/dev/null 2>&1; then
        info "Homebrew cask $installed_name is already installed."
        return 0
    fi

    info "Installing Homebrew cask: $cask"
    brew install --cask "$cask"
}

print_manual_mac_deps() {
    cat <<'EOT'
Install Homebrew from https://brew.sh/, then rerun this script.

If Homebrew is not available on your machine, ask your local administrator to
install the libraries needed by R packages such as gdtools, textshaping,
systemfonts, curl, openssl, and poppler:

  cairo curl fontconfig freetype fribidi harfbuzz jpeg-turbo libgit2
  gettext libpng libtiff libxml2 openssl pkg-config poppler
EOT
}

print_windows_rtools_help() {
    cat <<'EOT'
Install the Rtools version that matches your installed R version, then open a
new Git Bash terminal and rerun this script.

Download Rtools from:
  https://cran.r-project.org/bin/windows/Rtools/

If you recently upgraded R, remove older Rtools entries from PATH or make sure
the matching Rtools version appears first.
EOT
}

# --- DETECT SYSTEM ---
OS="$(uname -s)"
ARCH="$(uname -m)"
case "${OS}" in
    Linux*)   MACHINE=Linux;;
    Darwin*)  MACHINE=Mac;;
    CYGWIN*|MINGW*|MSYS*) MACHINE=Windows;;
    *)        MACHINE="UNKNOWN:${OS}";;
esac

info "Detected OS: $MACHINE ($ARCH)"

# --- PRE-FLIGHT CHECKS ---
info "Checking command line tools..."
command_exists curl || die "curl is required but was not found. Install curl and rerun this script."

if [ "$MACHINE" = "Linux" ]; then
    command_exists sudo || die "sudo is required on Linux so the script can install system dependencies."
    command_exists wget || warn "wget is not installed yet; it will be installed with the Linux dependencies."
elif [ "$MACHINE" = "Mac" ]; then
    if ! xcode-select -p >/dev/null 2>&1; then
        warn "Xcode Command Line Tools were not found."
        info "Starting the Apple installer. Complete the popup, then rerun this script."
        xcode-select --install
        exit 0
    fi

    if ! command_exists brew; then
        warn "Homebrew was not found, so this script cannot install macOS system libraries."
        print_manual_mac_deps
        exit 1
    fi
elif [ "$MACHINE" = "Windows" ]; then
    warn "Windows detected. This script is intended for Git Bash/Rtools-style shells."
    warn "If system dependencies fail, consider WSL2 or the FIMS container workflow."
elif [[ "$MACHINE" == UNKNOWN:* ]]; then
    warn "Unsupported operating system: $OS. Continuing with R setup only."
fi

if command_exists Rscript; then
    info "Detected Rscript: $(command -v Rscript)"
    R_VERSION="$(Rscript --vanilla -e "cat(as.character(getRversion()))" 2>/dev/null || true)"
    if [ -n "$R_VERSION" ]; then
        info "Detected R version: $R_VERSION"
        if ! version_at_least "$R_VERSION" "$R_MIN_VERSION"; then
            die "FIMS requires R >= $R_MIN_VERSION. Please update R, then rerun this script."
        fi
    else
        warn "Rscript exists, but R did not report a version cleanly."
    fi
else
    if [ "$MACHINE" = "Linux" ]; then
        warn "Rscript is not installed yet; r-base will be installed with Linux dependencies."
    elif [ "$MACHINE" = "Mac" ]; then
        die "Rscript was not found. Install R from https://cran.r-project.org/bin/macosx/ and rerun this script."
    fi
fi

if [ "$MACHINE" = "Windows" ]; then
    info "Checking Windows Rtools toolchain..."
    MAKE_PATH="$(command -v make || true)"
    GPP_PATH="$(command -v g++ || true)"
    info "Rtools make: ${MAKE_PATH:-<not found>}"
    info "Rtools g++: ${GPP_PATH:-<not found>}"

    EXPECTED_RTOOLS="rtools40"
    if version_at_least "$R_VERSION" "4.5.0"; then
        # CRAN lists Rtools 4.5 for R >= 4.5.0, including R-devel.
        EXPECTED_RTOOLS="rtools45"
    elif version_at_least "$R_VERSION" "4.4.0"; then
        EXPECTED_RTOOLS="rtools44"
    elif version_at_least "$R_VERSION" "4.3.0"; then
        EXPECTED_RTOOLS="rtools43"
    elif version_at_least "$R_VERSION" "4.2.0"; then
        EXPECTED_RTOOLS="rtools42"
    fi

    if [ -z "$MAKE_PATH" ] || [ -z "$GPP_PATH" ]; then
        warn "Rtools make and/or g++ were not found on PATH."
        print_windows_rtools_help
        die "Windows Rtools is missing or incomplete."
    fi

    TOOL_PATHS="$(printf '%s %s' "$MAKE_PATH" "$GPP_PATH" | tr '[:upper:]' '[:lower:]')"
    GPP_TARGET="$(g++ -dumpmachine 2>/dev/null || true)"
    if [ -n "$GPP_TARGET" ]; then
        info "Rtools g++ target: $GPP_TARGET"
    fi
    if [ -n "$GPP_TARGET" ] && [[ "$GPP_TARGET" != *"w64-mingw32"* ]]; then
        warn "g++ does not look like the Windows Rtools compiler target: $GPP_TARGET"
        print_windows_rtools_help
        die "Windows Rtools compiler is not configured correctly."
    fi

    if [[ "$TOOL_PATHS" != *"$EXPECTED_RTOOLS"* ]]; then
        warn "R $R_VERSION usually uses $EXPECTED_RTOOLS, but the shell paths do not include that directory name."
        warn "Continuing because make and g++ were found; GitHub Actions often exposes Rtools as /usr/bin and /x86_64-w64-mingw32.static.posix/bin."
    fi
fi

# --- QUARTO INSTALLATION ---
if command_exists quarto; then
    info "Quarto is already installed ($(quarto --version))."
else
    if [ "$MACHINE" = "Linux" ]; then
        info "Installing Quarto for Linux..."
        Q_ARCH="amd64"
        if [[ "$ARCH" == "aarch64" || "$ARCH" == "arm64" ]]; then
            Q_ARCH="arm64"
        fi
        QUARTO_VER="$(curl -fsSL "https://api.github.com/repos/quarto-dev/quarto-cli/releases/latest" | grep '"tag_name":' | sed -E 's/.*"v([^"]+)".*/\1/')"
        [ -n "$QUARTO_VER" ] || die "Could not determine the latest Quarto version from GitHub."
        curl -fsSL -o "quarto.deb" "https://github.com/quarto-dev/quarto-cli/releases/download/v${QUARTO_VER}/quarto-${QUARTO_VER}-linux-${Q_ARCH}.deb"
        sudo dpkg -i "quarto.deb"
        rm "quarto.deb"
    elif [ "$MACHINE" = "Mac" ]; then
        info "Installing Quarto via Homebrew..."
        install_brew_cask homebrew/cask/quarto
    else
        warn "Quarto is not installed and cannot be installed automatically on this platform."
    fi
fi

# --- SYSTEM DEPENDENCIES ---
if [ "$MACHINE" = "Linux" ]; then
    info "Syncing Linux dependencies..."
    sudo apt-get update -qq
    sudo apt-get install -y --no-install-recommends \
        build-essential git libblas-dev libcairo2-dev libcurl4-openssl-dev \
        libfontconfig1-dev libfreetype6-dev libfribidi-dev libgit2-dev \
        libharfbuzz-dev libjpeg-dev liblapack-dev libpng-dev libpoppler-cpp-dev \
        libssl-dev libtiff5-dev libwebp-dev libxml2-dev perl pkg-config \
        r-base wget xz-utils
elif [ "$MACHINE" = "Mac" ]; then
    info "Syncing macOS dependencies with Homebrew..."
    install_with_brew \
        homebrew/core/cairo homebrew/core/curl homebrew/core/fontconfig \
        homebrew/core/freetype homebrew/core/fribidi homebrew/core/harfbuzz \
        homebrew/core/jpeg-turbo homebrew/core/libgit2 homebrew/core/gettext \
        homebrew/core/libomp \
        homebrew/core/libpng homebrew/core/libtiff homebrew/core/libxml2 \
        homebrew/core/openssl homebrew/core/pkg-config homebrew/core/poppler

    BREW_PREFIX="$(brew --prefix)"
    export PKG_CONFIG_PATH="${BREW_PREFIX}/opt/curl/lib/pkgconfig:${BREW_PREFIX}/opt/gettext/lib/pkgconfig:${BREW_PREFIX}/opt/openssl/lib/pkgconfig:${BREW_PREFIX}/opt/libxml2/lib/pkgconfig:${BREW_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
    export LDFLAGS="-L${BREW_PREFIX}/opt/libomp/lib -L${BREW_PREFIX}/opt/gettext/lib ${LDFLAGS:-}"
    export CPPFLAGS="-I${BREW_PREFIX}/opt/libomp/include -I${BREW_PREFIX}/opt/gettext/include ${CPPFLAGS:-}"
    export LIBRARY_PATH="${BREW_PREFIX}/opt/libomp/lib:${BREW_PREFIX}/opt/gettext/lib:${LIBRARY_PATH:-}"
    export DYLD_FALLBACK_LIBRARY_PATH="${BREW_PREFIX}/opt/libomp/lib:${DYLD_FALLBACK_LIBRARY_PATH:-}"
    export PATH="${BREW_PREFIX}/opt/curl/bin:${BREW_PREFIX}/opt/gettext/bin:${BREW_PREFIX}/opt/libxml2/bin:${PATH}"
    info "Configured PKG_CONFIG_PATH for Homebrew libraries used by gdtools and related R packages."
fi

# R may have been installed above on Linux.
command_exists Rscript || die "Rscript is required but was not found after system dependency installation."

FIMS_R_LIB="$(Rscript --vanilla -e "cat(normalizePath(Sys.getenv('R_LIBS_USER', unset = file.path('~', 'R', paste0('lib-fims-', getRversion()))), winslash = '/', mustWork = FALSE))")"
export R_LIBS_USER="$FIMS_R_LIB"
info "FIMS R library path: $FIMS_R_LIB"

if [ -n "${GITHUB_ENV:-}" ]; then
    info "Persisting R_LIBS_USER for later GitHub Actions steps."
    {
        echo "R_LIBS_USER=$FIMS_R_LIB"
        echo "PKG_CONFIG_PATH=$PKG_CONFIG_PATH"
        echo "LDFLAGS=$LDFLAGS"
        echo "CPPFLAGS=$CPPFLAGS"
        echo "LIBRARY_PATH=$LIBRARY_PATH"
        echo "DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH"
    } >> "$GITHUB_ENV"
fi

# --- R CONFIGURATION (.Rprofile) ---
R_PROF="${R_PROFILE_USER:-$HOME/.Rprofile}"
R_ENVIRON="${R_ENVIRON_USER:-$HOME/.Renviron}"
R_PROFILE_MARKER_START="# >>> FIMS setup >>>"
R_PROFILE_MARKER_END="# <<< FIMS setup <<<"

info "Configuring R profile: $R_PROF"
if [ ! -f "$R_PROF" ]; then
    info "No .Rprofile file was found. Creating $R_PROF."
    touch "$R_PROF"
else
    info "Using existing .Rprofile file."
fi

# Ensure file ends with a newline before appending.
if [ -s "$R_PROF" ] && [ -n "$(tail -c1 "$R_PROF")" ]; then
    echo "" >> "$R_PROF"
fi

if grep -Fq "$R_PROFILE_MARKER_START" "$R_PROF"; then
    info "FIMS .Rprofile block already exists; leaving it unchanged."
else
    if grep -Fq "noaa-fisheries-integrated-toolbox" "$R_PROF"; then
        info "NOAA R-universe repository already appears in .Rprofile."
        info "Adding the managed FIMS block so the FIMS library path is available in future R sessions."
    else
        info "Adding FIMS repository and library settings to .Rprofile."
    fi
    cat <<EOT >> "$R_PROF"
$R_PROFILE_MARKER_START
local({
  fims_lib <- Sys.getenv(
    "R_LIBS_USER",
    unset = "$FIMS_R_LIB"
  )
  libomp_fallback <- Sys.getenv(
    "DYLD_FALLBACK_LIBRARY_PATH",
    unset = ""
  )
  if (!dir.exists(fims_lib)) {
    dir.create(fims_lib, recursive = TRUE, showWarnings = FALSE)
  }
  .libPaths(unique(c(fims_lib, .libPaths())))
  if (nzchar(libomp_fallback)) {
    Sys.setenv(
      DYLD_FALLBACK_LIBRARY_PATH = libomp_fallback,
      DYLD_LIBRARY_PATH = libomp_fallback
    )
    libomp_candidates <- unique(file.path(strsplit(libomp_fallback, ":", fixed = TRUE)[[1]], "libomp.dylib"))
    libomp_path <- libomp_candidates[file.exists(libomp_candidates)][1]
    if (!is.na(libomp_path) && nzchar(libomp_path)) {
      try(dyn.load(libomp_path, local = FALSE, now = TRUE), silent = TRUE)
    }
  }
  options(repos = c(
    NOAA = "$NOAA_REPO",
    CRAN = "$CRAN_REPO"
  ))
})
$R_PROFILE_MARKER_END
EOT
fi

info "Configuring R environment file: $R_ENVIRON"
if [ ! -f "$R_ENVIRON" ]; then
    info "No .Renviron file was found. Creating $R_ENVIRON."
    touch "$R_ENVIRON"
else
    info "Using existing .Renviron file."
fi

if [ -s "$R_ENVIRON" ] && [ -n "$(tail -c1 "$R_ENVIRON")" ]; then
    echo "" >> "$R_ENVIRON"
fi

if grep -Fq "$R_PROFILE_MARKER_START" "$R_ENVIRON"; then
    info "FIMS .Renviron block already exists; leaving it unchanged."
else
    info "Adding FIMS library settings to .Renviron."
    cat <<EOT >> "$R_ENVIRON"
$R_PROFILE_MARKER_START
R_LIBS_USER="$FIMS_R_LIB"
DYLD_FALLBACK_LIBRARY_PATH="${DYLD_FALLBACK_LIBRARY_PATH:-}"
DYLD_LIBRARY_PATH="${DYLD_FALLBACK_LIBRARY_PATH:-}"
$R_PROFILE_MARKER_END
EOT
fi

# --- R PACKAGE INSTALLATION ---
info "Checking R packages..."

R_CODE="
message('>>> Starting FIMS R package setup.')

target_lib <- Sys.getenv(
  'R_LIBS_USER',
  unset = file.path('~', 'R', paste0('lib-fims-', getRversion()))
)
if (!dir.exists(target_lib)) {
  dir.create(target_lib, recursive = TRUE, showWarnings = FALSE)
}
.libPaths(unique(c(target_lib, .libPaths())))

repos_config <- c(
  NOAA = '$NOAA_REPO',
  CRAN = '$CRAN_REPO'
)
options(repos = repos_config)

required_pkgs <- c('languageserver', 'renv', 'FIMS', 'stockplotr')
diagnostic_pkgs <- c('gdtools', 'systemfonts', 'textshaping')
install_targets <- unique(c(required_pkgs, diagnostic_pkgs))

message('>>> R library path: ', target_lib)
message('>>> Repositories: ', paste(names(repos_config), repos_config, sep = '=', collapse = ', '))

install_base <- function(pkgs) {
  if (length(pkgs) == 0) {
    return(invisible(TRUE))
  }
  message('>>> Installing with install.packages(): ', paste(pkgs, collapse = ', '))
  install.packages(pkgs, lib = target_lib, repos = repos_config, INSTALL_opts = '--no-lock')
}

if (!requireNamespace('pkgbuild', quietly = TRUE)) {
  message('>>> Installing helper package pkgbuild...')
  install_base('pkgbuild')
}

tool_status <- tryCatch({
  pkgbuild::check_build_tools(debug = FALSE)
}, error = function(e) {
  message('>>> Could not run pkgbuild::check_build_tools(): ', conditionMessage(e))
  FALSE
})

if (!isTRUE(tool_status)) {
  message('>>> R build tools may be missing or outdated.')
  if (Sys.info()[['sysname']] == 'Darwin') {
    message('>>> On macOS, run xcode-select --install and make sure Homebrew dependencies installed successfully.')
  } else if (.Platform[['OS.type']] == 'windows') {
    message('>>> On Windows, install the Rtools version that matches your R release.')
  }
}

missing_from_target <- function(pkgs) {
  installed <- rownames(installed.packages(lib.loc = .libPaths()))
  setdiff(pkgs, installed)
}

if (!requireNamespace('pak', quietly = TRUE)) {
  message('>>> Installing helper package pak...')
  install_base('pak')
}

tryCatch({
  pak::pkg_install(install_targets, lib = target_lib, ask = FALSE, upgrade = FALSE)
}, error = function(e) {
  message('>>> {pak} encountered an issue: ', conditionMessage(e))
  message('>>> Falling back to install.packages().')
})

missing_pkgs <- missing_from_target(install_targets)
if (length(missing_pkgs) > 0) {
  install_base(missing_pkgs)
}

if (Sys.getenv('TERM_PROGRAM') == 'vscode') {
  if (!requireNamespace('httpgd', quietly = TRUE)) {
    message('>>> Installing httpgd for VS Code plotting support...')
    if (!requireNamespace('remotes', quietly = TRUE)) {
      install_base('remotes')
    }
    remotes::install_github('nx10/httpgd', lib = target_lib, upgrade = 'never')
  }
}

final_check <- missing_from_target(install_targets)
if (length(final_check) > 0) {
  message('')
  message('>>> Packages still missing: ', paste(final_check, collapse = ', '))
  message('>>> Troubleshooting hints:')
  if (Sys.info()[['sysname']] == 'Darwin') {
    message('>>>   - R packages such as gdtools need cairo, fontconfig, freetype, fribidi, gettext, harfbuzz, and pkg-config.')
    message('>>>   - Run: brew install cairo fontconfig freetype fribidi gettext harfbuzz libomp pkg-config')
    message('>>>   - Then rerun setup_fims.sh.')
  } else if (Sys.info()[['sysname']] == 'Linux') {
    message('>>>   - Make sure apt-get installed libcairo2-dev, libfontconfig1-dev, libfreetype6-dev, libfribidi-dev, and libharfbuzz-dev.')
  } else if (.Platform[['OS.type']] == 'windows') {
    message('>>>   - Make sure Rtools is installed and matches your R version.')
  }
  stop('Critical Error: Required FIMS setup packages failed to install.')
}

message('>>> All required R packages successfully verified.')
message('>>> FIMS version: ', as.character(packageVersion('FIMS')))
message('>>> Installed at: ', target_lib)
"

R_TEMP_FILE="$(mktemp "${TMPDIR:-/tmp}/fims-setup.XXXXXX.R")"
cleanup() {
    rm -f "$R_TEMP_FILE"
}
trap cleanup EXIT

printf '%s\n' "$R_CODE" > "$R_TEMP_FILE"

if ! Rscript --vanilla "$R_TEMP_FILE"; then
    echo "------------------------------------------------------------------"
    warn "R setup failed. Review the messages above for the missing tools."
    echo "------------------------------------------------------------------"
    exit 1
fi

# --- POST-INSTALLATION CONFIG (VS CODE PLOTTING) ---
if [[ "$TERM_PROGRAM" == "vscode" ]] && ! grep -Fq "options(vsc.plot = FALSE)" "$R_PROF"; then
    info "Adding VS Code plotting settings to .Rprofile."
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
echo "* R profile: $R_PROF"
echo "* All required R packages and system dependencies are verified."
echo "* Restart R or your IDE so the .Rprofile changes are loaded."
echo "******************************************************************"
