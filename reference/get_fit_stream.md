# Extract a single data stream from a FIMSFit augmented tibble

Convenience filter to pull out one specific data stream (e.g. the
landings for a given module) so you can pass it directly to any
yardstick metric or plot it.

## Usage

``` r
get_fit_stream(x, stream_label = NULL, module_id = NULL, ...)
```

## Arguments

- x:

  A `FIMSFit` object **or** an already-augmented tibble from
  [`augment.FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/augment.FIMSFit.md).

- stream_label:

  Character scalar. The value of the `label` column to retain, e.g.,
  `"landings_expected"`, `"index_expected"`, `"agecomp_expected"`, or
  `"lengthcomp_expected"`. If `NULL` (default), no filtering on label is
  done.

- module_id:

  Integer scalar. The `module_id` of the fleet or survey to retain
  (e.g., `1` for the first fishing fleet, `2` for the first survey in
  `data_big`). If `NULL` (default), all modules are included.

- ...:

  Forwarded to
  [`augment.FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/augment.FIMSFit.md)
  when `x` is a `FIMSFit`.

## Value

A
[`tibble::tibble()`](https://tibble.tidyverse.org/reference/tibble.html)
subset of the augmented data.

## Details

In the FIMS output the `fleet` column is `NA` for derived-quantity rows
(which is where all observed/expected pairs live). Use `module_id`
instead to distinguish fleets and surveys - this matches the convention
used in the FIMS vignettes, where `module_id == 1` is the first fishing
fleet and `module_id == 2` is the first survey. To discover which
`module_id` values are present in your fit, inspect `augment(fit)`
directly.

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

# Landings for the fishing fleet (module_id 1) - compute RMSE
get_fit_stream(fit, stream_label = "landings_expected", module_id = 1) |>
  yardstick::rmse(truth = .truth, estimate = .pred)

# Survey index stream (module_id 2 in the data_big example)
get_fit_stream(fit, stream_label = "index_expected", module_id = 2)

# All streams for fleet 1 (landings + age comp + length comp)
get_fit_stream(fit, module_id = 1)
} # }
```
