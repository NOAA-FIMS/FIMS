# Compute yardstick metrics for a fitted FIMS model

Extracts observed-vs-expected pairs from a `FIMSFit` object via
[`augment.FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/augment.FIMSFit.md)
and evaluates a
[`yardstick::metric_set()`](https://yardstick.tidymodels.org/reference/metric_set.html)
over them.

## Usage

``` r
get_fit_metrics(
  x,
  metrics = yardstick::metric_set(yardstick::rmse, yardstick::mae, yardstick::rsq),
  group_by = NULL,
  weighted = FALSE,
  ...
)
```

## Arguments

- x:

  A `FIMSFit` object returned from
  [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md).

- metrics:

  A
  [`yardstick::metric_set()`](https://yardstick.tidymodels.org/reference/metric_set.html).
  Defaults to
  `yardstick::metric_set(yardstick::rmse, yardstick::mae, yardstick::rsq)`.
  Any regression metric that accepts `truth` / `estimate` (and
  optionally `case_weights`) can be included.

- group_by:

  A character vector of column names in the augmented tibble to group by
  before computing metrics. Common choices:

  - `"label"` – one row-set per data-stream label

  - `"fleet"` – one row-set per fleet

  - `"distribution"` – one row-set per likelihood distribution

  - `c("label", "fleet")` – per label × fleet combination Defaults to
    `NULL` (no grouping).

- weighted:

  Logical (default `FALSE`). When `TRUE`, inverse-variance weights from
  `uncertainty` are passed to the metric functions via the
  `case_weights` argument. Only metrics that accept `case_weights` will
  use them; others silently ignore the column.

- ...:

  Additional arguments forwarded to
  [`augment.FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/augment.FIMSFit.md).

## Value

A
[`tibble::tibble()`](https://tibble.tidyverse.org/reference/tibble.html)
in the standard yardstick result format: columns `.metric`,
`.estimator`, `.estimate`, plus any grouping columns.

## Details

By default the metrics are computed over **all** data streams combined.
Pass one or more column names to `group_by` to get per-stream breakdowns
(e.g., `group_by = "label"` gives one row per data-stream label such as
`"landings_expected"`, `"age_comp_expected"`, etc.).

## See also

[`augment.FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/augment.FIMSFit.md),
[`yardstick::metric_set()`](https://yardstick.tidymodels.org/reference/metric_set.html)

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

# Overall RMSE / MAE / R² across all data streams
get_fit_metrics(fit)

# Per-data-stream: one row-set per label
# (e.g. "landings_expected", "index_expected", "age_comp_expected")
get_fit_metrics(fit, group_by = "label")

# Per-fleet with a custom metric set
get_fit_metrics(
  fit,
  metrics  = yardstick::metric_set(yardstick::rmse, yardstick::mape),
  group_by = "fleet"
)

# Inverse-variance weighted metrics
get_fit_metrics(fit, weighted = TRUE)
} # }
```
