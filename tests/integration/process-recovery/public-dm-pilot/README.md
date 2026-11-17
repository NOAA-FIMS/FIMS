# Public DM simulation and matched assessment integration

This study validates the public simulation extension at `921ef82`. It adds
benchmark code and evidence, without changing production code, earlier generators,
stored stress datasets, convergence thresholds or the historical pilots.

## Reproduce

From repository root, using an installed FIMS with the public DM extension:

```sh
Rscript tests/integration/process-recovery/run-public-dm.R /tmp/fims-public-dm 5 1000
Rscript tests/integration/process-recovery/audit-public-dm.R /tmp/fims-public-dm regenerate
Rscript tests/integration/process-recovery/plot-public-dm.R /tmp/fims-public-dm
```

The runner requires an empty directory and saves completed-case checkpoints.
The runner regenerates `result.rds`, containing exact assessment input data,
truth, plain snapshots and tables. Compact CSVs, configuration, console output, audit and the standalone
figure are committed. Source/production fingerprints and session information
identify the execution. No native TMB object is serialized.

## Distribution study

For each concentration infinity/2000/200, call `simulate_fims()` with 1000
three-year replicates and fixed R/F/M paths. The null override supplies the
multinomial control. Evaluate the initial-year age/length rows for both fleet1
and survey1: four fixed probability vectors, 70 category bins in total. All
composition nominal sizes are 2000. Predictions are checked identical across
replicates before any conditional-moment comparisons.

An independent reference uses the historical `robustness_probabilities()` gamma
**quantile** transform and `robustness_counts()` inverse-CDF categorical draws,
with distinct seed streams. It does not call the production gamma sampler.
Reference uniforms are shared across concentration settings; scenarios/categories
are not treated as independent replications in a pooled significance test.
Public/reference draws are distributional comparisons, not same-seed equality.

For fixed `p`, compare means with `n*p` and second moments about that known mean
with `n*p*(1-p)*(n+kappa)/(1+kappa)` (inflation 1 at infinity). Mean MCSE uses the
analytical variance; second-moment MCSE uses replicate-level dispersion of the
squared centered count. Compare the selected two largest-probability categories'
cross-product with the corresponding negative covariance. Public-minus-reference
MCSEs combine their independent-stream errors in quadrature.

Rare positive-probability bins with zero empirical squared-count dispersion have
**unestimable** second-moment z scores, retained as missing with their denominators.
They are not counted as passes or structural zeros. Structural-zero categories,
if any, must remain exactly zero. Standardized residuals are descriptive MCSE
units, not calibrated normal test statistics in every rare bin. These marginal
moments and selected covariance pairs do not prove full joint-law equality.

## Assessment study fixed before execution

Five new process seeds 6301–6305 generate 18-year datasets through the public
simulator at concentration 2000/200. All four fleet/type concentrations share the
generating value. Nominal composition size is 2000, log-catch SD 0.1 and log-index
SD 0.05. Biological parameters are known. True process SDs are R=0.2, F=0.15,
M=0.05; the same proper-normal F/M initial rule as the earlier benchmark is used.
Process paths and catch/index observations are checked identical within each seed
across concentration settings; composition draws can differ. These are fresh
datasets, not replacements for seeds 4301–4305.

Round trips check fleet/type/year/age/length alignment, finite generated values,
unit-sum compositions and integer nominal counts before placing observations
back into `FIMSFrame`. Refit each dataset with known concentration or four
separately estimated fleet/type concentrations (all initialized at 2000). Both
cases estimate all three process SDs. The two cases share the exact same dataset.
There is no misspecified multinomial assessment comparator in this study.

Four starts cross process SD 0.05/0.2 with default/deterministically shifted
latent states. Only if none passes, restart the best available snapshot from
saved states with original and tight outer controls, retaining both additional
attempts. This predeclared adaptive rule does not relax acceptance. If no valid
start exists, retain the selected invalid attempt and label its report accordingly.
There are 20 planned fitted cases, 80 initial attempts, up to 40 extra restart
attempts and 20 warm uncertainty-report attempts.

