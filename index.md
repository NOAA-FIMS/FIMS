# Overview

[![FIMS status
badge](https://noaa-fims.r-universe.dev/badges/FIMS)](https://noaa-fims.r-universe.dev/FIMS)
[![R-CMD-check
badge](https://github.com/NOAA-FIMS/FIMS/actions/workflows/call-r-cmd-check.yml/badge.svg?branch=main)](https://github.com/NOAA-FIMS/FIMS/actions/workflows/call-r-cmd-check.yml)
[![Codecov test
coverage](https://codecov.io/gh/NOAA-FIMS/FIMS/branch/main/graph/badge.svg)](https://app.codecov.io/gh/NOAA-FIMS/FIMS?branch=main)
[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://lifecycle.r-lib.org/articles/stages.html#experimental)

FIMS is an R package for fitting integrated statistical
population-dynamics models to fishery data. The back end is written in
C++, and the use of Rcpp allows users to easily interact with the models
using R. The package is unique because it includes multiple model types,
e.g., surplus production and catch-at-age models. This reduces the
burden of learning a new framework when exploring major changes in model
structure. Stay tuned for the addition of even more model types.

If you are new to FIMS, the best place to start is the [“Introducing
FIMS”](https://noaa-fims.github.io/FIMS/articles/fims-demo.html)
vignette.

## Installation

A [precompiled version of the latest
release](https://noaa-fisheries-integrated-toolbox.r-universe.dev/FIMS)
is available from the instance of [R
universe](https://noaa-fisheries-integrated-toolbox.r-universe.dev)
hosted by NOAA Fisheries Integrated Toolbox. You can install the latest
version from GitHub.

``` r
# Install the pre-compiled version of the latest release
install.packages(
  "FIMS",
  repos = c(
    "https://noaa-fisheries-integrated-toolbox.r-universe.dev",
    "https://cloud.r-project.org"
  )
)

# Compile and install it yourself, where you can specify the branch
pak::pak("NOAA-FIMS/FIMS@main")
```

**Expand this section if you want to use `load_all()`**

Developers who are interested in building FIMS can clone this
repository, e.g., `git clone https://github.com/NOAA-FIMS/FIMS.git` and
build it with
[`devtools::install()`](https://devtools.r-lib.org/reference/install.html),
which builds the package from the cloned files and installs the R
library. Installing with
[`devtools::load_all()`](https://devtools.r-lib.org/reference/load_all.html)
is not recommended because it uses shim files for things like
[`system.file()`](https://rdrr.io/r/base/system.file.html), which can
cause FIMS to not function as expected. For example, if you try to
optimize the same model twice, you will more than likely not get
estimates of uncertainty from the second model run. Additionally,
[`devtools::load_all()`](https://devtools.r-lib.org/reference/load_all.html)
adds the debugger flag `-O0 -g` which leads to Windows users seeing
`Fatal error: can't write <xxx> bytes to section .text of FIMS.o: 'file too big`.
Thus, Windows users need to run
`withr::local_options(pkg.build_extra_flags = FALSE)` at the beginning
of every R session before calling
[`devtools::load_all()`](https://devtools.r-lib.org/reference/load_all.html).
If you want to compile FIMS with the debugger turned on you will need to
run the {withr} function in addition to manually modifying the call to
PKG_CXXFLAGS in the
[src/Makevars.win](https://github.com/NOAA-FIMS/FIMS/blob/main/src/Makevars.win)
file to include `-O1 -g`.

A comprehensive list of all the tools needed to build, test, and
document FIMS can be found
[here](https://github.com/nmfs-ost/on-off-boarding/blob/211d3895afb892ac2fba919935eb27d933e4d048/.github/workflows/onboard-fims.yml#L22).
Additionally, information on best practices regarding how to install
each requirement can be found on the [NOAA Fisheries Office of Science
and Technology on- and off-boarding
repository](https://nmfs-ost.github.io/on-off-boarding/).

## Cheatsheet

[![](https://raw.githubusercontent.com/noaa-fims/fims/main/pkgdown/assets/fims-cheatsheet-thumb.png)](hhttps://github.com/NOAA-FIMS/FIMS/blob/main/pkgdown/assets/fims-cheatsheet.pdf)

## Usage

``` r
library(FIMS)
# Load sample data
data("data_big")
# Prepare data for FIMS model
data_4_model <- FIMSFrame(data_big)

# Create parameters
parameters <- data_4_model |>
  create_default_configurations() |>
  create_default_parameters(data = data_4_model)

# Run the  model with optimization
fit <- parameters |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)
# Clear memory post-run
clear()
```

## Troubleshooting

If you encounter a 🐛, please file an
[Issue](https://github.com/NOAA-FIMS/FIMS/issues) with a minimal
reproducible example on GitHub.

If you are uncertain that you have actually encountered a 🐛, are
looking for more information, or want to talk about some new ideas
please use the [Discussion
Board](https://github.com/orgs/NOAA-FIMS/discussions) to post your
thoughts. Anyone interested in our project is welcome to join the
discussions, this is a place for **all** members of the assessment and
open-source software community to ask and answer questions, share
updates, have open-ended conversations, and follow along. Please
introduce yourself 👋 on the [Introduction
Discussion](https://github.com/orgs/NOAA-FIMS/discussions/801).

More information can be found on the [NOAA-FIMS
website](https://noaa-fims.github.io), particularly on [our
blog](https://noaa-fims.github.io/blog/).

------------------------------------------------------------------------

## Contributors

All contributions to this project are gratefully acknowledged using the
[`allcontributors` package](https://github.com/ropensci/allcontributors)
following the [all-contributors](https://allcontributors.org)
specification. Contributions of any kind are welcome!

### Code

[TABLE]

### Issue Authors

[TABLE]

### Issue Contributors

[TABLE]

## NOAA Disclaimer

This repository is a scientific product and is not official
communication of the National Oceanic and Atmospheric Administration, or
the United States Department of Commerce. All NOAA GitHub project code
is provided on an ‘as is’ basis and the user assumes responsibility for
its use. Any claims against the Department of Commerce or Department of
Commerce bureaus stemming from the use of this GitHub project will be
governed by all applicable Federal law. Any reference to specific
commercial products, processes, or services by service mark, trademark,
manufacturer, or otherwise, does not constitute or imply their
endorsement, recommendation or favoring by the Department of Commerce.
The Department of Commerce seal and logo, or the seal and logo of a DOC
bureau, shall not be used in any manner to imply endorsement of any
commercial product or activity by DOC or the United States Government.

Software code created by U.S. Government employees is not subject to
copyright in the United States (17 U.S.C. section 105). The United
States/Department of Commerce reserve all rights to seek and obtain
copyright protection in countries other than the United States for
Software authored in its entirety by the Department of Commerce. To this
end, the Department of Commerce hereby grants to Recipient a
royalty-free, nonexclusive license to use, copy, and create derivative
works of the Software outside of the United States.

------------------------------------------------------------------------

![NOAA
Fisheries](https://raw.githubusercontent.com/nmfs-general-modeling-tools/nmfspalette/main/man/figures/noaa-fisheries-rgb-2line-horizontal-small.png)

[U.S. Department of Commerce](https://www.commerce.gov/) \| [National
Oceanic and Atmospheric Administration](https://www.noaa.gov) \| [NOAA
Fisheries](https://www.fisheries.noaa.gov/)
