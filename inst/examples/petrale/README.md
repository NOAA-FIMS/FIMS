# Petrale sole comparison: input audit

This is an input audit and proposed comparison sequence, **not a reproduced
assessment**. No SS3 executable has been run and no fitted results have been
matched. Sources were inspected on 2026-11-23 at repository revision
`09cee012c397df4793a859103941a54b544c6ebe`.

## Source models

The assessment repository's [report compilation function](https://github.com/pfmc-assessments/petrale/blob/09cee012c397df4793a859103941a54b544c6ebe/R/compile_petrale.R#L49)
selects `2023.a034.001` by default. Its supplied
[data](https://github.com/pfmc-assessments/petrale/blob/09cee012c397df4793a859103941a54b544c6ebe/models/2023.a034.001/petrale_data.ss)
and [control](https://github.com/pfmc-assessments/petrale/blob/09cee012c397df4793a859103941a54b544c6ebe/models/2023.a034.001/petrale_control.ss)
files are the two-sex reference inspected here. Parameter INIT values in that
control file must not be mistaken for independently reproduced fitted estimates.

The same repository already contains FIMS case-study variants:

- `2023.a050.003_FIMS_case-study_wtatage`
- `2023.a050.004_FIMS_case-study_1sex_with_growth`
- `2023.a050.005_FIMS_case-study_1sex_with_growth_no_fecundity`

These are modified models, not interchangeable with the two-sex reference. The
last variant's [data](https://github.com/pfmc-assessments/petrale/blob/09cee012c397df4793a859103941a54b544c6ebe/models/2023.a050.005_FIMS_case-study_1sex_with_growth_no_fecundity/petrale_data.ss)
sets `Nsexes=-1` and `Nages=17`; its
[control](https://github.com/pfmc-assessments/petrale/blob/09cee012c397df4793a859103941a54b544c6ebe/models/2023.a050.005_FIMS_case-study_1sex_with_growth_no_fecundity/petrale_control.ss)
uses weight-proportional fecundity (`fecundity_option=1`, alpha=1, beta=0).
It still uses length-based maturity, so its name does not imply complete
compatibility with FIMS's current age-based maturity calculation.

`input-manifest.json` records pinned URLs and SHA-256 hashes for the four inspected
input files. The full datasets are not copied into FIMS. Obtain the matching SS3
version and complete input bundle (including starter, forecast, parameter, and
any weight-at-age files) before claiming a reproducible reference run.

## What the two-sex inputs require

| Feature | Inspected SS3 configuration | FIMS implication |
| --- | --- | --- |
| Population | 1876–2022, one annual season, two sexes, one area, `Nages=40` | Separate linked female/male populations; reconcile age indexing, plus group, initial equilibrium and early recruitment. |
| Fleets | North and South fisheries; Triennial and WCGBTS surveys | Each actual fleet has two dynamic sex components and one pooled observation target where observations are unsexed. |
| Recruitment | Standard Beverton–Holt, steepness fixed at 0.8, female recruitment fraction 0.5 | One total-stock recruitment process; fixed 0.5/0.5 allocation. Match deviation eras, bias adjustment, and initial recruitment conventions separately. |
| Biology | Separate female/male M and von Bertalanffy growth parameters; age/length variation | Keep biology independent. Audit growth spread, units and integration rather than copying only mean curves. |
| Fishing mortality | `F_Method=3`, hybrid tuning, five iterations, max F=4 | Share F between sexes within a fishery, not between North and South. Current free F estimation is not SS3's hybrid catch-matching method. |
| Survey catchability | One log-q base per survey, `float=1` for both; additional Triennial observation error | Share q across sexes within each survey; keep survey owners separate. Explicit q estimation is not automatically equivalent to floated q. Match observation variance. |
| Selectivity | Size pattern 24 with male effects, blocks; age pattern 10 | Shared F/q must retain sex-specific selectivity. Current logistic-at-age selectivity is not a substitute for these SS3 patterns. |
| Reproduction | Length-logistic maturity, first mature age 3; fecundity option 2: eggs = alpha × length^beta | Female reproductive output is not simply mature biomass. Need size-integrated maturity/fecundity and matching units for stock–recruitment normalization. |
| Data | Sex-specific/combined composition conventions, ages conditional on length, survey timing and fishery discard settings | Audit each observation type. Pooling simple marginal compositions does not reproduce conditional age-at-length likelihoods or discard/retention structure. |

The biological and observation details above come directly from the pinned input
files. Mapping them onto FIMS is an implementation proposal. The source files must
remain the authority for choices such as fleet IDs, parameter phases, blocks,
likelihood weights, timing, and survey-error treatment.

## Parameter ownership to use in FIMS

For each fishery, choose one sex-specific component as the annual F owner and
make the other follow it. North and South need distinct owners. For each survey,
choose one component as its q owner; use distinct owners for Triennial and WCGBTS.
The choice of female versus male as the owner is administrative and does not
change which population supplies spawning output.

```r
model$ShareFishingMortality(north_female$get_id(), north_male$get_id())
model$ShareFishingMortality(south_female$get_id(), south_male$get_id())
model$ShareCatchability(triennial_female$get_id(), triennial_male$get_id())
model$ShareCatchability(wcgbts_female$get_id(), wcgbts_male$get_id())
```

Configure owner parameters first. Followers must remain constant, are not
independently estimated, and receive the owner's values at every evaluation.
Their selectivity, weight, abundance, and length mappings remain independent.
Ownership links are distinct from pooled-observation links.

## Comparison sequence

1. Run the pinned SS3 reference with the matching executable and preserve all
   inputs, outputs, executable version and hashes. Confirm which supplied model
   is intended as the assessment comparison target with its assessment authors.
2. Inspect the existing FIMS case-study variants and select a deliberately
   simplified common configuration. Reconcile maturity, growth, reproductive
   output and units explicitly, even for the no-fecundity variant.
3. Compare predictions at fixed parameters before optimizing: sex-specific
   abundance, mortality, catches, vulnerable numbers/biomass, marginal
   composition counts, reproductive output, and unfished output per total
   recruit. Use component-level residuals and declared numerical tolerances.
4. Add assessment features one at a time, including size selectivity, conditional
   ages, timing, retention/discards, recruitment bias adjustment, and the F/q
   estimation conventions. Match individual likelihood components and weights.
5. Only after fixed-parameter agreement, compare estimates, uncertainty and
   reference points. A fit to a simplified model must be labeled as such.
