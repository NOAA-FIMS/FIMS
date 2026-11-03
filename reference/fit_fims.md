# Fit a FIMS model (BETA)

Fit a FIMS model (BETA)

## Usage

``` r
fit_fims(
  input,
  get_sd = TRUE,
  save_sd = TRUE,
  number_of_loops = 3,
  optimize = TRUE,
  number_of_newton_steps = 0,
  control = list(eval.max = 10000, iter.max = 10000, trace = 0),
  filename = NULL,
  getReportCovariance = FALSE
)
```

## Arguments

- input:

  Input list as returned by
  [`initialize_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_fims.md).
  An optional `map` entry is a named list of factors for `p` (fixed
  effects) and/or `re` (random effects), in the order of the
  corresponding `parameters` vector. Each factor must have one entry per
  parameter. `NA` holds a parameter at its starting value; repeated
  levels share one estimated value. Shared entries must have identical
  starting values; otherwise, the error lists each parameter's name,
  1-based index in `p` or `re`, and initial value. Factor levels
  determine the reduced parameter order; unused levels are dropped.
  Levels in `p` and `re` are independent; sharing across these vectors
  is not supported.

- get_sd:

  A boolean specifying if the
  [`TMB::sdreport()`](https://rdrr.io/pkg/TMB/man/sdreport.html) should
  be calculated? It is skipped when the maximum gradient is above 0.01
  or not finite, because standard errors far from the optimum are not
  meaningful.

- save_sd:

  A logical, with the default `TRUE`, indicating whether the sdreport is
  returned in the output. If `FALSE`, the slot for the report will be
  empty.

- number_of_loops:

  A positive integer specifying the number of iterations of the
  optimizer that will be performed to improve the gradient. The default
  is three, leading to four total optimization steps.

- optimize:

  Optimize (TRUE, default) or (FALSE) build and return a list containing
  the obj and report slot.

- number_of_newton_steps:

  The number of Newton steps using the inverse Hessian to do after
  optimization. Not yet implemented.

- control:

  A list of optimizer settings passed to
  [`stats::nlminb()`](https://rdrr.io/r/stats/nlminb.html). The the
  default is a list of length three with `eval.max = 1000`,
  `iter.max = 10000`, and `trace = 0`.

- filename:

  Character string giving a file name to save the fitted object as an
  RDS object. Defaults to 'fit.RDS', and a value of NULL indicates not
  to save it. If specified, it must end in .RDS. The file is written to
  folder given by `input[["path"]]`. Not yet implemented.

- getReportCovariance:

  A logical passed to
  [`TMB::sdreport()`](https://rdrr.io/pkg/TMB/man/sdreport.html), with
  the default `FALSE`. Standard errors of all parameters and derived
  quantities are calculated either way. If `TRUE`, the full covariance
  matrix of the derived quantities is also calculated and stored as
  `cov` in the `sdreport`; it grows with the square of the number of
  derived quantities and can use several GB of memory. If `FALSE`, `cov`
  is `NA`.

## Value

An object of class `FIMSFit` is returned, where the structure is the
same regardless if `optimize = TRUE` or not. Uncertainty information is
only included in the `estimates` slot if `get_sd = TRUE`. The result of
each convergence check is stored on the object and returned by
[`get_convergence()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md).

## Details

This function is a beta version still and subject to change without
warning. Mapped parameters are expanded to their full FIMS vectors for
reporting. Shared parameters receive the same estimate and standard
error in
[`get_estimates()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md).
Mapped-off parameters retain their starting values and have `NA`
standard errors. Gradients describe reduced fixed-effect optimization
variables and are reported only on the first full parameter in each
shared level. Parameter counts describe the reduced vectors. With no
active fixed effects, TMB evaluates the random-effects model without
running an outer optimizer. If no parameters remain active, use
`optimize = FALSE`.

## See also

- [`FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFit.md)
