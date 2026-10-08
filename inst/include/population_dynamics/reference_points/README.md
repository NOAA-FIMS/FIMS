# Reference points

This module calculates equilibrium quantities per annual recruit and solves
for fishing mortality at a specified spawning potential ratio (SPR). It is a
C++ module with R wrappers and a CatchAtAge adapter. Callers can supply a fixed
biological snapshot or extract one from a live model. Beverton-Holt equilibrium
recruitment and bounded MSY calculations use the same per-recruit engine.

## Conventions

- Each input vector has one entry per annual age, youngest first. Scalars must
  be expanded by the caller. Select a reference year or prepare an averaged
  snapshot before calling the calculator.
- One recruit enters the first age each year. The final age is a plus group;
  a one-age model consists entirely of that plus group.
- Natural mortality is a finite, strictly positive annual instantaneous rate.
  Weights and selectivities are finite and nonnegative. Maturity and female
  proportions are in [0, 1]. A numerically singular plus group is an error.
- Biomass and spawning biomass are evaluated at the start of the year, as in
  `CatchAtAge`. Spawning biomass uses population weight, maturity, and female
  proportion. Catch uses fleet-specific weight and the Baranov catch equation.
- Include fishing fleets only. Fleet shares are nonnegative and sum to one.
  Fishing mortality at age is `F * sum(share[f] * selectivity[f][age])`.
  Selectivities are used as supplied, without normalization. Thus reported F
  is the sum of fleet coefficients, not necessarily maximum F at age.
- Each fleet defaults to `include_in_msy = true`. Setting it to false excludes
  its yield from the objective while keeping its mortality and catch. Shares
  are not renormalized, and SPR is unchanged by these flags.
- An empty fleet list is allowed for unfished and SPR calculations. Inputs are never
  modified, and results hold no references to population state.

Survival into age `a` uses mortality at age `a - 1`. The plus-group abundance
is divided by `1 - exp(-Z[last])`. SPR is `SBPR(F) / SBPR(0)`, not the annual
spawning biomass divided by initial unfished spawning biomass.

## Example

```cpp
#include "population_dynamics/reference_points/reference_points.hpp"

fims_popdy::ReferencePointInputs<double> inputs;
inputs.natural_mortality = {0.2, 0.2, 0.2};
inputs.weight = {1.0, 2.0, 3.0};
inputs.maturity = {0.0, 0.5, 1.0};
inputs.proportion_female = {0.5, 0.5, 0.5};
inputs.fleets.push_back({1.0, {0.2, 0.7, 1.0}, {1.0, 2.0, 3.0}});

const auto per_recruit = fims_popdy::CalculatePerRecruit(inputs, 0.3);
const auto f40 = fims_popdy::CalculateSPR(inputs, 0.4);
if (f40.status == fims_popdy::SPRStatus::converged) {
  // Use f40.fishing_mortality; f40.spr records the achieved target.
}
```

`CalculatePerRecruit` returns numbers at age, total biomass, spawning biomass,
and total, objective, and fleet-specific annual yield, all per recruit.
The C++ `yield` member remains total catch; `objective_yield` sums only included
fleets. Equilibrium results use the same convention. Its arithmetic is
templated to follow FIMS conventions. Automatic differentiation integration has
not yet been validated.

`CalculateSPR` is a double-only post-fit bisection solver. `SPROptions` controls
maximum F (default 5), absolute SPR residual tolerance (1e-8), and maximum
iterations (100). The target must be in (0, 1]. Check `status` before using the
returned candidate:

- `converged`: the absolute SPR residual is within tolerance.
- `not_bracketed`: the target is below the SPR at maximum F. The result contains
  that upper-bound candidate; it does not establish that the target is
  unreachable at a higher F.
- `iteration_limit`: the result contains the last evaluated candidate.

Invalid dimensions, ranges, or solver options throw `std::invalid_argument`.
Undefined SPR (zero unfished spawning biomass), numerically singular equilibrium,
and nonfinite results throw `std::domain_error`. The solver does not calculate
standard errors or derivatives of the root.

## Tests

`tests/gtest/test_reference_points.cpp` is registered as the `reference_points`
CMake target. Tests include analytic abundance and SPR solutions, a long-run
projection with two fleets, fleet splitting, invalid inputs, solver diagnostics,
and repeated calls. Run the target's tests with
`ctest --test-dir build -R '^ReferencePoints\.' --output-on-failure` after building.

## R and population integration

