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
  filename = NULL
)
```

## Arguments

- input:

  Input list as returned by
  [`initialize_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_fims.md).

- get_sd:

  A boolean specifying if the
  [`TMB::sdreport()`](https://rdrr.io/pkg/TMB/man/sdreport.html) should
  be calculated?

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

## Value

An object of class `FIMSFit` is returned, where the structure is the
same regardless if `optimize = TRUE` or not. Uncertainty information is
only included in the `estimates` slot if `get_sd = TRUE`.

## Details

This function is a beta version still and subject to change without
warning.

## See also

- [`FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFit.md)
