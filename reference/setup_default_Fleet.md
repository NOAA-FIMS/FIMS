# Set up default fleet parameters

This function sets up default parameters for a fleet module, including
natural log of catchability coefficients (log_q) and fishing mortality
(log_Fmort) for fleets. For fishing fleets, log_q is set to an
estimation type of "constant" with a default value of 0, while log_Fmort
is set to "fixed_effects". For survey fleets, log_q is set to
"fixed_effects" and log_Fmort is set to "constant" with a default value
of -200.

## Usage

``` r
setup_default_Fleet(data, fleet)
```

## Arguments

- data:

  A `FIMSFrame` object returned from running
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  on your long input data.

- fleet:

  A character. Name of the fleet.

## Value

A tibble containing the default fleet parameters. See
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
default_fleet_parameters <- setup_default_Fleet(
  data = FIMSFrame(data_big),
  fleet = "fleet1"
)
} # }
```
