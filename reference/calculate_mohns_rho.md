# Calculate Mohn's rho for retrospective analysis

Calculates Mohn's rho, a diagnostic statistic that summarizes the bias
in retrospective analysis. The statistic measures the average relative
difference between estimates from retrospective peels and the reference
model at the corresponding terminal year of each peel. A value close to
zero suggests minimal retrospective bias, while larger absolute values
indicate potential model misspecification or data conflicts.

The formula for Mohn's rho is: \$\$\rho = \frac{1}{n} \sum\_{i=1}^{n}
\frac{x\_{peel,i} - x\_{base,i}}{x\_{base,i}}\$\$ where n is the number
of retrospective peels, \\x\_{peel,i}\\ is the estimated value from peel
i at its terminal year, and \\x\_{base,i}\\ is the value from the base
model at that same year.

## Usage

``` r
calculate_mohns_rho(retro_fit, quantity)
```

## Arguments

- retro_fit:

  List returned by the
  [`run_fims_retrospective()`](https://NOAA-FIMS.github.io/FIMS/reference/run_fims_retrospective.md)
  function containing `years_to_remove` (vector of years peeled) and
  `estimates` (data frame with retrospective results)

- quantity:

  Character string specifying which quantity to calculate Mohn's rho for
  (e.g., "spawning_biomass", "expected_recruitment")

## Value

A numeric value representing Mohn's rho. Values close to zero indicate
minimal retrospective bias. Positive values suggest the model
retrospectively underestimates the quantity, while negative values
suggest overestimation.

## Details

Calculate Mohn's rho for retrospective analysis

## References

Mohn (1999). The retrospective problem in sequential population
analysis: An investigation using cod fishery and simulated data. ICES
Journal of Marine Science, 56(4), 473-488.
https://doi.org/10.1006/jmsc.1999.0481.

Hurtado-Ferro et al. (2015). Looking in the rear-view mirror: bias and
retrospective patterns in integrated, age-structured stock assessment
models. ICES Journal of Marine Science, 72(1), 99-110.
https://doi.org/10.1093/icesjms/fsu198.

## Examples

``` r
if (FALSE) { # \dontrun{
library(FIMS)
# Use built-in dataset from FIMS
data("data_big")
# Prepare data for FIMS model
data_4_model <- FIMSFrame(data_big)
# Create parameters
parameters <- data_4_model |>
  create_default_configurations() |>
  create_default_parameters(data = data_4_model)

# Run retrospective analysis
retro_fit <- run_fims_retrospective(
  years_to_remove = 0:5,
  data = data_big,
  parameters = parameters,
  n_cores = 1
)

# Calculate Mohn's rho for spawning biomass
rho_ssb <- calculate_mohns_rho(retro_fit, quantity = "spawning_biomass")

# Calculate Mohn's rho for recruitment
rho_rec <- calculate_mohns_rho(retro_fit, quantity = "expected_recruitment")
} # }
```
