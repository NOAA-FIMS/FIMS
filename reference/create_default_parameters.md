# Create default parameters for a FIMS model

This function generates a tibble with all of the parameters necessary to
run a FIMS model given the desired high-level configuration that is
specified in `configurations`. The tibble contains default initial
values and estimation settings required to build and run the model. You
can edit the returned tibble if you want to changes things such as
initial values to values more specific to your population before running
your model. For example, the default maturity parameters will need
modified.

## Usage

``` r
create_default_parameters(configurations, data)
```

## Arguments

- configurations:

  A tibble of model configurations. Typically created by
  [`create_default_configurations()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_configurations.md).
  Users can modify this tibble to customize the model structure before
  using it as input to this function.

- data:

  A `FIMSFrame` object returned from running
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  on your long input data.

## Value

A nested `tibble` containing information on parameters for your model
with the same top-level columns as the input tibble passed to
`configurations` but with additional information in the nested `data`
column. See below for more details:

- `model_family`::

  The specified model family (e.g., "catch_at_age").

- `module_name`::

  The name of the FIMS module (e.g., "Data", "Selectivity",
  "Recruitment", "Growth", "Maturity"). These entries are always written
  in PascalCase to match the names used in the C++ code.

- `fleet_name`::

  The name of the fleet the module applies to. This will be `NA` for
  non-fleet-specific modules like "Recruitment".

- `data`::

  A list-column containing a `tibble` with detailed parameters.
  Unnesting this column reveals:

  `module_type`:

  :   The specific type of the module (e.g., "Logistic" for a
      "Selectivity" module). This column will always be written in
      PascalCase to match the names used in the C++ code.

  `label`:

  :   The name of the parameter (e.g., "inflection_point").

  `age`:

  :   The age the parameter applies to.

  `length`:

  :   The length bin the parameter applies to.

  `time`:

  :   The time step (i.e., year) the parameter applies to.

  `value`:

  :   The initial value of the parameter.

  `estimation_type`:

  :   The type of estimation (e.g., "constant", "fixed_effects",
      "random_effects").

  `distribution_type`:

  :   The type of distribution (e.g., "Data", "process"), where a
      process distribution can refer to a fixed effect or a random
      effect but it does not fit to data, e.g., recruitment deviations.

  `distribution`:

  :   The name of distribution (e.g., "Dlnorm", `Dmultinom`). The column
      will always be written in PascalCase to match the names used in
      the C++ code.

## Details

The function processes the `configurations` tibble, which only contains
high-level information for running your model by calling internal helper
functions on each row and returning a multi-row parameter set for each
input row. For example, if a selectivity for the first fleet is
configured as `"Logistic"`, it takes that single row of input
information and returns a parameter set with two rows, one for each
parameter, `"inflection_point"` and `"slope"`.

## See also

- [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)

- [`create_default_configurations()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_configurations.md)

## Examples

``` r
if (FALSE) { # \dontrun{
# Load the example dataset and create a FIMS data frame
data("data_big")
fims_frame <- FIMSFrame(data_big)

# Create default configurations
default_configurations <- create_default_configurations(fims_frame)

# Create default parameters
default_parameters <- create_default_parameters(
  configurations = default_configurations,
  data = fims_frame
) |>
  tidyr::unnest(cols = data)

# Update selectivity parameters for survey1
updated_parameters <- default_parameters |>
  dplyr::rows_update(
    tibble::tibble(
      fleet_name = "survey1",
      label = c("inflection_point", "slope"),
      value = c(1.5, 2)
    ),
    by = c("fleet_name", "label")
  )

# Do the same as above except, model fleet1 with double logistic selectivity
# To see required parameters for double logistic selectivity, run
# show(DoubleLogisticSelectivity) and look at the Fields list
parameters_with_double_logistic <- default_configurations |>
  tidyr::unnest(cols = data) |>
  dplyr::rows_update(
    tibble::tibble(
      module_name = "Selectivity",
      fleet_name = "fleet1",
      module_type = "DoubleLogistic"
    ),
    by = c("module_name", "fleet_name")
  ) |>
  create_default_parameters(
    data = fims_frame
  )
} # }
```
