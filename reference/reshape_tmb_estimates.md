# Reshape TMB estimates

This function processes the TMB std and reshapes them into a structured
tibble for easier analysis and manipulation.

## Usage

``` r
reshape_tmb_estimates(
  obj,
  sdreport = NULL,
  opt = NULL,
  parameter_names,
  precomputed_gradient = NULL
)
```

## Arguments

- obj:

  An object returned from
  [`TMB::MakeADFun()`](https://rdrr.io/pkg/TMB/man/MakeADFun.html).

- sdreport:

  An object of the `sdreport` class as returned from
  [`TMB::sdreport()`](https://rdrr.io/pkg/TMB/man/sdreport.html).

- opt:

  An object returned from an optimizer, typically from
  [`stats::nlminb()`](https://rdrr.io/r/stats/nlminb.html), used to fit
  a TMB model. If the model is not optimized, opt is an empty list and
  is not used in the function.

- parameter_names:

  A character vector of parameter names. This is used to identify the
  parameters in the `std` object.

- precomputed_gradient:

  A numeric vector of pre-computed gradient values at the MLE, typically
  from `get_gradient(x)`. When provided, this avoids a second call to
  `obj[["gr"]]()`, which would crash after
  [`clear()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  has freed the C++ memory. If `NULL` (default), the gradient is
  computed directly from `obj[["gr"]](opt[["par"]])`.

## Value

A tibble containing the reshaped estimates (i.e., parameters and derived
quantities).