```r
# After CreateTMBModel(), synchronize fitted fixed and random parameters as
# for model$get_output(), then select a modeled year (one-based index).
result <- get_reference_points(model, population$get_id(), year = 10,
                               spr_targets = c(0.4, 0.35))
result$spr
result$msy

# Reuse the saved snapshot without accessing live model state.
calculate_reference_points(result$inputs, max_f = 10)
```

The adapter refreshes derived quantities with the current internal double
parameters. It does not refit the model or load estimates from a saved FIMSFit.
The snapshot contains the population ID, year index, ages, fleet IDs, and all
biological inputs. By default, fleet shares follow that year's F coefficients.
To override them, pass `fleet_shares` in ascending fleet ID order; give survey
fleets zero share. Selectivity is used as supplied. Catch weights match the
population weights used in CatchAtAge's catch calculation. The live model
adapter requires at least two consecutive annual ages.

For MSY, the adapter retains the recruitment model's `R0`, steepness, and initial
`CalculateSBPR0()` baseline. It deliberately preserves the fitted model's
stock-recruit relationship when reference-year biology differs. Therefore the
new year's unfished equilibrium recruitment need not equal R0. The adapter
does not change the existing model's recruitment or unfished SBPR calculations.

## Equilibrium and MSY

`CalculateEquilibrium(inputs, F, recruitment)` substitutes `S = R * SBPR(F)`
into the existing FIMS Beverton-Holt equation and solves for positive recruitment:

```
R / R0 = [0.8 h - 0.2 (1-h) phi0 / SBPR(F)] / (h-0.2)
```

A nonpositive solution is reported as collapse, with zero recruitment, biomass,
and yield. Recruitment deviations are excluded. R0 and phi0 must be positive;
steepness must be strictly between 0.2 and 1. Unsupported recruitment models
can still calculate SPR by setting `msy = FALSE` in R. MSY requires at least
one included fleet; an empty or all-excluded objective throws an error.

`CalculateMSY` evaluates a grid on [0, max_f], then refines the best sampled
region using golden-section search. If the grid misses a narrow productive
region, the solver also searches below the first grid point. Endpoints remain
candidates. The result includes F, yield, recruitment, total and spawning
biomass, yield by fleet, iterations, and status:

- `converged`: the refined F interval is within tolerance.
- `upper_bound`: the best candidate is max_f; reassess with a larger bound.
- `no_positive_yield`: no positive yield was found at the search resolution.
- `iteration_limit`: refinement stopped before the F interval met tolerance.

A bounded grid search is not a guarantee of a global maximum for arbitrary
fishing patterns. Check sensitivity to bounds and grid resolution. In R,
`max_f`, `tolerance`, and `max_iterations` control both solvers (the tolerance
measures an SPR residual for SPR and an F interval for MSY); `grid_intervals`
controls the MSY grid. Returned `settings` record these choices.

Tests also cover recruitment fixed points, extinction, MSY against a dense
grid and R's independent optimizer, narrow productive regions, and the live
Rcpp model lifecycle including parameter synchronization and snapshot reuse.

## Bycatch and objective inclusion

A fleet with positive share and `include_in_msy = false` still produces
mortality and catch. It remains part of the fixed fishing pattern as F varies.
Only its catch is omitted from the optimized objective. This does not hold
bycatch mortality constant as F changes, and is different from a zero share.

```r
# Names refer to IDs returned in result$inputs$fleet_ids.
result <- get_reference_points(model, population$get_id(), year = 10,
                               include_in_msy = c(`1` = TRUE, `2` = FALSE))
result$msy$objective_yield  # MSY for the included fleets
result$msy$total_yield     # All fleets' catch at the chosen F
result$msy$fleet_yield     # Includes catch from the excluded fleet
```

For standalone snapshots, set `inputs$fleets[[i]]$include_in_msy <- FALSE`.
Omitted flags default to TRUE, preserving existing calculations. Inclusion
flags must be logical scalars; numeric weights and missing values are rejected.
An all-excluded objective is undefined for MSY, but remains valid for SPR.

For compatibility, the R `msy$yield` field remains an alias for `total_yield`.
When a fleet is excluded, use `objective_yield` for the optimized MSY quantity.
The snapshot saves all inclusion choices; it can be serialized and reused
without live model state. Tests cover distinct fleet selectivities and weights
against independent R equations and optimization, bound/grid sensitivity,
unchanged biology at fixed F, and fleet ordering and splitting invariance.
