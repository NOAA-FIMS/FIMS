# Set up default recruitment parameters

This function sets up default parameters for a recruitment module.

## Usage

``` r
setup_default_Recruitment(
  data,
  module_type = c("BevertonHolt"),
  distribution = c("Dnorm", NA_character_)
)
```

## Arguments

- data:

  A `FIMSFrame` object returned from running
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  on your long input data.

- module_type:

  A string specifying the type of recruitment model. The available
  options are BevertonHolt.

- distribution:

  A string specifying the distribution for the recruitment process. The
  available options are Dnorm, NA.

## Value

A tibble containing default recruitment parameters. See
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
and `setup_default_Recruitment()`, which are passed to
`setup_default_init_naa()` to calculate initial numbers at age.

## See also

- [`setup_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md)

## Examples

``` r
if (FALSE) { # \dontrun{
default_recruitment_parameters <- setup_default_Recruitment(
  data = FIMSFrame(data_big),
  module_type = "BevertonHolt",
  distribution = "Dnorm"
)
} # }
```