Bounds, code-zero/gradient/drift criteria and checked saved-mode reports are
unchanged from the [profile study](../dm-profile-pilot/README.md). Reports require
consistent objectives before/after `sdreport`, positive-definite reported Hessian
and finite positive state SEs to be eligible; interior strata also exclude fixed
covariance/concentration bounds. State-only reporting omits unused derived
ADREPORT uncertainty. It is not posterior sampling or global-mode validation.

Coverage averages dataset-level pointwise log-state metrics, with Monte Carlo
SE across five independent datasets, not across correlated age/year cells.
Known/estimated concentration contrasts require eligibility in both fits. Every
planned state report has a row; parameter points can remain recorded when their
report is ineligible. Negative/missing fixed-covariance diagonals are retained,
with unavailable SE/coverage labeled missing. These ten controlled assessments
cannot establish operational calibration, forecast or derived-quantity coverage.

## Attribution and outputs

The DM family follows the scientific precedent documented in the
[SS3 manual](https://nmfs-ost.github.io/ss3-doc/SS330_User_Manual_release.html);
[SAM simulation/re-estimation](https://fishfollower.r-universe.dev/stockassessment/doc/manual.html)
is a validation precedent. The public/reference comparison, new seeds, round-trip
checks, adaptive restart rule and local report protocol are FIMS-specific.
No reference-framework code is copied and no external assessment executable or
numerical model equivalence is tested. WHAM/SAM biological-process precedents
remain in the maintained change log.

`bins.csv`, `covariance.csv`, `generator_differences.csv` retain all moment
comparisons and uninformative bins. `attempts.csv`, `reports.csv`, `metrics.csv`,
`points.csv`, `checks.csv` retain generation, fitting and uncertainty outcomes.
Summary tables retain denominators, covariance-bound strata and replicate MCSEs.
`configuration.R` records design, source/production hashes and session versions.
The audit checks counts, the adaptive rule, paired input truth/scalars, reference
objectives and fingerprints; its optional `regenerate` mode verifies complete
input/truth reproduction and records the RNG kind. `run-output.txt` retains the
original console output.

## Recorded outcomes

The run retained 84 attempts: 80 initial and four predeclared refinements.
60/80 initial starts and all four refinements passed; all 20 selected fits and
warm reports are eligible, with 17 interior reports. Strong estimated-concentration
cases 1 and 3 required restart. No failed attempt or threshold was removed.

| Generating concentration | Fit | Eligible / 5 | Interior / 5 | M coverage %, MCSE pp | Mean log-M RMSE, MCSE |
|---|---|---:|---:|---:|---:|
| 2000 | known_dm | 5 | 4 | 92.0 (4.5) | 0.166 (0.018) |
| 2000 | estimated_dm | 5 | 4 | 91.9 (4.0) | 0.169 (0.017) |
| 200 | known_dm | 5 | 5 | 95.6 (2.1) | 0.233 (0.026) |
| 200 | estimated_dm | 5 | 4 | 95.1 (2.1) | 0.234 (0.026) |

Paired M-state differences (estimated minus known concentration):

| Generating concentration | Pairs | Coverage difference pp (MCSE) | RMSE difference (MCSE) |
|---|---:|---:|---:|
| 2000 | 5 | -0.09 (0.66) | 0.0031 (0.0016) |
| 200 | 5 | -0.46 (0.21) | 0.0013 (0.0018) |

Estimated concentration, separately by owner (all five reports eligible):

| Generating concentration | Owner | Mean estimate | Log RMSE | Local interval coverage |
|---|---|---:|---:|---:|
| 2000 | fleet1/age_comp | 2152.4 | 0.234 | 100% |
| 200 | fleet1/age_comp | 217.0 | 0.113 | 100% |
| 2000 | fleet1/length_comp | 2102.0 | 0.169 | 100% |
| 200 | fleet1/length_comp | 212.9 | 0.130 | 80% |
| 2000 | survey1/age_comp | 1962.5 | 0.192 | 100% |
| 200 | survey1/age_comp | 211.1 | 0.173 | 80% |
| 2000 | survey1/length_comp | 2237.0 | 0.185 | 100% |
| 200 | survey1/length_comp | 202.9 | 0.057 | 100% |

The moment checks retain these informative denominators (maximum absolute
standardized residual in parentheses):

- Analytical means: 420/420 (3.33 MCSE units).
- Analytical second moments: 373/420 (4.17); 47 empirical-error estimates are unavailable.
- Public/reference mean differences: 210/210 (2.68).
- Public/reference second-moment differences: 188/210 (2.96); 22 error estimates are unavailable.
- Selected covariance checks: 24/24 (2.86); 12 public/reference covariance differences peak at 2.69.

These are observed descriptive maxima, rounded upward; no familywise p value or
formal equivalence threshold is inferred. Complete counts and zero-bin checks
passed. All ten input/truth datasets regenerated identically under Mersenne-Twister,
Inversion, Rejection RNG settings, recorded in `audit-output.txt`.

See the [standalone figure](public-dm-validation.png) for descriptive residuals
and dataset-level coverage.

## Interpretation and limits

The public sampler and independent quantile/categorical sampler are consistent
with the checked analytical moments and selected covariances in this pilot.
Sparse empirical variance errors remain explicit rather than being called good
agreement. This is not a proof of full joint numerical equivalence, nor a check
against an external stock-assessment executable. Native generation and refitting
share FIMS population dynamics; the independent reference checks observation
sampling only and cannot rule out a shared dynamics bug.

Matched known/estimated concentration fits give similar state metrics on these
new datasets. The strong-setting paired coverage difference is -0.46 pp (MCSE
0.21 pp), so it should not be described as zero or universally negligible.
Moderate-setting coverage is about 92%, with MCSE around four pp; strong-setting
coverage is about 95%, with MCSE around two pp. Five process realizations, shared
across concentration settings, are insufficient for a calibration claim and must
not be pooled as ten independent biological replicates.

M-SD bounds remain: one moderate known and estimated fit each, plus one strong
estimated fit. All estimated concentrations are interior here, with owner-specific
mean estimates about 1962–2237 at truth 2000 and 203–217 at truth 200. Their five-fit
interval coverage of 80–100% is not a reliable nominal-coverage assessment.
Neither a successful public round trip nor recovery of plausible concentration
eliminates weak process-variance information or the need for multiple starts.
Coverage differences from earlier pilots use different data and RNG streams;
they do not demonstrate that the new generator improved the estimator.

The next scoped task is **mode-preserving spawning-biomass uncertainty validation**.
`CatchAtAge::Report()` already has spawning-biomass ADREPORT support
(`inst/include/models/functors/catch_at_age.hpp`, lines 1929/2000). Extend the
checked benchmark handoff to that existing derived quantity, verify owner/year
indices and fitted-year truth, and report local delta-method coverage with the
same eligibility/boundary/replicate rules. Do not treat the extra terminal
mean-recruitment report year as a stochastic forecast. Forecast innovations and
full parameter uncertainty remain separate extensions, not features established
by this fixed-parameter simulation study.

## Verification

Three focused tests / 15 assertions passed without failures, warnings or skips.
They check public observation round trips and RNG preservation, paired seed
truth/scalars, malformed-bin rejection, eligible-pair summaries and all-failed
planned rows. The execution audit checks 20 reports, 60 process-metric rows,
80 initial attempts plus exactly declared restarts, pairwise truth/scalar equality,
source/production hashes and before/after reference objectives. All ten complete
inputs/truth regenerated identically; R scripts parse, document links and
whitespace checks pass, and the final standalone figure was visually inspected.
No production source, dependency or historical pilot was changed.

Execution provenance remains `921ef82` with the recorded source/input fingerprints.
Publication also preserves `d06877e`, an automated refresh of packaged saved-fit
and estimate fixtures. Its only internal-data changes are `fit_big_unoptimized`
and `estimates_big_unoptimized`; `fims_input_types`, `data_big` and all production
source used here are unchanged. That refresh is not a new scientific algorithm
or a rerun of this study.
