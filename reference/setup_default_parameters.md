# Set up default parameters for a FIMS model

Initializing a FIMS model, i.e.,
[`initialize_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_fims.md)
requires a `FIMSFrame` object and a tibble of parameters. The parameter
tibble can be automatically generated using this function or by building
up your own tibble from helper functions used within this function. The
resulting tibble will have all of the necessary parameters, specific to
your data, to run a FIMS model. Initializing a FIMS model, i.e.,
[`initialize_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_fims.md)
requires a `FIMSFrame` object and a tibble of parameters. The parameter
tibble can be automatically generated using this function or by building
up your own tibble from helper functions used within this function. The
resulting tibble will have all of the necessary parameters, specific to
your data, to run a FIMS model.

## Usage

``` r
setup_default_parameters(data)
```

## Arguments

- data:

  A `FIMSFrame` object returned from running
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  on your long input data.

## Value

A `tibble` containing default parameter values and metadata for your
model. Key columns are listed below:

- `module_name`::

  The name of the FIMS module (e.g., "Data", "Selectivity",
  "Recruitment", "Growth", "Maturity"). These entries are always written
  in PascalCase to match the names used in the C++ code.

- `fleet`::

  The name of the fleet the module applies to. This will be `NA` for
  non-fleet-specific modules like "Recruitment".

- `module_type`::

  The specific type of the module (e.g., "Logistic" for a "Selectivity"
  module). This column will always be written in PascalCase to match the
  names used in the C++ code.

- `label`::

  The parameter name (e.g., "inflection_point").

- `age`::

  The age the parameter applies to.

- `length`::

  The length bin the parameter applies to.

- `timing`::

  The timing step (year) the parameter applies to.

- `value`::

  The initial value of the parameter.

- `estimation_type`::

  The estimation type (e.g., "constant", "fixed_effects",
  "random_effects").

- `distribution_type`::

  The type of distribution (e.g., "data", "process"), where a process
  distribution can refer to a fixed effect or a random effect but it
  does not fit to data, e.g., recruitment deviation.

- `distribution`::

  The distribution name (e.g., "Dlnorm", "Dmultinom"). This column will
  always be written in PascalCase to match the names used in the C++
  code.

## Details

The function builds module-specific defaults by calling helper functions
for data, fleet, selectivity, recruitment, maturity, growth, and
population components, then combines those defaults into one tibble. You
can modify the returned tibble before fitting a model (for example,
updating maturity and selectivity parameter values).

To create the default initial numbers at age, this function uses the
defaults from
[`setup_default_Population()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_Population.md)
and
[`setup_default_Recruitment()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_Recruitment.md),
which are passed to `setup_default_init_naa()` to calculate initial
numbers at age.

## See also

- [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)

- [`initialize_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_fims.md)

## Examples

``` r
if (FALSE) { # \dontrun{
# Load the example dataset and create a FIMS data frame
data("data_big")
fims_frame <- FIMSFrame(data_big)

# Set up default parameters
default_parameters <- setup_default_parameters(data = fims_frame)

# Update selectivity parameters for survey1
updated_parameters <- default_parameters |>
  dplyr::rows_update(
    tibble::tibble(
      fleet = "survey1",
      label = c("inflection_point", "slope"),
      value = c(1.5, 2)
    ),
    by = c("fleet", "label")
  )

# Do the same as above except, model fleet1 with double logistic selectivity
# To see required parameters for double logistic selectivity, run
# show(DoubleLogisticSelectivity) and look at the Fields list
parameters_with_double_logistic <- updated_parameters |>
  dplyr::filter(!(fleet == "fleet1" & module_name == "Selectivity")) |>
  dplyr::bind_rows(
    setup_default_Selectivity(
      data = fims_frame,
      fleet = "fleet1",
      module_type = "DoubleLogistic"
    )
  )
} # }
```
