# Overview

[![FIMS status
badge](https://noaa-fims.r-universe.dev/badges/FIMS)](https://noaa-fims.r-universe.dev/FIMS)
[![R-CMD-check
badge](https://github.com/NOAA-FIMS/FIMS/actions/workflows/call-r-cmd-check.yml/badge.svg?branch=main)](https://github.com/NOAA-FIMS/FIMS/actions/workflows/call-r-cmd-check.yml)
[![Codecov test
coverage](https://codecov.io/gh/NOAA-FIMS/FIMS/branch/main/graph/badge.svg)](https://app.codecov.io/gh/NOAA-FIMS/FIMS?branch=main)
[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://lifecycle.r-lib.org/articles/stages.html#experimental)

The NOAA Fisheries Integrated Modeling System (FIMS) is a software
system designed to support next-generation fisheries stock assessment,
ecosystem, and socioeconomic modeling. A [team of
experts](https://noaa-fims.github.io/about/contributors.html) within
NOAA Fisheries is designing and developing the system, and we are
advised by the [FIMS
Council](https://noaa-fims.github.io/about/contributors.html#fims-council),
which includes academic, industry, and international partners. Our
software is currently operational for estimation purposes and we plan
for it to be operational for management in late 2025.

## Installing FIMS

A [precompiled version of the latest release of
FIMS](https://noaa-fisheries-integrated-toolbox.r-universe.dev/FIMS) is
available from [R
universe](https://noaa-fisheries-integrated-toolbox.r-universe.dev) by
running the following code within R:

``` r
install.packages("FIMS", repos = c("https://noaa-fisheries-integrated-toolbox.r-universe.dev", "https://cloud.r-project.org"))
```

Users can also compile it themselves, like any other TMB package by
running `devtools::install_github("NOAA-FIMS/FIMS")`. This workflow
allows for the specification of a particular branch or tag using the
`ref` argument.

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
[Makevars.win](https://github.com/NOAA-FIMS/FIMS/blob/doc-install/src/Makevars.win)
file in `src` to
`PKG_CXXFLAGS = -DTMB_MODEL -DTMB_EIGEN_DISABLE_WARNINGS -O1 -g`. For a
[list of development tools needed to build, test, and document
FIMS](https://github.com/nmfs-ost/on-off-boarding/blob/211d3895afb892ac2fba919935eb27d933e4d048/.github/workflows/onboard-fims.yml#L22)
see the [on- and off-boarding repository for NOAA Fisheries Office of
Science & Technology](https://github.com/nmfs-ost/on-off-boarding).

## Getting Help

If you encounter a clear 🐛, please file an
[Issue](https://github.com/NOAA-FIMS/FIMS/issues) with a minimal
reproducible example on GitHub. If you are uncertain if you have
actually encountered a 🐛, are looking for more information, or want to
talk about some new ideas please use the [Discussion
Board](https://github.com/orgs/NOAA-FIMS/discussions) to post your
thoughts. Users, developers, lurkers, and anyone else interested in our
project are welcome to join the discussions, this is a place for **all**
members of the assessment and open-source software community to ask and
answer questions, share updates, have open-ended conversations, and
follow along on decisions affecting FIMS. Also, feel free to introduce
yourself 👋 on the [FIMS Discussion
Board](https://github.com/orgs/NOAA-FIMS/discussions/801).

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
Oceanographic and Atmospheric Administration](https://www.noaa.gov) \|
[NOAA Fisheries](https://www.fisheries.noaa.gov/)
