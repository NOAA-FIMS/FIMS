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
- `TMB` version 1.8.0 or newer (install instructions at are [here](https://github.com/kaskr/adcomp/wiki/Download).)

#### Windows Users

- Rtools4 (available from [here](https://cran.r-project.org/bin/windows/Rtools/rtools40.html))

#### Installing from R

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
