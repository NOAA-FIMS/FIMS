# Set up a new distribution for a data type

Use [`methods::new()`](https://rdrr.io/r/methods/new.html) to set up a
distribution within an existing module with the necessary linkages
between the two. For example, a fleet module will need a distributional
assumption for parts of the data associated with it, which requires the
use of `initialize_data_distribution()`.

## Usage

``` r
initialize_data_distribution(
  module,
  data_type = c("catch", "index", "age_comp", "length_comp"),
  uncertainty
)
```

## Arguments

- module:

  An identifier to a C++ fleet module that is linked to the data of
  interest.

- data_type:

  A string specifying the type of data that the distribution will be fit
  to. Allowable types include "catch", "index", "age_comp", and
  "length_comp" and the default is catch.

- uncertainty:

  A vector of strings specifying formulas for each data point. See
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  for more information on what the formula should look like.

## Value

A reference class. is returned. Use
[`methods::show()`](https://rdrr.io/r/methods/show.html) to view the
various Rcpp class fields, methods, and documentation.

## See also

- [`initialize_process_distribution()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_process_distribution.md)

## Examples

``` r
if (FALSE) { # \dontrun{
# Set up a new data distribution
n_years <- 30
# Create a new fleet module
fleet <- methods::new(Fleet)
# Create a distribution for the fleet module
sd_log <- rep(sqrt(log(0.01^2 + 1)), n_years)
fleet_distribution <- initialize_data_distribution(
  module = fishing_fleet,
  uncertainty = glue::glue(
    "~dlnorm(meanlog = log_index_expected, sdlog = {sd_log})"
  ),
  data_type = "index"
)
} # }
```
