# Glance at a FIMSFit object — one-row model summary

Returns a single-row tibble of model-level diagnostics following the
[`generics::glance()`](https://generics.r-lib.org/reference/glance.html)
convention. Standard information-criterion columns (`logLik`, `AIC`,
`BIC`, `nobs`) are included alongside FIMS-specific diagnostics
(`max_gradient`, `marginal_nll`, `total_nll`, `converged`,
`terminal_ssb`).

## Usage

``` r
# S3 method for class 'FIMSFit'
glance(x, ...)
```

## Arguments

- x:

  A `FIMSFit` object returned from
  [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md).

- ...:

  Unused; present for S3 method compatibility.

## Value

A
[`tibble::tibble()`](https://tibble.tidyverse.org/reference/tibble.html)
with one row and the following columns:

- `logLik`:

  Marginal log-likelihood at the MLE (`-opt$objective`).

- `AIC`:

  Akaike information criterion (marginal, fixed effects only).

- `BIC`:

  Bayesian information criterion (marginal, fixed effects only).

- `nobs`:

  Total number of data observations (rows with both an observed and an
  expected value in
  [`get_estimates()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)).

- `npar_fixed`:

  Number of fixed-effect parameters.

- `npar_random`:

  Number of random-effect parameters.

- `marginal_nll`:

  Marginal negative log-likelihood (`opt$objective`); `NA` if the model
  was not optimized.

- `total_nll`:

  Total (joint) negative log-likelihood from the TMB report
  (`report$jnll`).

- `max_gradient`:

  Maximum absolute gradient at the MLE. Values below 0.001 are generally
  considered well-converged.

- `converged`:

  Logical; `TRUE` when `opt$convergence == 0` **and**
  `max_gradient < 0.001`.

- `terminal_ssb`:

  Spawning stock biomass at the terminal year, extracted from the TMB
  report. Returns a list-column when multiple populations are present.

- `fims_version`:

  The version of FIMS used to fit the model.

- `runtime_secs`:

  Total wall-clock time of the fit in seconds.

## Information criteria

AIC and BIC are computed from the **marginal** log-likelihood (i.e.
after integrating out random effects), using the number of
**fixed-effect** parameters as \\k\\: \$\$AIC = 2k - 2 \ell_m\$\$
\$\$BIC = k \log(n) - 2 \ell_m\$\$ where \\\ell_m\\ is the marginal
log-likelihood and \\n\\ is the total number of data observations. These
values will be `NA` when `optimize = FALSE` was passed to
[`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md).

## See also

[`tidy.FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/tidy.FIMSFit.md),
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

glance(fit)

# Compare sensitivity runs in one table
dplyr::bind_rows(
  glance(age_only_fit) |> dplyr::mutate(model = "age_only"),
  glance(length_only_fit) |> dplyr::mutate(model = "length_only")
)
} # }
```
