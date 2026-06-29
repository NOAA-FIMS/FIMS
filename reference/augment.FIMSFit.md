# Augment a FIMSFit object for use with yardstick

Returns a tidy tibble of observed-vs-expected pairs drawn from the
output of
[`get_estimates()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md).
Only rows that have both an observed value and a model-expected value
are included (i.e. data-likelihood rows), so parameter rows without data
observations are automatically dropped.

## Usage

``` r
# S3 method for class 'FIMSFit'
augment(x, include_weights = TRUE, ...)
```

## Arguments

- x:

  A `FIMSFit` object returned from
  [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md).

- include_weights:

  Logical (default `TRUE`). When `TRUE` and `uncertainty` is available,
  a `.weight` column is added equal to `1 / uncertainty^2`
  (inverse-variance weights). Rows where `uncertainty` is `NA` or zero
  receive `NA` weights, which yardstick silently drops when calling
  weighted metrics.

- ...:

  Unused; present for S3 method compatibility.

## Value

A
[`tibble::tibble()`](https://tibble.tidyverse.org/reference/tibble.html)
with at least the columns `.truth`, `.pred`, and optional `.weight`,
plus grouping-metadata columns.

## Details

The returned tibble follows the conventions expected by every
[`yardstick::metric_set()`](https://yardstick.tidymodels.org/reference/metric_set.html)
metric function:

|                |                                                        |
|----------------|--------------------------------------------------------|
| Column         | Role                                                   |
| `.truth`       | Observed data value (maps from `observed`)             |
| `.pred`        | Model-expected value (maps from `expected`)            |
| `.weight`      | Inverse-variance weight from `uncertainty` (optional)  |
| `label`        | Parameter / quantity label, e.g. `"landings_expected"` |
| `fleet`        | Fleet identifier (integer)                             |
| `module_id`    | Unique module identifier                               |
| `distribution` | Likelihood distribution used for this data stream      |
| `year_i`       | Year index (present when available in the estimates)   |
| `age_i`        | Age index (present when available in the estimates)    |

## See also

[`get_fit_metrics()`](https://NOAA-FIMS.github.io/FIMS/reference/get_fit_metrics.md),
[`get_estimates()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md),
[`yardstick::metrics()`](https://yardstick.tidymodels.org/reference/metrics.html)

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

# Tidy tibble of observed vs. expected, ready for any yardstick metric
augment(fit)
} # }
```
