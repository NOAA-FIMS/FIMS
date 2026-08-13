# Set up default population parameters

This function sets up default parameters for a population module.

## Usage

``` r
setup_default_Population(
  data,
  log_M = log(0.2),
  log_init_naa = NA_real_,
  proportion_female = 0.5
)
```

## Arguments

- data:

  A `FIMSFrame` object returned from running
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  on your long input data.

- log_M:

  A numeric value or vector (length equal to the number of ages \*
  number of years) specifying the log natural mortality rate. Default is
  `log(0.2)`.

- log_init_naa:

  A numeric value or vector (length equal to the number of ages)
  specifying the log initial numbers at age. Default is `NA_real_`.

- proportion_female:

  A numeric value between 0 and 1 specifying the proportion of females.
  Default is `0.5`.

## Value

A tibble of default population configurations, including initial numbers
at age, natural mortality rate, and proportion of females. See
[`setup_default_parameters`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md)
for full column descriptions.

## Details

The function creates a tibble of default population configurations,
including log natural mortality rate, log initial numbers at age, and
proportion of females. The function performs input checks to ensure that
the provided arguments are valid.

## See also

- [`setup_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md)

## Examples

``` r
if (FALSE) { # \dontrun{
default_population_parameters <- setup_default_Population(data = FIMSFrame(data_big))
} # }
```
