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
initial <- exp(parameters$value[parameters$label == "log_init_naa"])
groups <- list(
  list(name = "female", init_naa = initial / 2,
       recruitment_fraction = 0.5, reproductive_contribution = 1),
  list(name = "male", init_naa = initial / 2,
       recruitment_fraction = 0.5, reproductive_contribution = 0)
)
input <- initialize_fims(parameters, data, subpopulations = groups)
fit <- fit_fims(input, optimize = FALSE)
get_report(fit)$numbers_at_age_by_subpopulation
get_estimates(fit)
```

For direct Rcpp model construction, call `population$SetTwoSex(female_init_naa, male_init_naa, 0.5)` or
`population$SetSubpopulations(groups)` **before** `CreateTMBModel()`.
`GetSubpopulations()` returns configurations in parameter/output order.
`SetSubpopulations(list())` restores pooled configuration before model building;
when switching from explicit groups, also reset `population$log_init_naa` to
the desired pooled age vector.
Rebuild the model after configuration changes.

## State and parameter contract

Each group has its own abundance, natural and total mortality, unfished
abundance, and plus group. Population recruitment is calculated once and
allocated using fixed recruitment fractions. Each subpopulation owns its own
`log_init_naa` vector, with one value per modeled age. Initial abundance is not
allocated from a shared total, and there is no `initial_fraction` setting.
Initial and recruitment age/sex structures are independent.

`log_M` accepts either `n_years * n_ages` shared entries or
`n_subpopulations * n_years * n_ages` group-specific entries. Age varies fastest,
then year, then subpopulation. All entries retain the existing VariableVector
estimation mechanism. In a manually constructed Rcpp model, for example:

```r
population$SetTwoSex(initial / 2, initial / 2, 0.5)
population$log_M[] <- c(rep(log(0.2), n_years * n_ages),
                      rep(log(0.4), n_years * n_ages))
```

In R, provide each group's initial abundance using either:

- `init_naa`: a strictly positive numeric vector in modeled-age order. It is
  constant by default. Set `init_estimation_type` to `"fixed_effects"`,
  `"random_effects"`, or `"constant"`, either once or once per age.
- `log_init_naa`: a parameter tibble containing `value` (natural-log abundance)
  and `estimation_type`, ordered by modeled age. Supply only the initial-abundance
  rows for that group.

For example, to estimate female initial abundance:

```r
groups[[1]]$init_naa <- NULL
groups[[1]]$log_init_naa <- tibble::tibble(
  value = log(initial / 2),
  estimation_type = "fixed_effects"
)
```

Separate initial abundances can have different age patterns. Their sum determines
pooled initial abundance. One explicit group uses the same interface and requires
`recruitment_fraction = 1`; it receives exactly its supplied initial numbers.
Zero or nonfinite initial abundance is rejected because parameters use a log
scale. Random effects still require an appropriate distribution model.

The population's pooled initial parameter rows are replaced by the group's
parameters during high-level setup, so unused pooled initial parameters are not
estimated. Internally, the Rcpp parameter adapter and output serialization use a
flattened group/age view for compatibility; the model registers parameters from
each group's own vector. `get_estimates()` labels these with `subpopulation_i`,
including for one group. No automatic constraints tie estimated groups together.

The high-level parameter-table helpers still construct shared natural mortality;
use direct Rcpp construction for separate M in this increment. Adult group
proportions are derived from abundance. The legacy `proportion_female` input is
unused in explicit dynamics and must be constant.

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
