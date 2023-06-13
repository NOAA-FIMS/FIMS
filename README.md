  <!-- badges: start -->
  [![R-CMD-check](https://github.com/NOAA-FIMS/FIMS/workflows/call-r-cmd-check/badge.svg)](https://github.com/NOAA-FIMS/FIMS/actions)
  [![Codecov test coverage](https://codecov.io/gh/NOAA-FIMS/FIMS/branch/main/graph/badge.svg)](https://app.codecov.io/gh/NOAA-FIMS/FIMS?branch=main)
  [![Lifecycle: experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://lifecycle.r-lib.org/articles/stages.html#experimental)
  <!-- badges: end -->

# Overview
The repository for development of FIMS. 

The NOAA Fisheries Integrated Modeling System is a software system designed and architected to support next-generation fisheries stock assessment, ecosystem, and socioeconomic modeling. A team of experts within NOAA Fisheries is designing and developing the system, and we are advised by the FIMS Council which includes academic, industry, and international partners. The roles of internal and external collaborators are outlined in the [governance section](https://noaa-fims.github.io/collaborative_workflow/fims-governance.html) of our developer guide. We plan to have an operational software system that is released to the public in 2023. In the meantime, users and developers are welcome to submit feedback using Github issues. Please use the issues under [collaborative workflow](https://github.com/NOAA-FIMS/collaborative_workflow/issues) to make suggestions about the [developer guide](https://noaa-fims.github.io/collaborative_workflow/) and the issues under the FIMS software [repo](https://github.com/NOAA-FIMS/FIMS/issues) for software design and development feedback. You can follow the team discussion [here](https://github.com/NOAA-FIMS/FIMS/discussions).

## Installation

The following software is required:

- R version 4.0.0 or newer (or RStudio version 1.2.5042 or newer)
- the `remotes` R package
- `TMB` version 1.8.0 or newer (install instructions are [here](https://github.com/kaskr/adcomp/wiki/Download).)

### Windows Users

- Rtools4 (available from [here](https://cran.r-project.org/bin/windows/Rtools/rtools40.html))

#### Fixing Fatal Error

Windows users can expect to see some derivative of the following error message in their R session if they have not yet set some flags using {withr}.
```
Fatal error: can't write <xxx> bytes to section .text of FIMS.o: 'file too big
```
You can easily fix this by running the following line in your local R session. Note that this call will need to be repeated each time you open a new session.
```
withr::local_options(pkg.build_extra_flags = FALSE)
```
This fix does not work when `devtools::test()` is called and FIMS is re-compiled. The call to `devtools::test()` in this case overwrites the local options set by withr. Compile FIMS first using `devtools::load_all()` prior to running `devtools::test()`.

This fix removes the debugger flag `-O0 -g` from being automatically inserted for certain devtools calls (e.g. `devtools::load_all()`). Windows developers wanting to compile FIMS with the debugger turned on will need to run the above script in addition to manually modifying the call to PKG_CXXFLAGS in the [Makevars.win](https://github.com/NOAA-FIMS/FIMS/blob/doc-install/src/Makevars.win) file in the src directory to the following:

```
PKG_CXXFLAGS =  -DTMB_MODEL  -DTMB_EIGEN_DISABLE_WARNINGS -O1 -g
```
To turn off the debugger flag, remove the `-O1 -g` flag:
```
PKG_CXXFLAGS =  -DTMB_MODEL  -DTMB_EIGEN_DISABLE_WARNINGS
```

### Installing from R

```
remotes::install_github("NOAA-FIMS/FIMS")
library(FIMS)
```

## Getting Help
Please report bugs along with a minimal reproducible example on github [issues](https://github.com/NOAA-FIMS/FIMS/issues)

## Code of Conduct
All contributors participating and contributing to the FIMS project are expected to adhere to the [FIMS Code of Conduct](https://noaa-fims.github.io/collaborative_workflow/code-of-conduct.html#code-of-conduct)

****************************

## NOAA Disclaimer

This repository is a scientific product and is not official communication of the National Oceanic and Atmospheric Administration, or the United States Department of Commerce. All NOAA GitHub project code is provided on an 'as is' basis and the user assumes responsibility for its use. Any claims against the Department of Commerce or Department of Commerce bureaus stemming from the use of this GitHub project will be governed by all applicable Federal law. Any reference to specific commercial products, processes, or services by service mark, trademark, manufacturer, or otherwise, does not constitute or imply their endorsement, recommendation or favoring by the Department of Commerce. The Department of Commerce seal and logo, or the seal and logo of a DOC bureau, shall not be used in any manner to imply endorsement of any commercial product or activity by DOC or the United States Government.

Software code created by U.S. Government employees is not subject to copyright in the United States (17 U.S.C. §105). The United States/Department of Commerce reserve all rights to seek and obtain copyright protection in countries other than the United States for Software authored in its entirety by the Department of Commerce. To this end, the Department of Commerce hereby grants to Recipient a royalty-free, nonexclusive license to use, copy, and create derivative works of the Software outside of the United States.

****************************

<img src="https://raw.githubusercontent.com/nmfs-general-modeling-tools/nmfspalette/main/man/figures/noaa-fisheries-rgb-2line-horizontal-small.png" height="75" alt="NOAA Fisheries">

[U.S. Department of Commerce](https://www.commerce.gov/) | [National Oceanographic and Atmospheric Administration](https://www.noaa.gov) | [NOAA Fisheries](https://www.fisheries.noaa.gov/)
