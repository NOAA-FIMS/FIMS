# Run Likelihood Profile

Conducts likelihood profile analysis for a FIMS model by fixing a
specified parameter at a range of values and re-optimizing all other
parameters. This diagnostic tool helps assess parameter uncertainty,
identifiability, and potential local minima in the likelihood surface.

## Usage

``` r
run_fims_likelihood(
  model,
  parameters,
  data,
  module_name = NULL,
  parameter_name = "log_rzero",
  n_cores = NULL,
  min = -2,
  max = 2,
  length = 5
)
```

## Arguments

- model:

  A FIMSFit object returned by
  [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md).
  Used to extract the estimated value of the parameter being profiled

- parameters:

  A FIMS parameters object created by
  [`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md),
  containing the model configuration and initial parameter values

- data:

  A data frame, tibble, or FIMSFrame object containing the model data.
  This should include all data types required by FIMS

- module_name:

  A character string specifying the module containing the parameter to
  profile. Default is `NULL`. Required when the parameter name exists in
  multiple modules (e.g., multiple fleets). Examples include `"fleet1"`,
  `"survey1"`, or `"recruitment"`

- parameter_name:

  A character string specifying the parameter to profile. Default is
  `"log_rzero"`. Must match a parameter name in the FIMS model. Common
  options include `"log_rzero"`, `"log_sigma_recruit"`, `"logit_steep"`,
  or selectivity parameters

- n_cores:

  An integer specifying the number of CPU cores to use for parallel
  processing. If `NULL` (default), uses `parallel::detectCores() - 1`.
  Set to 1 for sequential processing. Must be a positive integer

- min:

  A numeric value specifying the minimum offset from the estimated
  parameter value. Default is `-2`. The profile range starts at
  `estimated_value + min`

- max:

  A numeric value specifying the maximum offset from the estimated
  parameter value. Default is `2`. The profile range ends at
  `estimated_value + max`. Must be greater than `min`

- length:

  An integer specifying the number of points in the likelihood profile.
  Default is `5`. Use an odd number to include the estimated value. Must
  be a positive integer. Values above 50 will generate a warning

## Value

A list with two named elements:

- `vec` - Numeric vector of parameter values used in the profile

- `estimates` - Data frame containing model estimates for each profiled
  value, with the following key columns:

  - `label` - Type of estimate (e.g., "spawning_biomass", "recruitment")

  - `year_i` - Year index for the estimate

  - `age_i` - Age index (if applicable)

  - `estimated` - Point estimate value

  - `uncertainty` - Standard error of the estimate

  - `lpdf` - Log probability density (negative log-likelihood component)

  - `value_{parameter_name}` - The fixed parameter value for this
    profile point

## Details

Run Likelihood Profile for a FIMS Model

Likelihood profiling involves systematically varying a parameter of
interest across a range of values while optimizing all other parameters.
At each fixed value, the negative log-likelihood is recorded, creating a
profile of the likelihood surface. This reveals whether the parameter is
well-estimated (sharp, parabolic profile) or poorly identified (flat
profile), and can identify multiple modes or asymmetric confidence
regions.

The function profiles over values specified relative to the estimated
value from the base model. For example, with `min = -2` and `max = 2`,
the profile spans from 2 units below to 2 units above the estimated
value. The profiled values are evenly spaced on the parameter scale (not
log scale).

Models are run in parallel for computational efficiency. The function
automatically handles parameter identification using module names when
needed to distinguish between multiple instances of the same parameter
type.

## References

Venzon, D.J. and Moolgavkar, S.H. 1988. A method for computing
profile-likelihood-based confidence intervals. Applied Statistics 37:
87-94.

Maunder, M.N. and Punt, A.E. 2013. A review of integrated analysis in
fisheries stock assessment. Fisheries Research 142: 61-74.

## See also

- [`plot_likelihood()`](https://NOAA-FIMS.github.io/FIMS/reference/plot_likelihood.md)
  for visualizing likelihood profiles

- [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)
  for fitting FIMS models

- [`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md)
  for creating parameter objects

Other diagnostic_functions:
[`plot_likelihood()`](https://NOAA-FIMS.github.io/FIMS/reference/plot_likelihood.md),
[`run_fims_retrospective()`](https://NOAA-FIMS.github.io/FIMS/reference/run_fims_retrospective.md)

## Examples

``` r
if (FALSE) { # \dontrun{
library(FIMS)

# Use built-in dataset from FIMS
data("data_big")
data_4_model <- FIMSFrame(data_big)

# Create a parameters object
parameters <- data_4_model |>
  create_default_configurations() |>
  create_default_parameters(data = data_4_model)

# Run the base model with optimization
base_model <- parameters |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)

# Run likelihood profile for log_rzero
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

# View the profiled parameter values
like_fit$vec

# View structure of estimates
head(like_fit$estimates)
} # }
```
