# Minimal subpopulation model

This implementation is ported onto NOAA-FIMS/FIMS main at `d7608eb`.
The original implementation was extracted from the pre-partition base `28b06d9`.
It introduces explicit biological groups without Cartesian axes,
wildcard selectors, output-demand routing, or a separate sex-mode enum.
An empty group configuration preserves the existing pooled model.

## Run the first two-sex model

```r
library(FIMS)
data <- FIMSFrame(data_big)
parameters <- setup_default_parameters(data = data)
groups <- list(
  list(name = "female", initial_fraction = 0.5,
       recruitment_fraction = 0.5, reproductive_contribution = 1),
  list(name = "male", initial_fraction = 0.5,
       recruitment_fraction = 0.5, reproductive_contribution = 0)
)
input <- initialize_fims(parameters, data, subpopulations = groups)
fit <- fit_fims(input, optimize = FALSE)
get_report(fit)$numbers_at_age_by_subpopulation
get_estimates(fit)
```

For direct Rcpp model construction, call `population$SetTwoSex(0.5, 0.5)` or
`population$SetSubpopulations(groups)` **before** `CreateTMBModel()`.
`GetSubpopulations()` returns configurations in parameter/output order.
`SetSubpopulations(list())` restores pooled configuration before model building.
Rebuild the model after configuration changes.

## State and parameter contract

Each group has its own abundance, natural and total mortality, unfished
abundance, and plus group. Population recruitment is calculated once and
allocated using recruitment fractions. Initial fractions allocate pooled initial
abundance; they need not equal recruitment fractions. Fractions are fixed
configuration, not estimated parameters in this increment.

`log_M` accepts either `n_years * n_ages` shared entries or
`n_subpopulations * n_years * n_ages` group-specific entries. Age varies fastest,
then year, then subpopulation. All entries retain the existing VariableVector
estimation mechanism. In a manually constructed Rcpp model, for example:

```r
population$SetTwoSex(0.5, 0.5)
population$log_M[] <- c(rep(log(0.2), n_years * n_ages),
                      rep(log(0.4), n_years * n_ages))
```

`log_init_naa` similarly accepts `n_ages` pooled entries or
`n_subpopulations * n_ages` separate initial abundances. Initial fractions apply
only to pooled input. The high-level parameter-table helpers still construct
shared biological parameters; use direct Rcpp construction for separate M or
initial abundance in this increment.

Adult group proportions are derived from abundance. The legacy
`proportion_female` input is unused in explicit dynamics and must be constant;
recruitment and initial fractions are configured explicitly instead.

Maturity remains biological maturity for both sexes. Reproductive contribution
is a separate multiplier on mature biomass. Female-only spawning biomass uses
contributions 1 and 0. Unfished initial abundance and spawning biomass per
recruit use each group's first-year M, including the equilibrium plus group.
Later unfished abundance advances with the corresponding year's M.

Pooled abundance, biomass, catch and survey predictions sum group quantities.
Pooled M and Z are abundance-weighted diagnostics; they do not drive survival.
Age compositions are calculated after pooling predicted numbers. Detailed
outputs use `*_by_subpopulation`, and `get_estimates()` supplies
`subpopulation_i` (one-based, in configuration order). Point reports are
available; group-specific standard errors and likelihood links are deferred.

## Deliberate scope

Growth, maturity, fleet selectivity, and catchability are shared across groups.
Fleets attached to explicit populations must have one population owner so group
indices are unambiguous. At least two ages and one modeled year are required.
Groups do not exchange individuals. New groups can be configured without adding
a sex-specific branch to the dynamics loop, but new biological processes still
require explicit implementation.

Next increments: group-specific module references; group-resolved observation
links and uncertainty reporting; configuration names in persisted output;
then transitions for movement or sex change when a concrete model needs them.

## Validation

C++ tests cover entry conservation, invalid inputs, unequal-M sex ratios,
plus groups, independent equilibrium calculations, SBPR consistency, pooled
catch/index sums, equal-biology fished-model equivalence, repeated evaluation,
and a third group. R tests exercise configuration, model construction, TMB,
and output reshaping.

Current NOAA main and explicit subpopulations both initialize unfished abundance
using the equilibrium plus-group geometric series. Equal-biology tests compare
both fished and unfished quantities. The explicit path preserves upstream
support for time-varying maturity (including terminal-year reuse) and fleet
weight-at-age overrides for catches and indices.

The TMB derivative check compares automatic derivatives of reported biological
quantities with a central finite difference for a group-specific M parameter.
Validated on this base: a clean R package installation, 31 C++ population
tests (12 subpopulation tests and 19 existing tests), and the subpopulation,
population-interface, and maturity R test files. The full package test suite
has not been run.
