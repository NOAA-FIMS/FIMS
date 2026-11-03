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
  [`setup_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md).
  It is the primary source of information for what is initialized. That
  is, if a fleet exists in the data but parameter information for how to
  specify selectivity for that fleet is not provided, then selectivity
  will not be initialized for that fleet.

- data:

  An S4 object with the `FIMSFrame` class, which is returned from
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md).
  Passing the data is required because initialization of the modules
  requires passing the data and information regarding the uncertainty of
  that data, i.e., input sample sizes for the multinomial distribution.

## Value

A named list with exactly two top-level elements:

- `parameters`: A named list of numeric starting-value vectors for TMB:

  - `p`: Fixed-effect values returned by
    [`get_fixed()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md).

  - `re`: Random-effect values returned by
    [`get_random()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md).
    This vector is empty if the model has no random effects.

  These vectors follow the initialized C++ parameter order, not
  necessarily the row order of the input `parameters` tibble. They
  contain values on the model's estimation scale (for example, log scale
  for log parameters). Parameters with `estimation_type = "constant"`
  are stored in the modules, not in these vectors. This nested list is
  not the input tibble.

- `model`: The instantiated C++ model module. Currently this is the
  catch-at-age model created with `methods::new(CatchAtAge)`.

The structure is:

    list(
      parameters = list(
        p = get_fixed(),
        re = get_random()
      ),
      model = fims_model
    )

If the returned list is assigned to `initialized`, pass the entire list
as `fit_fims(input = initialized)`. For manual fitting, pass only
`initialized[["parameters"]]` to the `parameters` argument of
[`TMB::MakeADFun()`](https://rdrr.io/pkg/TMB/man/MakeADFun.html), with
`random = "re"` and `DLL = "FIMS"`. The top-level name must be
`parameters`, not `parameter_list`.

No `map` element is returned. To map parameters when using
[`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md),
add `initialized[["map"]]` as a top-level named list of factors with
entries `p` and/or `re`. Each factor must match the length and order of
its corresponding vector in `initialized[["parameters"]]`. Omit an entry
to leave that vector unmapped. See
[`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)
for mapping rules.

It is important that you only have one FIMS model initialized in your R
workspace at a time. Thus, after you initialize and fit the model, you
should run
[`clear()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md).

## See also

- [`setup_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md)

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
parameters_list <- setup_default_parameters(data = data_4_model) |>
  initialize_fims(data = data_4_model)
clear()
} # }
```
