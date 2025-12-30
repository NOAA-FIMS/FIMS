# Minimal Example for Running a FIMS Model

## Introduction

This vignette provides the minimal steps needed to run a Fisheries
Integrated Modeling System (FIMS) model.

## Setup

Load the FIMS package and clear memory:

``` r
library(FIMS)

# clear memory
clear()
```

## Data and parameters

First, load the available data and create a FIMS data frame to create a
model with. Then create a set of default parameters.

``` r
# Load sample data
data("data1")
# Prepare data for FIMS model
data_4_model <- FIMSFrame(data1)

# Create parameters
parameters <- data_4_model |>
  create_default_configurations() |>
  create_default_parameters(data = data_4_model)
```

## Initialize and Fit

Initialize the FIMS modules and fit the model.

``` r
# Run the  model with optimization
fit <- parameters |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)
```

    ## ✔ Starting optimization ...
    ## ℹ Restarting optimizer 3 times to improve gradient.
    ## ℹ Maximum gradient went from 0.00557 to 0.00109 after 3 steps.
    ## ✔ Finished optimization
    ## ✔ Finished sdreport
    ## ℹ FIMS model version: 0.7.1
    ## ℹ Total run time was 1.23189 minutes
    ## ℹ Number of parameters: fixed_effects=77, random_effects=0, and total=77
    ## ℹ Maximum gradient= 0.00109
    ## ℹ Negative log likelihood (NLL):
    ## • Marginal NLL= 3240.1261
    ## • Total NLL= 3240.1261
    ## ℹ Terminal SB= 1803.2724

``` r
# Clear memory post-run
clear()
```
