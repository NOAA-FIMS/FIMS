# Subpopulation and partition branch comparison

The main change is moving from a general partition framework to a smaller,
explicit biological-group model. This supports the immediate goal of two sexes
with different biology while allowing additional groups later.

## Comparison scope

This comparison uses these locally available revisions:

- Our branch: `feat/subpopulation-noaa-main` at `20137ae`.
- Szu-yun's partition infrastructure: `feat/partition-infra` at `2e867b8`.
- Szu-yun's explicit two-sex implementation: `feat/explicit-two-sex` at `4ce5971`.

The comparison excludes unrelated changes inherited from NOAA main. Newer
remote revisions were not fetched for this comparison. Our implementation is
based on NOAA-FIMS main at `d7608eb`.

## Changes and benefits

| Change | Szu-yun's approach | Our branch | How it helps |
|---|---|---|---|
| Group structure | Generic axes, levels, Cartesian strata, selectors, and partition demands. | A list of named subpopulations. | Fewer abstractions to configure, validate, and maintain for a two-sex model. |
| Model configuration | Explicit-two-sex adds a `SexStructure` setting alongside partition configuration. | An empty subpopulation list uses pooled dynamics; configured groups use explicit dynamics. | Reduces interacting configuration settings. The same loop supports two or more groups. |
| Independent dynamics | Partition-infra splits pooled outputs; explicit-two-sex adds independent stratum dynamics. | Each group has its own abundance, mortality, survival, and plus group. | Sex ratios can change through differential survival. This preserves an idea already present in explicit-two-sex. |
| Initial and recruitment allocation | Explicit-two-sex provides separate allocation vectors, with defaults from `proportion_female`. | Each group declares `initial_fraction` and `recruitment_fraction`. | Makes entry assumptions explicit and keeps them separate from adult sex ratios. |
| Maturity and reproduction | Explicit-two-sex's default assigns zero maturity to males when calculating reproductive contribution. | Biological maturity and `reproductive_contribution` are separate. | Males can be biologically mature while contributing zero to female-only mature biomass. |
| Group-specific growth | Uses the population's shared growth module. | Groups can reference separate growth modules or inherit the default. | Supports sex-specific weights in biomass and catch calculations using existing FIMS modules. |
| Group-specific maturity | Explicit-two-sex leaves a hook for separate maturity modules. | Groups can reference their own maturity modules. | Supports different maturation schedules without sex-specific logic in the dynamics. |
| Fleet selectivity | Explicit-two-sex supports partition-specific selectivity values. | Groups reference existing selectivity modules by fleet, with shared fallbacks. | Reuses module parameter estimation and allows selective overrides without duplicating every fleet configuration. |
| R configuration | Primarily partition and sex settings exposed through Rcpp. | `initialize_fims(..., subpopulations = ...)` constructs group biology modules. | Makes the feature accessible through the ordinary R workflow and avoids managing raw module IDs. |
| Reporting | Requested strata are controlled through partition-demand routing. | Explicit groups expose `*_by_subpopulation` reports; pooled predictions sum group contributions. | Makes the relationship between modeled groups and reported quantities easier to inspect. |
| Validation | Tests partition indexing, demand handling, and explicit-sex calculations. | Adds conservation, pooled-equivalence, mixed-biology, third-group, module-link, and derivative checks. | Tests both biological accounting and whether group parameters correctly influence model calculations. |

## Deliberate omissions and tradeoffs

The implementation leaves out:

- Cartesian axes, wildcard selectors, and output-demand routing.
- A separate sex-mode enum.
- Movement or sex-change transitions.
- Group-specific observation links and uncertainty reporting.
- Group-specific derived age-to-length mappings.

These omissions reduce the initial implementation scope, but they are also
tradeoffs. Szu-yun's partition framework is better equipped to describe
combinations such as sex × area × stock and select subsets for reporting. Our
flat groups can represent those combinations manually, but do not provide
automatic cross-classification or transitions.

Initial and recruitment fractions are fixed configuration, not estimated
parameters. Separate natural mortality and initial abundance currently require
direct Rcpp construction. Catchability remains shared across groups.

Fixed age-to-length mappings remain supported. A group growth override combined
with an active growth-derived mapping for length observations is explicitly
rejected until group-specific length distributions are implemented.

## Implementation size

Relative to NOAA main at `d7608eb`, the feature changes through `20137ae` span
15 files, with 1,687 insertions and 29 deletions, including documentation, tests,
and an example. These counts exclude this comparison document.

"Minimal" means a narrower architecture, not a tiny patch or a demonstrated
speed improvement.

## Recommendation

Keep subpopulations as the biological modeling interface. If multi-axis
reporting becomes necessary, add classification metadata and selectors around
those groups. This preserves the simpler dynamics while introducing broader
partition machinery when a concrete model needs it.

## Review readiness

The implementation is ready for review as a scoped change. Validation completed
for `20137ae` includes:

- A clean package build.
- 48 selected C++ tests.
- Selected R tests, including derivative checks.
- A runnable two-sex example and finite-gradient check.

The full package suite has not been run. Before merging, run that suite and
review recruitment allocation, reproductive contribution, equilibrium
calculations, and pooled observation predictions. The PR should explicitly
identify the deferred features listed above.

## Related files

- [Implementation documentation](../inst/include/population_dynamics/population/README.md)
- [Runnable two-sex example](../inst/examples/two-sex-subpopulations.R)
- [High-level R configuration helper](../R/initialize_subpopulations.R)
- [C++ subpopulation tests](../tests/gtest/test_population_Subpopulation.cpp)
- [R subpopulation tests](../tests/testthat/test-subpopulations.R)

## Follow-up: group-owned initial abundance

The implementation now removes `initial_fraction`. Each subpopulation supplies
its own initial numbers at age, with fixed or estimated log-abundance parameters.
This replaces the earlier split-or-override behavior described in the comparison
above. Recruitment fractions remain separate and fixed. Total initial abundance
is the sum of group abundances; one group uses the same interface as several.
See the implementation documentation for the updated R configuration.
