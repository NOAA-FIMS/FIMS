# Set up a new distribution for a data type or a process

Use [`methods::new()`](https://rdrr.io/r/methods/new.html) to set up a
distribution within an existing module with the necessary linkages
between the two. For example, a fleet module will need a distributional
assumption for parts of the data associated with it, which requires the
use of `initialize_data_distribution()`, and a recruitment module, like
the Beverton–Holt stock–recruit relationship, will need a distribution
associated with the recruitment deviations, which requires
`initialize_process_distribution()`.

## Usage

``` r
initialize_data_distribution(
  module,
  family = NULL,
  sd = tibble::tibble(value = 1, estimation_type = "constant"),
  data_type = c("landings", "index", "agecomp", "lengthcomp")
)

initialize_process_distribution(
  module,
  par,
  family = NULL,
  sd = tibble::tibble(value = 1, estimation_type = "constant"),
  is_random_effect = FALSE
)

initialize_process_structure(module, par)
```

## Arguments

- module:

  An identifier to a C++ fleet module that is linked to the data of
  interest.

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

- data_type:

  A string specifying the type of data that the distribution will be fit
  to. Allowable types include c, landings, index, agecomp, lengthcomp
  and the default is c.

- par:

  A string specifying the parameter name the distribution applies to.
  Parameters must be members of the specified module. Use
  `methods::show(module)` to obtain names of parameters within the
  module.

- is_random_effect:

  A boolean indicating whether or not the process is estimated as a
  random effect.

## Value

A reference class. is returned. Use
[`methods::show()`](https://rdrr.io/r/methods/show.html) to view the
various Rcpp class fields, methods, and documentation.

## Examples

``` r
if (FALSE) { # \dontrun{
# Set up a new data distribution
n_years <- 30
# Create a new fleet module
fleet <- methods::new(Fleet)
# Create a distribution for the fleet module
fleet_distribution <- initialize_data_distribution(
  module = fishing_fleet,
  family = lognormal(link = "log"),
  sd = list(
    value = rep(sqrt(log(0.01^2 + 1)), n_years),
    estimation_type = rep("constant", n_years) # Could also be a single "constant"
  ),
  data_type = "index"
)

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
  sd = list(value = 0.4, estimation_type = "constant"),
  is_random_effect = FALSE
)
} # }
```
