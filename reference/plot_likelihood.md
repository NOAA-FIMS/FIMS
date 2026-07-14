# Plot Likelihood Profile

Creates a visualization of likelihood profile results, displaying the
change in log-likelihood as a function of a fixed parameter value. The
plot shows profiles for different data components and the total
likelihood, helping to assess parameter identifiability and uncertainty.

## Usage

``` r
plot_likelihood(like_fit, group = "label")
```

## Arguments

- like_fit:

  A list returned by
  [`run_fims_likelihood()`](https://NOAA-FIMS.github.io/FIMS/reference/run_fims_likelihood.md)
  containing:

  - `vec` - Vector of parameter values used in the profile

  - `estimates` - Data frame with model estimates and likelihoods for
    each profiled value

- group:

  A character string or vector specifying the grouping variable(s) for
  likelihood components. Default is `"label"` to group by data type.
  Other options could include `c("label", "fleet_name")` for more
  detailed grouping, though fleet names are not currently in FIMS
  output. Must match column name(s) in the estimates data frame

## Value

A ggplot object displaying likelihood profiles. The plot includes:

- Line plots for each data component

- Total likelihood across all components (black solid line)

- Different colors (viridis palette) and line types for components

- X-axis showing the profiled parameter values

- Y-axis showing change in log-likelihood from maximum

- NOAA-themed styling via
  [`stockplotr::theme_noaa()`](https://noaa-fisheries-integrated-toolbox.r-universe.dev/stockplotr/reference/theme_noaa.html)

The plot is also automatically saved as "likelihood.png".

## Details

Plot Likelihood Profile Results

The function generates line plots showing how the log-likelihood changes
relative to its maximum value as a parameter is varied. Each data
component (e.g., landings, index, age composition) is shown as a
separate line, along with the total likelihood across all components.
This reveals which data sources provide information about the parameter
and whether there are conflicting signals.

The y-axis shows the change in log-likelihood from the maximum
(best-fit) value. A parabolic shape indicates a well-identified
parameter with symmetric uncertainty. Flat profiles suggest weak
identifiability, while multiple local minima indicate potential
convergence issues.

The plot automatically saves to "likelihood.png" in the current working
directory in addition to returning a ggplot object that can be further
customized. The total likelihood is shown in black as a solid line,
while individual data components use colors from the viridis palette.

## References

Venzon, D.J. and Moolgavkar, S.H. 1988. A method for computing
profile-likelihood-based confidence intervals. Applied Statistics 37:
87-94.

Hilborn, R. and Mangel, M. 1997. The Ecological Detective: Confronting
Models with Data. Princeton University Press.

## See also

- [`run_fims_likelihood()`](https://NOAA-FIMS.github.io/FIMS/reference/run_fims_likelihood.md)
  for running likelihood profile analysis

Other diagnostic_functions:
[`run_fims_likelihood()`](https://NOAA-FIMS.github.io/FIMS/reference/run_fims_likelihood.md),
[`run_fims_retrospective()`](https://NOAA-FIMS.github.io/FIMS/reference/run_fims_retrospective.md)

## Examples

``` r
if (FALSE) { # \dontrun{
library(FIMS)

# Use built-in dataset from FIMS
data("data_big")
data_4_model <- FIMSFrame(data_big)

# Create parameters object
parameters <- data_4_model |>
  create_default_configurations() |>
  create_default_parameters(data = data_4_model)

# Run base model
base_model <- parameters |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)

# Run likelihood profile
like_fit <- run_fims_likelihood(
  model = base_model,
  parameters = parameters,
  data = data_big,
  parameter_name = "log_rzero",
  n_cores = 3,
  min = -1,
  max = 1,
  length = 21
)

# Plot the likelihood profile
plot_likelihood(like_fit)

# Plot with custom grouping
plot_likelihood(like_fit, group = "label")
} # }
```
