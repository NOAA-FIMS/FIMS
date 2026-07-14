# Modify data for a FIMS Model

Function to remove a given number of years of data and run FIMS model.
This function is called by run_fims_retrospective()

## Usage

``` r
run_modified_data_fims(years_to_remove = 0, data, parameters)
```

## Arguments

- years_to_remove:

  number of years to remove

- data:

  full dataset used in base model run

- parameters:

  input parameters used in base FIMS model

## Value

FIMS model fitted with years of data removed

## Examples

``` r
if (FALSE) { # \dontrun{
library(FIMS)
# Use built-in dataset from FIMS
data("data_big")
data_4_model <- FIMSFrame(data_big)
# Create a parameters object
parameters <- data_4_model |>
  create_default_configurations() |>
  create_default_parameters(data = data_4_model)
# Fit a FIMS model with 1 year of data removed
fit <- run_modified_data_fims(
  years_to_remove = 1,
  data = data_big,
  parameters = parameters
)
} # }
```
