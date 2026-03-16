# Initialize C++ modules via Rcpp for a FIMS model

This function uses information from a parameter data frame that stores
the model specifications and a`FIMSFrame` object that stores the data to
instantiate, i.e., create an instance of a class, the required C++
modules. Several C++ modules are needed to run a FIMS model and the
required modules will be different for each model type. For example, for
a catch-at-age model one needs to instantiate recruitment, growth, and
maturity modules and at least one fleet and population module.

## Usage

``` r
initialize_fims(parameters, data)
```

## Arguments

- parameters:

  A tibble returned from
  [`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md).
  The tibble can be nested, i.e., contain a data column, or unnested,
  i.e., `tidyr::unnest(create_default_parameters(), cols = "data")`.
  Regardless, it is the primary source of information for what is
  initialized. That is, if a fleet exists in the data but parameter
  information for how to specify selectivity for that fleet is not
  provided, then selectivity will not be initialized for that fleet.

- data:

  An S4 object with the `FIMSFrame` class, which is returned from
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md).
  Passing the data is required because initialization of the modules
  requires passing the data and information regarding the uncertainty of
  that data, i.e., input sample sizes for the multinomial distribution.

## Value

A list is returned with two elements, `parameters` and `model`. The list
can be passed to the `input` argument of
[`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)
to fit the model. The first element of the list can also be passed to
the `parameters` argument of
[`TMB::MakeADFun()`](https://rdrr.io/pkg/TMB/man/MakeADFun.html) if you
wish to have more control over the model-fitting process. The model
element of the returned list stores the instantiated C++ model module,
e.g., the results of `methods::new(CatchAtAge)` for a catch-at-age
model. It is important that you only have one FIMS model initialized in
your R workspace at a time. Thus, after you initialize and fit the
model, you should run
[`clear()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md).

## See also

- [`create_default_configurations()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_configurations.md)

- [`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md)

- [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)

- [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)

- [`clear()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)

## Examples

``` r
if (FALSE) { # \dontrun{
# Prepare data for FIMS model
data("data_big", package = "FIMS")
data_4_model <- FIMSFrame(data_big)
# Instantiate modules
parameters_list <- data_4_model |>
  create_default_configurations() |>
  create_default_parameters(data = data_4_model) |>
  initialize_fims(data = data_4_model)
clear()
} # }
```
