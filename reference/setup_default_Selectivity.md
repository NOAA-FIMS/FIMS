# Set up default selectivity parameters

This function sets up default parameters for a selectivity module.

## Usage

``` r
setup_default_Selectivity(
  data,
  fleet,
  module_type = c("Logistic", "DoubleLogistic", "AgeSpecific")
)
```

## Arguments

- data:

  A `FIMSFrame` object returned from running
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  on your long input data.

- fleet:

  A string specifying the name of the fleet for which selectivity
  parameters are being created.

- module_type:

  A string specifying the desired form of selectivity. Allowable forms
  include Logistic, DoubleLogistic, AgeSpecific and the default is
  Logistic. See
  [`setup_default_parameters`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md)
  for full column descriptions.

## Value

A tibble containing the default selectivity parameters. See
[`setup_default_parameters`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md)
for full column descriptions.

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

- [`setup_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md)

## Examples

``` r
if (FALSE) { # \dontrun{
default_selectivity_parameters <- FIMS:::setup_default_Selectivity(
  data = FIMSFrame(data_big),
  fleet = "fleet1",
  module_type = "Logistic"
)
} # }
```
