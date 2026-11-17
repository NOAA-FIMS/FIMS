# Replicated matched-DM assessment validation

Run from the repository root against the DM-enabled installed FIMS:

```sh
Rscript tests/integration/process-recovery/run-dm-validation.R /tmp/fims-dm-validation 5 18
Rscript tests/integration/process-recovery/plot-dm-validation.R /tmp/fims-dm-validation
```

The runner refuses to replace a nonempty directory and checkpoints every completed
case. `result.rds` regenerates full inputs, truth and plain fitted snapshots;
compact CSV results, configuration, this report and a standalone figure are
committed. The output is a benchmark, not a new public fitting/persistence API.

## Design and attribution

This is a paired follow-up to the [misspecification pilot](../robustness-pilot/README.md)
and [single-dataset mode handoff](../mode-handoff-pilot/README.md). It regenerates
the same process seeds 4301–4305, 18-year R/F/M paths, catch/survey observations
and common-random-number composition datasets using unchanged prior helpers.
There are five independent datasets per composition-generating scenario:
multinomial, DM concentration 2000, and DM concentration 200, all at nominal
composition sample size 2000. Age/year cells within a dataset are correlated;
they are not independent Monte Carlo replicates.

The generating composition model is `p ~ Dirichlet(kappa * pi)`,
`x | p ~ Multinomial(n, p)`, so each category's multinomial variance is multiplied
by `(n + kappa)/(1 + kappa)`: about 2.0 at kappa 2000 and 10.95 at kappa 200.
The multinomial control is the infinite-concentration limit. Fits use the native
normalized DM row likelihood; the operating generator is the prior independent
benchmark sampler, not a new production simulation pathway.

All cases estimate recruitment, F and M innovation SDs jointly. Biological
parameters, observation errors outside compositions, and other population
parameters remain known. Initial F/M states follow the same proper-normal
operating/fitted model. The three observation fits are:

- Multinomial, preserving the prior misspecified comparator.
- DM with known generating concentration (finite-DM scenarios only).
- DM estimating four independent concentrations: age/length compositions for
  fleet1/survey1, constant across years. All start at 2000. A common generating
  concentration does not imply that fitted concentration parameters are shared.

The four starts cross process SD starts 0.05/0.2 with default/shifted latent
states. The shift is deterministic `0.15 * sin(index)` on the entire random-state
vector; it does not use the generating truth. Each fit estimates process SDs on
[0.001, 1] and concentration on [0.1, 100000]. The selected candidate is the
lowest objective among optimizer-valid starts; if none pass, the best finite
candidate is retained as invalid. Errors, warnings, rejected starts, bound hits,
conditional gradient checks and valid-start objective spreads are retained.
There are 40 planned fitted cases, 160 optimization attempts and 80 cold/warm
report attempts. This limited start grid does not establish global optimality.

