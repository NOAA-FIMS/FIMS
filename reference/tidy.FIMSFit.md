# Tidy a FIMSFit object into a parameter-level tibble

Returns one row per estimated parameter following the
[`generics::tidy()`](https://generics.r-lib.org/reference/tidy.html)
convention. Standard columns (`term`, `estimate`, `std.error`,
`statistic`, `p.value`) are always present; FIMS-specific columns
(`module_name`, `module_id`, `estimation_type`, `gradient`) are appended
so the full context is available for filtering and plotting.

## Usage

``` r
# S3 method for class 'FIMSFit'
tidy(
  x,
  parameters = c("fixed_effects", "random_effects"),
  conf.int = FALSE,
  conf.level = 0.95,
  ...
)
```

## Arguments

- x:

  A `FIMSFit` object returned from
  [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md).

- parameters:

  Character vector controlling which `estimation_type` values to
  include. Defaults to `c("fixed_effects", "random_effects")`. Pass
  `"derived_quantity"` to include derived quantities such as spawning
  biomass and expected data values, or pass all three to get every row.

- conf.int:

  Logical (default `FALSE`). When `TRUE`, `conf.low` and `conf.high`
  columns are added using a normal approximation:
  `estimate +/- qnorm((1 + conf.level) / 2) * std.error`.

- conf.level:

  Numeric (default `0.95`). The confidence level used when
  `conf.int = TRUE`.

- ...:

  Unused; present for S3 method compatibility.

## Value

A
[`tibble::tibble()`](https://tibble.tidyverse.org/reference/tibble.html)
with columns:

- `term`:

  Parameter label (from `label` in
  [`get_estimates()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)).

- `estimate`:

  Point estimate at the MLE.

- `std.error`:

  Standard error from
  [`TMB::sdreport()`](https://rdrr.io/pkg/TMB/man/sdreport.html).

- `statistic`:

  Wald z-statistic (`estimate / std.error`).

- `p.value`:

  Two-sided p-value for the z-test.

- `conf.low`, `conf.high`:

  Confidence bounds (only when `conf.int = TRUE`).

- `module_name`:

  Name of the FIMS module (e.g. `"Selectivity"`).

- `module_id`:

  Integer module identifier.

- `estimation_type`:

  One of `"fixed_effects"`, `"random_effects"`, or `"derived_quantity"`.

- `gradient`:

  Gradient of the log-likelihood at the MLE. Values close to zero
  indicate a well-converged parameter.

## Parameter types

FIMS distinguishes three `estimation_type` values:

- `"fixed_effects"`:

  Directly optimized parameters (selectivity, log_Fmort, log_q, etc.).

- `"random_effects"`:

  Integrated-out random effects (log_devs, etc.).

- `"derived_quantity"`:

  Model outputs that are not parameters (spawning biomass, expected
  catches, etc.). Uncertainty here comes from the delta method via
  [`TMB::sdreport()`](https://rdrr.io/pkg/TMB/man/sdreport.html).

Pass any subset of these strings to `parameters` to control which rows
are returned.

## Inference

`statistic` and `p.value` are computed as a two-sided Wald z-test:
`z = estimate / std.error`, `p = 2 * pnorm(-|z|)`. These are
asymptotically valid for fixed effects under regularity conditions;
treat them as approximate for random effects and derived quantities.

## See also

[`glance.FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/glance.FIMSFit.md),
[`get_estimates()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md),
[`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)

## Examples

``` r
if (FALSE) { # \dontrun{
data("data_big")
data_4_model <- FIMSFrame(data_big)

fit <- create_default_parameters(
  configurations = create_default_configurations(data = data_4_model),
  data = data_4_model
) |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)

# Fixed and random effects (default)
tidy(fit)

# Fixed effects only, with 95% confidence intervals
tidy(fit, parameters = "fixed_effects", conf.int = TRUE)

# All rows including derived quantities
tidy(fit, parameters = c("fixed_effects", "random_effects", "derived_quantity"))
} # }
```
