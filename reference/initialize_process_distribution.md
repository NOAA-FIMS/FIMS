# Set up a new distribution for a process

Use [`methods::new()`](https://rdrr.io/r/methods/new.html) to set up a
distribution within an existing module with the necessary linkages
between the two. For example, a recruitment module, like the
Beverton–Holt stock–recruit relationship, will need a distribution
associated with the recruitment deviations.

## Usage

``` r
initialize_process_distribution(
  module,
  par,
  family = NULL,
  sd = tibble::tibble(value = 1, estimation_type = "fixed_effects")
)

initialize_process_structure(module, par)
```

## Arguments

- module:

  An identifier to a C++ fleet module that is linked to the data of
  interest.

- par:

  A string specifying the parameter name the distribution applies to.
  Parameters must be members of the specified module. Use
  `methods::show(module)` to obtain names of parameters within the
  module.

- family:

  A description of the error distribution and link function to be used
  in the model. The argument takes a family class, e.g.,
  `stats::gaussian(link = "identity")`.

- sd:

  A list of length two. The first entry is named `"value"` and it stores
  the initial values (scalar or vector) for the relevant standard
  deviations. The default is `value = 1`. The second entry is named
  `"estimation_type"` and it stores a vector of booleans (default =
  "constant") is a string indicating whether or not standard deviation
  is estimated as a fixed effect or held constant. If `"value"` is a
  vector and `"estimation_type"` is a scalar, the single value specified
  `"estimation_type"` value will be repeated to match the length of
  `value`. Otherwise, the dimensions of the two must match.

## Value

A reference class. is returned. Use
[`methods::show()`](https://rdrr.io/r/methods/show.html) to view the
various Rcpp class fields, methods, and documentation.

## See also

- [`initialize_data_distribution()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_data_distribution.md)

## Examples

``` r
if (FALSE) { # \dontrun{
# Set up a new process distribution
# Create a new recruitment module
recruitment <- methods::new(BevertonHoltRecruitment)
# view parameter names of the recruitment module
methods::show(BevertonHoltRecruitment)
# Create a distribution for the recruitment module
recruitment_distribution <- initialize_process_distribution(
  module = recruitment,
  par = "log_devs",
  family = gaussian(),
  sd = list(value = 0.4, estimation_type = "constant")
)
} # }
```