[SS3's composition error options](https://nmfs-ost.github.io/ss3-doc/SS330_User_Manual_release.html)
provide the DM scientific precedent; [SAM's simulation/re-estimation tools](https://fishfollower.r-universe.dev/stockassessment/doc/manual.html)
provide a validation precedent. These references were checked 2026-11-17.
The FIMS paired designs, concentration convention, state snapshots and numerical
acceptance checks are FIMS-specific work, not copied platform code or a numerical
comparison with SS3/SAM executables. WHAM remains the biological-process precedent
recorded in the [maintained change log](../../../../docs/scientific-capability-change-log.md).
No production C++/R interface or dependency is changed in this increment.

## Reporting contract

Cold and saved-state rebuilds independently check joint/reference and marginal
objectives before uncertainty reporting, then the marginal objective afterward
(tolerance 1e-5), using the existing mode-preserving helper. Disagreements withhold
intervals rather than replace the reference fit. Endpoint checks do not prove
that every Hessian perturbation follows one basin. State-only `sdreport` omits
unused derived-quantity delta-method calculations.

An **eligible** report requires a valid selected optimizer (including repeated
objective, projected-gradient and inner-gradient checks), consistent rebuild,
positive-definite reported Hessian and finite positive state SEs. **Regular**
also excludes any fixed parameter within 1e-3 log units of a bound. Both strata
are shown; eligibility alone does not imply regular Wald inference. Cold reports
are numerical diagnostics; state/parameter summaries use saved-state reports.

Coverage is pointwise 95% Gaussian coverage on recruitment log-deviations and
fitted log-F/log-M states,
not joint path coverage, forecast calibration or derived-quantity uncertainty.
`state_summary.csv` averages each dataset's state metric, giving datasets equal
weight. Monte Carlo SE is `sd(dataset metric)/sqrt(number of usable datasets)`;
it is undefined for fewer than two usable datasets. Paired differences require
eligible reports in both compared fits. Every planned state report has a row,
even if withheld. Report denominators and boundary strata must accompany coverage.
With only five replicates, these MCSEs are descriptive and cannot establish
nominal coverage or reliable tail behavior.

Parameter tables retain point estimates even for invalid/ineligible reports,
but summaries filter by the stated stratum. Hyperparameter summaries separate
each concentration owner and process SD; correlated concentration types are not
pooled as independent replicates. Log-error/coverage for the multinomial truth's
infinite concentration is undefined. Reaching the finite upper cap under that
truth is expected limiting behavior, not recovery of a finite parameter.

## Outputs

- `attempts.csv`: every start, warnings/errors, validity, selected candidate and time.
- `reports.csv`: both rebuild modes, objective probes, report status and eligibility.
- `metrics.csv`: planned warm state metrics, including withheld rows.
- `points.csv`: selected SD/concentration estimates, limits, local SE and coverage.
- `fit_summary.csv`, `state_summary.csv`, `paired_summary.csv`,
  `hyperparameter_summary.csv`: denominators, strata and replicate-level MCSEs.
- `configuration.R`: design, exact execution-source fingerprints, production-source
  fingerprints, base commit and R/TMB session versions.
- `dm-validation.png`: dataset-level M recovery/coverage and parameter estimates;
  concentration dots across types within one dataset are correlated.

## Recorded outcomes

The completed pilot retained 160 optimization attempts, 80 report attempts and 180 selected parameter rows. 134/160 starts passed the optimizer checks; 37/40 selected fits passed.

Cold report statuses: consistent_local_report: 36/40, rebuild_mismatch: 4/40.

Saved-state report statuses: consistent_local_report: 40/40.

Mean M-state metrics below use **eligible warm reports**, including boundary fits; the interior count excludes *any* fitted covariance/concentration bound. Parentheses are replicate-level MCSEs. Different eligible denominators make unpaired means unsuitable as paired effect estimates.

| Generating errors | Fit | Eligible / 5 | Interior / 5 | M coverage %, MCSE pp | Mean log-M RMSE, MCSE |
|---|---|---:|---:|---:|---:|
| multinomial | multinomial | 5 | 5 | 94.7 (1.9) | 0.161 (0.006) |
| multinomial | estimated_dm | 5 | 1 | 94.8 (1.9) | 0.161 (0.006) |
| moderate | multinomial | 5 | 5 | 90.8 (2.2) | 0.218 (0.017) |
| moderate | known_dm | 5 | 4 | 91.3 (3.2) | 0.181 (0.020) |
| moderate | estimated_dm | 5 | 4 | 91.1 (3.1) | 0.183 (0.020) |
| strong | multinomial | 3 | 0 | 61.4 (2.8) | 1.203 (0.113) |
| strong | known_dm | 5 | 3 | 87.5 (4.4) | 0.252 (0.033) |
| strong | estimated_dm | 3 | 1 | 81.0 (2.9) | 0.288 (0.025) |

Paired M-coverage contrasts use only datasets eligible under both observation fits. Positive differences favor the DM fit in this descriptive comparison; they do not establish nominal coverage.

| Generating errors | DM comparison against multinomial | Paired datasets | Coverage difference pp | MCSE pp |
|---|---|---:|---:|---:|
| moderate | known_dm minus multinomial | 5 | 0.5 | 2.0 |
| moderate | estimated_dm minus multinomial | 5 | 0.3 | 2.1 |
| strong | known_dm minus multinomial | 3 | 28.7 | 10.2 |
| strong | estimated_dm minus multinomial | 1 | 8.8 | NA |

Estimated finite concentrations are summarized per owner (eligible reports). The generating values are 2000/200 for moderate/strong errors. These are five-dataset summaries, not independent evidence from every composition row.

| Generating errors | Concentration owner | Reports | Mean estimate | Log RMSE | 95% coverage | Bound hits |
|---|---|---:|---:|---:|---:|---:|
| moderate | fleet1/age_comp | 5 | 1637.2 | 0.266 | 80% (5 reports) | 0 |
| strong | fleet1/age_comp | 3 | 193.6 | 0.060 | 100% (3 reports) | 0 |
| moderate | fleet1/length_comp | 5 | 1848.6 | 0.201 | 100% (5 reports) | 0 |
| strong | fleet1/length_comp | 3 | 215.3 | 0.095 | 100% (3 reports) | 0 |
| moderate | survey1/age_comp | 5 | 2460.4 | 0.291 | 100% (5 reports) | 0 |
| strong | survey1/age_comp | 3 | 209.4 | 0.096 | 100% (3 reports) | 0 |
| moderate | survey1/length_comp | 5 | 1779.9 | 0.149 | 100% (5 reports) | 0 |
| strong | survey1/length_comp | 3 | 199.7 | 0.033 | 100% (3 reports) | 0 |

M innovation-SD bound hits among **all selected fits**, including ineligible reports:

| Generating errors | Fit | Lower bound | Upper bound | Selected fits |
|---|---|---:|---:|---:|
| multinomial | multinomial | 0 | 0 | 5 |
| multinomial | estimated_dm | 0 | 0 | 5 |
| moderate | multinomial | 0 | 0 | 5 |
| moderate | known_dm | 1 | 0 | 5 |
| moderate | estimated_dm | 1 | 0 | 5 |
| strong | multinomial | 0 | 5 | 5 |
| strong | known_dm | 2 | 0 | 5 |
| strong | estimated_dm | 2 | 0 | 5 |

See the [standalone figure](dm-validation.png) and CSVs for the complete distributions and numerical diagnostics.

## Interpretation, numerical limits and next step

All 40 saved-state reports reproduce their reference objectives; four cold
rebuilds disagree. This resolves the local handoff in this run, not global mode
selection. Replicate 3's strong-stress multinomial fit has two optimizer-valid
start groups separated by **5.152 NLL units**; the lower found mode is retained.
The four-start grid is demonstrably sensitive to basin/initialization choices.

The three selected optimizer failures are strong estimated-DM replicates 3/5
and strong multinomial replicate 4. Each returns optimizer code zero but exceeds
the pre-set 0.001 projected-gradient threshold (about 0.00118–0.00140). They are
not relabeled valid because they narrowly miss the criterion. Strong multinomial
replicate 2 also has a non-positive-definite reported Hessian. Thus 36/40 warm
reports are eligible even though all 40 are numerically consistent rebuilds.
One negative fixed-covariance diagonal produces an explicit NaN-SE warning;
its missing SE/coverage and ineligible status remain in the tables. The original
console output is preserved in `run-output.txt`.

Known DM improves strong-stress M coverage by **28.7 percentage points** (MCSE
10.2 pp) on the three datasets eligible under both known-DM and multinomial fits.
Estimated-DM has only one corresponding usable pair: its 8.8 pp difference has
no estimable replicate-level MCSE. The 81.0% versus 87.5% unpaired coverage means
use different subsets and must not be interpreted as the cost of estimating
concentration. Neither fitted observation family establishes nominal calibration.
At moderate stress, paired coverage changes are small relative to MCSE, although
mean state error is lower with DM in this pilot.

Finite concentrations have plausible recovery in the eligible fits, without
finite-concentration bound hits. Moderate means vary across composition owners
(about 1637–2460 at truth 2000); strong eligible means are about 194–215 at truth
200. These small, selected samples do not prove unbiasedness or reliable Wald
coverage. Under multinomial truth, 12/20 fitted concentrations reach the upper
cap; infinity has no finite log-error or concentration coverage target.

The next scientific priority is **profiled M-SD/concentration identifiability and
optimizer convergence refinement** on these fixed datasets: retain the original
failed attempts, tighten stopping controls rather than relax acceptance checks,
compare alternate inner solutions, and profile M variance while reoptimizing
concentration and other SDs. No unrestricted covariance extension is justified
as a remedy for these findings. Production DM observation simulation remains a
useful separate capability increment, using validated sampling/zero/missing/RNG
contracts; it would not by itself repair estimation or calibrate forecast
uncertainty. Derived-quantity/forecast coverage remains a later validation target.

## Checks

Four focused tests / 25 assertions passed with no failures, warnings or skips
(`RUN_SLOW_TESTS=true`), covering dataset-level MCSEs, paired eligibility/failure
denominators, all-failed runs, fleet/type parameter ownership and alternate-state
estimated-DM snapshots across native model clearing. The 160-attempt run's audit
confirmed planned row counts, one selected candidate per case, matching execution
and production fingerprints, and objective tolerances for every consistent
report. Prior operating-model helper fingerprints also remain unchanged. R scripts
parse, report links and whitespace checks pass, and the figure was visually
inspected. No production code was changed or external model executable compared.
