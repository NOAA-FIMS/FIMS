# Set up default maturity parameters

This function sets up default parameters for a maturity module.

## Usage

``` r
setup_default_Maturity(data, module_type = c("Logistic"))
```

## Arguments

- data:

  A `FIMSFrame` object returned from running
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  on your long input data.

- module_type:

  A string specifying the type of maturity module. The available options
  are Logistic.

## Value

A tibble containing default maturity parameters. See
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
default_maturity_parameters <- setup_default_Maturity(
  data = FIMSFrame(data_big),
  module_type = "Logistic"
)
} # }
```
