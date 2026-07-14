# Modify Parameters of a FIMS Model

Modify a parameter input and run a FIMS model. This function is called
by run_fims_likelihood()

## Usage

``` r
run_modified_pars_fims(
  new_value,
  parameter_name,
  module_name = NULL,
  parameters,
  data
)
```

## Arguments

- new_value:

  The new value to be changed in the FIMS model.

- parameter_name:

  A string specifying the parameter name to modify. This should match a
  value in the `label` column of the parameters tibble.

- module_name:

  The name of module associated with the parameter to be changed.
  Default is NULL.

- parameters:

  The tibble of input parameters for a FIMS model

- data:

  A dataframe of input data for FIMS model

## Value

FIMS model fitted to the new parameter input value

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
fit <- run_modified_pars_fims(
  new_value = 12.9,
  parameter_name = "log_rzero",
  parameters = parameters, data = data_big
)
} # }
```
