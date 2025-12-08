# Create default parameters for a FIMS model

This function generates a Fisheries Integrated Modeling System (FIMS)
model configuration with detailed parameter specifications. This
function takes a high-level configuration `tibble` and generates the
corresponding parameters with default initial values and estimation
settings required to build and run the model.

## Usage

``` r
create_default_parameters(configurations, data)
```

## Arguments

- configurations:

  A tibble of model configurations. Typically created by
  [`create_default_configurations()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_configurations.md).
  Users can modify this tibble to customize the model structure before
  generating default parameters.

- data:

  An S4 object. FIMS input data.

## Value

A `tibble` with default model parameters. The tibble has a nested
structure with the following top-level columns.

- `model_family`::

  The specified model family (e.g., "catch_at_age").

- `module_name`::

  The name of the FIMS module (e.g., "Data", "Selectivity",
  "Recruitment", "Growth", "Maturity").

- `fleet_name`::

  The name of the fleet the module applies to. This will be `NA` for
  non-fleet-specific modules like "Recruitment".

- `data`::

  A list-column containing a `tibble` with detailed parameters.
  Unnesting this column reveals:

  `module_type`:

  :   The specific type of the module (e.g., "Logistic" for a
      "Selectivity" module).

  `label`:

  :   The name of the parameter (e.g., "inflection_point").

  `distribution_link`:

  :   The component the distribution module links to.

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

  :   The type of distribution (e.g., "Data", "process").

  `distribution`:

  :   The name of distribution (e.g., "Dlnorm", `Dmultinom`).

## Details

The function processes the input `configurations` tibble, which defines
the modules for different model components (e.g., `"Selectivity"`,
`"Recruitment"`). For each module specified, it calls internal helper
functions to create a default set of parameters. For example, if a
fleet's selectivity is configured as `"Logistic"`, it generates initial
values for `"inflection_point"` and `"slope"`.

## See also

- [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)

- [`create_default_configurations()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_configurations.md)

## Examples

``` r
if (FALSE) { # \dontrun{
# Load the example dataset and create a FIMS data frame
data("data1")
fims_frame <- FIMSFrame(data1)

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
# show(DoubleLogisticSelectivity)
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
