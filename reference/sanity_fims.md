# Check whether a fitted FIMS model is ready to use

Prints the result of each convergence check that
[`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)
stored on the fit, similar to `sdmTMB::sanity()`. The checks and
thresholds are the ones
[`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)
warns with:

- `optimizer`: the nlminb convergence code is 0.

- `max_gradient`: the maximum absolute gradient is below 0.001. It fails
  above 0.01, and then sdreport is skipped.

- `nll`: the objective, the total negative log-likelihood, and each
  negative log-likelihood component are finite.

- `hessian`: the Hessian is positive definite.

- `standard_errors`: no standard error is NA.

- `large_standard_errors`: no fixed or random effect has a standard
  error above 1000.

- `condition_number`: the condition number of the Hessian is at or below
  1e5.

The checks are read from the fit rather than recomputed, so this works
after
[`clear()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md).

## Usage

``` r
sanity_fims(fit)
```

## Arguments

- fit:

  An object of class `FIMSFit` returned by
  [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md).

## Value

Invisibly, a list with `all_ok`, which is `TRUE` when every check is
`"OK"`, and `checks`, the tibble returned by
[`get_convergence()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md).

## Examples

``` r
if (FALSE) { # \dontrun{
data("data_big")
data_4_model <- FIMSFrame(data_big)
fit <- setup_default_parameters(data = data_4_model) |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)
sanity <- sanity_fims(fit)
sanity[["all_ok"]]
} # }
```
