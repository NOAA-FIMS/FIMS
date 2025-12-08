# Get a slot in a FIMSFit object

There is an accessor function for each slot in the S4 class `FIMSFit`,
where the function is named `get_*()` and the star can be replaced with
the slot name, e.g., `get_input()`. These accessor functions are the
preferred way to access objects stored in the available slots.

## Usage

``` r
get_input(x)

# S4 method for class 'FIMSFit'
get_input(x)

get_report(x)

# S4 method for class 'FIMSFit'
get_report(x)

get_obj(x)

# S4 method for class 'FIMSFit'
get_obj(x)

get_opt(x)

# S4 method for class 'FIMSFit'
get_opt(x)

get_max_gradient(x)

# S4 method for class 'FIMSFit'
get_max_gradient(x)

get_sdreport(x)

# S4 method for class 'FIMSFit'
get_sdreport(x)

get_estimates(x)

# S4 method for class 'FIMSFit'
get_estimates(x)

get_number_of_parameters(x)

# S4 method for class 'FIMSFit'
get_number_of_parameters(x)

get_timing(x)

# S4 method for class 'FIMSFit'
get_timing(x)

get_version(x)

# S4 method for class 'FIMSFit'
get_version(x)

get_model_output(x)

# S4 method for class 'FIMSFit'
get_model_output(x)
```

## Arguments

- x:

  Output returned from
  [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md).

## Value

`get_input()` returns the list that was used to fit the FIMS model,
which is the returned object from
[`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md).

`get_report()` returns the TMB report, where anything that is flagged as
reportable in the C++ code is returned.

`get_obj()` returns the output from
[`TMB::MakeADFun()`](https://rdrr.io/pkg/TMB/man/MakeADFun.html).

`get_opt()` returns the output from
[`nlminb()`](https://rdrr.io/r/stats/nlminb.html), which is the
minimizer used in
[`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md).

`get_max_gradient()` returns the maximum gradient found when optimizing
the model.

`get_sdreport()` returns the list from
[`TMB::sdreport()`](https://rdrr.io/pkg/TMB/man/sdreport.html).

`get_estimates()` returns a tibble of parameter values and their
uncertainties from a fitted model.

`get_number_of_parameters()` returns a vector of integers specifying the
number of fixed-effect parameters and the number of random-effect
parameters in the model.

`get_timing()` returns the amount of time it took to run the model in
seconds as a `difftime` object.

`get_version()` returns the `package_version` of FIMS that was used to
fit the model.

`get_model_output()` returns the finalized FIMS output as a JSON list.

## See also

- [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)

- [`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md)
