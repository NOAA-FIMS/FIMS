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
shared natural mortality and initial abundance; use direct Rcpp construction for separate M or
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

## Biological module overrides

Each group can override growth, maturity, and selectivity for selected fleets.
Omitted references inherit the population's growth/maturity or the fleet's
selectivity. The same module can be referenced by more than one group in direct
Rcpp construction. Maturity remains separate from reproductive contribution.

With `initialize_fims()`, provide optional `growth` and `maturity` lists with a
`parameters` tibble and/or a `data` FIMSFrame. Omitted inputs use the shared
population inputs. Override data must have the same modeled ages and years.
EWAA growth also accepts a `weights` vector of length `n_ages` or
`(n_years + 1) * n_ages`, in year/age order and the same units as the model.
Use `selectivity = list(fleet_name = list(parameters = ...))` for fleet-specific
overrides; unspecified fleets retain their shared selectivity. For example:

```r
female_maturity <- parameters |>
  dplyr::filter(.data$module_name == "Maturity") |>
  dplyr::mutate(
    value = dplyr::if_else(.data$label == "inflection_point", 3, 0.7),
    estimation_type = "constant"
  )
groups[[1]]$maturity <- list(parameters = female_maturity)
groups[[1]]$growth <- list(weights = 0.1 * get_ages(data)^2)
input <- initialize_fims(parameters, data, subpopulations = groups)
```

These are synthetic values for exercising the interface. A complete runnable
example, including selectivity and checks of biomass/spawning-biomass sums, is
in `inst/examples/two-sex-subpopulations.R`.

For direct Rcpp construction, add `growth_id`, `maturity_id`, and/or
`selectivity_ids` to a group's configuration before `SetSubpopulations()`.
`selectivity_ids` is a numeric vector named by fleet IDs:

```r
groups <- population$GetSubpopulations()
groups[[1]]$growth_id <- female_growth$get_id()
groups[[1]]$maturity_id <- female_maturity$get_id()
groups[[1]]$selectivity_ids <- stats::setNames(
  female_selectivity$get_id(), as.character(fleet$get_id())
)
population$SetSubpopulations(groups)
```

`initialize_fims()` accepts configurations rather than IDs because it clears
and rebuilds the registry. Additional modules use the ordinary VariableVector
estimation mechanism. Their parameters affect the pooled likelihood through
the group's predictions. Set unused shared-module parameters to constant when
all groups override a shared module; those parameters no longer inform the fit.

The dynamics use each group's own weight, maturity and total fishing mortality
for survival, equilibrium SBPR, biomass, catches and survey predictions. Fleet
weight-at-age overrides still take precedence for observation weights only.
Pooled maturity, M, F and Z are abundance-weighted diagnostics. Group-resolved
`weight_at_age_by_subpopulation`, `proportion_mature_at_age_by_subpopulation`,
and `mortality_F_by_subpopulation` reports expose the biological inputs and F.
Cached derived-growth products are refreshed at each model evaluation.

## Deliberate scope

Catchability is shared across groups.
Fleets attached to explicit populations must have one population owner so group
indices are unambiguous. At least two ages and one modeled year are required.
Groups do not exchange individuals. New groups can be configured without adding
a sex-specific branch to the dynamics loop, but new biological processes still
require explicit implementation.

Age-to-length mappings remain shared. Fixed mappings may be used with group
biology. A growth override together with an active growth-derived mapping for
length observations is rejected at model construction, because that combination
needs group-specific length distributions. Biological weights from derived
growth are supported when a group-specific derived length mapping is not needed.

Next increments: group-resolved observation links and uncertainty reporting;
group-specific derived age-to-length mappings; configuration names in persisted
output; then transitions for movement or sex change when a concrete model needs
them.

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
Tests also cover mixed growth, mortality, maturity and selectivity; shared-module
fallback and invalid references; derived-growth cache refresh; and derivatives
through growth, maturity and selectivity overrides. The full package test suite
has not been run.
