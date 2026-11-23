# Linked-population recruitment prototype

This branch explores two independently tracked populations connected by one
stock recruitment process. It does not activate the subpopulation scaffold.
The low-level Rcpp interface supports the link; `initialize_fims()` still creates
one population and does not yet provide a wrapper configuration for links.

## Configuration

Construct female and male `Population` objects with their own initial abundance,
growth, maturity, mortality, and fleets. Assign both populations the **same**
`BevertonHoltRecruitment` object, with one recruitment process and one set of
recruitment deviations. Then add them to a single catch-at-age model:

```r
model <- methods::new(CatchAtAge)
model$AddPopulation(female$get_id())
model$AddPopulation(male$get_id())
model$LinkRecruitment(
  c(female$get_id(), male$get_id()),
  c(0.4, 0.6)
)
```

The fractions are fixed recruitment allocations, not multipliers on spawning
biomass. They must be finite, nonnegative, and sum to one. A population can occur
in only one link. Members must have matching years and recruitment ages; their
older age structures and biology can differ.

For exact zero male spawning contribution, attach a fixed maturity schedule:

```r
male_maturity <- methods::new(ConstantMaturity)
male_maturity$SetValue(0)
male$SetMaturityID(male_maturity$get_id())
```

Here “maturity” means contribution to spawning biomass; a zero male schedule does
not assert biological immaturity. `ConstantMaturity` accepts any fixed proportion
in [0, 1]. It introduces no estimated parameter. Female maturity can use the
existing logistic module. An executable minimal equilibrium configuration is
exercised by `make_linked_recruitment_example()` in
`tests/testthat/test-recruitment-link.R`.

## Execution and reference points

The model advances all populations one year at a time. Once all previous-year
spawning outputs are available, each stock recruitment process is evaluated
exactly once, and its total recruits are allocated to the members. The existing
terminal-year convention is retained: the final reporting year uses mean
recruitment without a process deviation.

For population g with allocation p_g:

- Stock spawning output is the sum of each member's maturity-weighted biomass.
- Stock spawning output per total recruit is `sum(p_g * phi0_g)`.
- `R0` denotes total stock recruits, and each member receives `p_g * R0` in
  unfished calculations.
- Initial numbers at age remain independently supplied or estimated. The link
  does not force the initial sex ratio to match the recruitment allocation.
- Each member's `expected_recruitment` reports its allocated entrants. Sum these
  to obtain total stock recruitment.
- Each member's `spawning_biomass_ratio` reports the same stock-level ratio:
  summed spawning output divided by summed initial unfished spawning output.
  This avoids an undefined male-only zero/zero ratio.

Link allocations and the constant maturity value are included in model JSON.
No links means the existing independent-population recruitment behavior.

## Pooled fleet observations

Keep each dynamic fleet component attached to exactly one population. This
preserves that population's abundance, selectivity, mortality, weight at age,
and age-to-length conversion. Create a separate `Fleet` for pooled observations,
set its year/age/length dimensions and length-bin centers, and do **not** add it
to a population. Then configure:

```r
model$LinkFleetObservations(
  pooled_fleet$get_id(),
  c(female_fleet$get_id(), male_fleet$get_id()),
  TRUE # use catch counts for compositions; FALSE uses survey index counts
)
```

Attach pooled catch, index, and composition observations to `pooled_fleet`.
Link their density components once to that fleet's `catch_expected` (or log
prediction), `index_expected`, `agecomp_proportion`, and
`lengthcomp_proportion`, as appropriate. The target is observation-only: it has
no selectivity, population mortality, catchability multiplier, or length mapping
of its own. Its F and q parameters must remain constant; their placeholder values
are unused. `LinkFleetObservations()` handles the target's unused mapping and
mortality-vector defaults.

Counts and weights at age and annual catch/index totals are summed. Each
component maps its own counts to length **before** pooling; the target then
normalizes the summed age or length counts and applies its own observed sample
size. Component proportions are never averaged. The composition source is
explicit and does not depend on whether the component fleets have their own
catch observations.

Sources must share the same year and age grids. Pooled length compositions
require matching length-bin centers and boundaries across components, and an
active age-to-length mapping for each source. Targets cannot be sources of other
links; nested or cyclic pooling is rejected. Raw fleet objects still cannot be
shared between populations. Link configuration and observation-only fleet
outputs are included in model JSON and TMB reports.

Independent sex-specific samples may remain on component fleets. Do not copy the
same pooled dataset onto the components as well: that would count its likelihood
multiple times. Missing composition rows retain the existing missing-data
convention in the density modules.

## Shared fishing mortality and catchability

Parameter ownership is separate from observation pooling. Configure an owner's
parameter values and estimation types, keep follower parameters constant, and
link the dynamic components before `CreateTMBModel()`:

```r
model$ShareFishingMortality(female_fleet$get_id(), male_fleet$get_id())
model$ShareCatchability(female_survey$get_id(), male_survey$get_id())
```

Only the owner registers estimated parameters. Each evaluation copies the
owner's log-parameter values as the model's scalar `Type` before transforming
F and q. Followers therefore contribute derivatives to the same owner parameters
without independently estimating duplicates. Each component retains its own
selectivity, biology, abundance and length mapping. The owner choice does not
imply that it is the reproductive population.

F is an annual vector. q may be scalar or annual; the follower's configuration
is resized to the owner's vector when linking. Finish configuring owner shapes
before linking. Grids must match at evaluation, and followers must remain
constant even if the R objects are edited after linking. Chains, cycles,
self-links, duplicate ownership, and observation-only fleet participants are
rejected. F and q may have different owners because they are separate parameters.

JSON records the ownership links and the current copied follower values.
Followers retain constant parameter rows and are not separate estimates;
parameter uncertainty belongs to the owner's rows. Do not put duplicate priors
on both an owner and its follower unless two independent prior contributions
are actually intended.

## Current boundary

This is a working recruitment, observation and parameter-coupling prototype, not
a Petrale sole assessment implementation. The low-level Rcpp interface supports
all three link types; wrapper configuration still needs multi-population support.
SS3-specific F tuning and analytical q floating are not implemented by these
links. The [Petrale input audit](../../../examples/petrale/README.md) records
remaining assessment-specific differences and a fixed-parameter comparison plan.
