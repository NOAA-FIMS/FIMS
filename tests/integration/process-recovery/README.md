# Recruitment, F and M recovery benchmark

Run from the repository root against an installed FIMS containing the process
upgrades (through `053646a`). No additional packages are required beyond FIMS's
existing dependencies.

```sh
Rscript tests/integration/process-recovery/run.R /tmp/fims-recovery 5 6 4301
```

Arguments are an empty output directory, number of replicates, number of model
years (3–30), and base seed. Replicate `i` uses seed `base + i - 1`. The runner
refuses to replace a nonempty output directory. To run the integration check:

```sh
RUN_SLOW_TESTS=true Rscript -e 'testthat::test_dir("tests/testthat", filter="^process-recovery-benchmark$", package="FIMS", load_package="installed")'
```

## Experiment

The operating model uses the existing `data_big` assessment structure, all 12
ages, one catch fleet and its survey, and shared empirical weights covering the
terminal report year. It generates IID Gaussian recruitment deviations (SD 0.2),
fleet log-F random walks (innovation SD 0.15) and independent calendar-age log-M
random walks (common innovation SD 0.05). Initial log-F and each age's log-M are
independently drawn from `N(log(0.2), 0.25^2)`. Other biology, initial abundance,
selectivity, catchability and observation parameters are known. Catch log-SD is
0.1, index log-SD is 0.2 and compositions have multinomial design size 200.
Simulated observations replace the corresponding input rows, retaining their
layout and uncertainty formulas.

Each replicate is fitted in 14 paired configurations:

- Estimate recruitment, F or M alone; condition other process trajectories on
  their exact simulated truth. These are deliberately optimistic oracle cases.
- Estimate all three processes jointly.
- For each case, hold innovation SDs at truth or estimate the active SDs.
- For F/M, condition on known first-year states or estimate those states under
  the same proper-normal prior as the operating model. Recruitment is IID, so
  its single-process fit has no initialization duplicate.

The fixed-initial case knows the simulated first-year states exactly; the
proper-normal case does not. They answer different information questions, so a
performance difference cannot be attributed solely to the prior's form. All
estimated states start away from truth (0.05 for recruitment log deviations,
log(0.18) for log-F/log-M). Estimated innovation SDs start at 0.1. Inactive
processes and fixed anchors alone retain true values. The benchmark currently
uses one starting point per fit and does not establish global optimality.

Inference uses TMB Laplace integration, `nlminb` on covariance parameters,
`sdreport` marginal uncertainty, and the inverse joint precision for posterior
correlations. Native names map full-precision values to each process; rounded
JSON output is not used. Only innovation SDs are optimized externally, bounded
to [0.001, 1]. A log-SD within 0.001 of a bound is flagged. This is a declared
benchmark constraint, not a new FIMS default. No production C++ or R code is
changed by this benchmark.

## Outputs and interpretation

- `result.rds`: configuration, per-replicate truth and simulated input data, RNG
  metadata, session versions, and all raw result tables. It contains no live
  FIMS/TMB objects.
- `configuration.R`: readable configuration, source MD5, base Git revision and
  session information. The source MD5 identifies the benchmark script beyond
  the base production revision.
- `diagnostics.csv`: every planned fit, including caught errors, optimizer code,
  raw outer and inner gradients, Hessian status, SD-boundary flag, parameter
  dimensions, runtime and warning/error messages.
- `states.csv` and `state_metrics.csv`: log-scale truth, estimates and marginal
  SEs, bias, RMSE and pointwise 95% interval coverage. Only estimated states are
  scored; known anchors and oracle trajectories cannot inflate recovery.
- `hyperparameters.csv`: natural-scale innovation SD recovery with log-scale
  Wald intervals and boundary flags. Intervals near variance boundaries should
  not be interpreted as regular asymptotic confidence intervals.
- `correlations.csv`: maximum absolute posterior state correlation for each
  process pair. Missing blocks are NA. This is a regularized local diagnostic,
  not proof of structural identifiability.
- `fit_summary.csv`, `state_summary.csv`, `hyperparameter_summary.csv`: summaries
  with explicit fit and interval denominators. A qualified fit has no error,
  optimizer code zero, inner and raw outer maximum gradients below 0.001,
  positive-definite `sdreport` Hessian, and no SD boundary. Other fits remain in
  the tables and are summarized separately. Qualification is a diagnostic label,
  not a guarantee of a unique solution or valid intervals.

State coverage is averaged within each replicate and then across replicates.
Its Monte Carlo SE uses the variation of replicate coverage fractions; cells
are not treated as independent Bernoulli trials. Single-replicate summaries
have NA Monte Carlo SE. Failures are counted in the planned-fit denominator,
not assigned successful estimates or intervals. Runtime includes initialization,
optimization, uncertainty and correlation calculation, but excludes operating
model simulation, garbage collection between fits and result writing.

A short series with known biology is a screening experiment. It cannot establish
operational recovery, interval coverage, long-horizon scaling or robustness to
model misspecification. Larger runs should vary horizon, observation information,
starting points and process SDs before adding age covariance or common mortality
shocks. `pilot/` records the small local pilot, including failures and boundary
fits; the full RDS stays in the chosen output directory.

## Longer-series, multiple-start and mortality-SD profile experiment

```sh
Rscript tests/integration/process-recovery/run-sensitivity.R /tmp/fims-sensitivity 3
Rscript tests/integration/process-recovery/plot-sensitivity.R /tmp/fims-sensitivity
```

This extension preserves the original benchmark and its source checksum. It
compares joint recruitment/F/M estimation at 6 and 18 years, with three common
innovation-SD starts (0.025, 0.1, 0.35) and both initial-information settings.
Each start sets all three covariance parameters to that value, while the latent
states retain the original benchmark's deliberately displaced starts. It does
not try all combinations of covariance starts, perturb latent starts, or prove
global optimization. Three replicated datasets are generated per horizon.

For replicate 1 at each horizon, the experiment also fixes mortality SD on
`0.001, 0.005, 0.01, 0.025, 0.05, 0.1, 0.2, 0.4`. At **every** point and start,
recruitment and F SDs and all latent states are reoptimized. This is a Laplace
marginal-likelihood profile, not evaluation of a fixed trajectory or posterior
Wald interval. TMB maps fix only the mortality covariance parameter. Density
normalizers and integrated-state dimensions are retained. A test compares the
free and mapped objectives at the same covariance values.

The `attempts.csv` table retains every start, including errors and nonconverged
solutions. `best.csv` selects the lowest objective among valid optimizer results
for each free fit or profile point; when none pass, the failed candidate remains
with `valid = FALSE` and no delta NLL. Validity requires optimizer code zero,
inner maximum gradient below 0.001 and maximum box-projected outer gradient
below 0.001. At a lower bound a positive raw gradient satisfies the KKT condition;
at an upper bound a negative one does. Both raw and projected gradients remain
visible. This validity label permits SD boundaries for likelihood exploration,
unlike the original benchmark's stricter interval-qualification label. No
Hessian/coverage qualification is performed in this likelihood experiment.

`objective_spread` compares the valid starting-point solutions, and
`valid_starts` gives its denominator. Relative NLL uses the best valid free or
profile objective for that same dataset and initial-information setting. A
profile point below the free fit therefore remains visible as a sign that the
free optimization may need attention. No chi-square confidence threshold is
applied: boundaries, short series and the coarse grid require more care.

`checkpoint.rds` saves completed datasets and attempts after each replicate;
`result.rds` also retains operating-model truth/data and final tables. The runner
captures source checksums before the experiment. It refuses to overwrite
nonempty output directories and does not currently implement automatic checkpoint
resume. Same seed labels across horizons do **not** produce nested trajectories,
because the numbers and ordering of process draws change with horizon. Fits are
paired within each generated dataset, not between lengths.

Timings include initialization, outer optimization and latent-mode diagnostics,
but exclude `sdreport`, covariance inversion, simulation, garbage collection
and writing. They are not directly comparable to the original pilot's timings,
which included uncertainty calculations. `M_rmse` scores only estimated mortality
states, so proper and anchored settings have different first-year scoring sets.
The PNG shows replicate 1 only. Findings must be checked across more replicates
before choosing additional mortality covariance or claiming coverage/scalability.

The retained [longer-series/profile pilot](sensitivity-pilot/README.md) records
all attempts, profile results, source provenance and the inspected profile plot.

## Paired observation-information experiment

```sh
Rscript tests/integration/process-recovery/run-information.R /tmp/fims-information 3 18
Rscript tests/integration/process-recovery/plot-information.R /tmp/fims-information
```

The runner accepts an empty output directory, replicate count and horizon. The
four designs form a two-by-two information comparison:

| Design | Survey log-SD | Age/length multinomial sample size |
|---|---:|---:|
| baseline | 0.20 | 200 |
| survey | 0.05 | 200 |
| compositions | 0.20 | 2000 |
| both | 0.05 | 2000 |

Catch log-SD remains 0.1. The operating recruitment/F/M processes, truth SDs,
initial rules and known biology remain those of the original benchmark. A single
latent trajectory is generated per replicate, then all four observation datasets
are generated around it. They use matching likelihood formulas when fitted.
These are hypothetical sampling designs, not a cost analysis or recommendation
that actual data achieve independent multinomial information of this size.

Pairing uses a separate observation seed (`process_seed + 100000`), common normal
errors for catch/survey observations, and one independent categorical pool per
composition observation. The larger sample includes the smaller sample as a
prefix, so both have the appropriate multinomial marginal distribution. Age
and length compositions remain independent observation streams, as assumed by
the likelihood. Each design retains identical catch observations. The RNG state
is restored. This coupling reduces sampling differences between designs without
changing their marginal observation models. These observations differ from the
older pilots' sampling algorithm, so compare information designs within this
experiment rather than attributing changes from prior pilot numbers to precision.

Joint recruitment/F/M fits use common SD starts 0.05 and 0.2, with known first-year
anchors or proper initial priors. At default settings there are three replicates
and 18 model years. Baseline and `both` designs receive reoptimized mortality-SD
profiles for dataset 1 at six grid points (`0.001, 0.01, 0.05, 0.1, 0.2, 0.4`),
from both starts under both initial settings. Intermediate designs receive free
fits only. The default experiment therefore has 96 optimizer attempts.

Selection and gradient diagnostics reuse the sensitivity benchmark. Every
attempt remains in `attempts.csv`; `best.csv` and `summary.csv` retain valid-fit
and dataset denominators, boundary flags, mortality state RMSE and profile deltas.
All-invalid groups remain present with missing estimates. Profile references are
computed **separately for every observation dataset and initial-information
setting**: raw objectives from different designs or conditioning settings must
not be compared. No coverage, Hessian qualification or
asymptotic profile confidence cutoff is calculated in this experiment.

RDS results/checkpoints retain every design's data, shared truth, predictions,
standard-normal errors, categorical pools and seeds. Source checksums are captured
before running. Existing pilot sources and artifacts remain unchanged. The plot
of SD estimates shows paired datasets across designs; the profiles show dataset
1 only, on each design's own relative NLL scale. Missing/invalid fitted values
are not drawn as successful estimates. Three paired replicates provide screening
evidence only; stronger sampling does not guarantee identifiable process variance,
and these runs do not incorporate unknown selectivity, aging error, composition
overdispersion or biological misspecification.

The retained [observation-information pilot](information-pilot/README.md) includes
all attempts, paired state/SD results, separately normalized profiles and inspected plots.

## Interval calibration and true mortality-SD sensitivity

```sh
Rscript tests/integration/process-recovery/run-calibration.R /tmp/fims-calibration 10 18
Rscript tests/integration/process-recovery/plot-calibration.R /tmp/fims-calibration
```

This extension uses the combined informative design (survey log-SD 0.05 and
composition size 2000), proper-normal first-year uncertainty, and true mortality
innovation SDs 0.025, 0.05 and 0.10. Ten replicates per setting are the default;
the runner also accepts replicate count and horizon after the output directory.
Recruitment/F SDs remain 0.20/0.15. All biology and observation parameters remain
known. Within each dataset, fits compare **all** innovation SDs held at truth
against **all three** estimated, so differences cannot be attributed exclusively
to estimating the mortality SD. Estimated fits use common starts 0.05 and 0.20.
Every optimizer attempt and its selection/validity flag is retained.

The same process/observation seed labels are used across true M-SD settings.
Recruitment deviations, F paths, initial M states and standardized mortality
innovations are paired; biological trajectories and observation probabilities
change with M SD. Categorical observations need not be nested across M-SD values.
At true SD 0.05, the first three datasets reproduce the prior combined-information
design's data. Per-dataset truth, inputs, sampling pools and seeds are stored in
RDS/checkpoint output. The original source files and pilots remain unchanged.

For estimated covariance, the best valid starting-point result is rebuilt at
its selected full-precision covariance parameters, with a checked objective
agreement before uncertainty estimation. A fresh latent-mode solve and TMB
`sdreport` produce Laplace/Gaussian marginal log-state intervals. Known-SD models
have no outer parameters but still check inner-mode stationarity. State modes,
SEs, pointwise interval coverage, interval width and standardized errors are
recorded for recruitment log-deviations, annual log-F and age-specific log-M.
These are not intervals for total recruits after biological-mean uncertainty.
This does not assess simultaneous all-cell
coverage, forecast intervals or derived-quantity intervals such as spawning biomass. These are joint-generative simulation checks
for latent-state uncertainty, not fixed-state frequentist coverage guarantees.

`diagnostics.csv` distinguishes optimizer validity, `sdreport` exceptions,
positive-definite outer Hessian status, any-SD and M-specific boundaries, and a
`regular` reporting flag. Regular fits require valid optimization, a positive
`sdreport` Hessian, finite positive state SEs and no covariance boundary. That is
a reporting stratum, not evidence of a unique solution or calibrated intervals.
All computed numerical intervals remain in the `all` summaries; regular and
nonregular strata are also shown. Off-optimum or boundary intervals must not be
interpreted as regular asymptotic inference. Missing intervals are not silently
assigned zero error or successful coverage. Fit/interval denominators and failure
rows are retained; strata with no computed fits may have no summary row.

State coverage is first averaged within each replicate, then across replicates.
Its MCSE uses the variation in those fractions, not a binomial count of dependent
age-year cells. The plot shows one MCSE on either side, not a confidence interval. Its
postprocessor also writes `paired-summary.csv`, with estimated-minus-known
coverage differences and MCSEs calculated from paired replicate differences.
Ten replicates remain a screening run; even zero empirical MCSE does not prove
coverage. Regular-only results condition on successful, interior outcomes and
cannot substitute for unconditional performance.

Hyperparameter tables retain natural-scale SD estimates and log-SD Wald-interval
coverage. `hyperparameter_summary.csv` gives covered/available counts and exact
binomial uncertainty limits for the **coverage proportion**, not for the process
SD. At variance boundaries, huge log-SD SEs can yield superficially high coverage;
those apparent successes are recorded but are not evidence of valid Wald
intervals. All, regular and nonregular results are separated.

The default run has 30 generated datasets, 60 known/estimated uncertainty reports
and 60 estimated-covariance optimizer starts. Checkpoints persist after each
dataset. Candidate fit timings and uncertainty-report timings are separate; the
latter include initialization, rebuilding/evaluation and `sdreport`, not a second
outer optimization. No production code, dependencies or prior pilot sources are
changed. Results apply to the declared correct-model, informative-data scenario;
more replication, misspecification and observation uncertainty require separate
experiments before operational conclusions.

The retained [interval-calibration pilot](calibration-pilot/README.md) records
all 60 uncertainty reports, covariance starts, state/hyperparameter summaries,
paired differences and the inspected coverage/width plot.

## Composition-overdispersion misspecification

```sh
Rscript tests/integration/process-recovery/run-robustness.R /tmp/fims-robustness 5 18
Rscript tests/integration/process-recovery/plot-robustness.R /tmp/fims-robustness
```

The three generating scenarios share latent process truth, catch/survey data and
composition random-number pools. Every fitted likelihood remains multinomial
with nominal sample size 2000; no production Dirichlet–multinomial likelihood
is introduced. Recruitment/F/M innovation SDs remain 0.20/0.15/0.05, proper initial
uncertainty and known biology are retained, survey log-SD is 0.05 and catch log-SD
is 0.1. The five-replicate default is a bounded robustness pilot, not a calibration
study with adequate power for operational claims.

For each independent fleet/type/year composition observation, generate

\[
q \sim \operatorname{Dirichlet}(\kappa p),\qquad
C\mid q \sim \operatorname{Multinomial}(n,q).
\]

The control uses `kappa = Inf`, so `q = p`. Finite concentrations are 2000 and
200. With `n = 2000`, the marginal covariance of observed proportions is

\[
\operatorname{Cov}(C/n)=\frac{n+\kappa}{n(1+\kappa)}
\{\operatorname{diag}(p)-pp^\top\}.
\]

Variance inflation relative to multinomial sampling is 1, about 2.00 and about
10.95; moment-matched effective sample sizes are 2000, 1000.5 and 182.7. These
are analytical design descriptors, not estimated weights and not sample sizes
passed to the fitted model. Equal covariance does not make a downweighted
multinomial likelihood identical to a Dirichlet–multinomial likelihood.

The generic concentration parameter is used solely for this stress-test
hierarchy. [Stock Synthesis documents Dirichlet–multinomial composition options](https://nmfs-ost.github.io/ss3-doc/SS330_User_Manual_release.html#composition-error-distribution),
which motivates the comparison, but this is not an implementation of its
parameter interface. The FIMS audit's missing production overdispersed-composition
capability remains missing.

Independent uniforms generate gamma quantiles at each concentration, then a
common set of categorical uniforms generates the counts. This gives the declared
Dirichlet–multinomial marginal law while coupling scenarios. Structural zeros
are preserved. Samples are not nested across concentrations. Catch/survey
observations are identical, but control compositions use the new common uniform
pool and differ from prior pilots; comparisons should remain within this
experiment. The generator's empirical mean/covariance are tested against the
analytical formulas, including negative covariance and zero bins.

Known- and estimated-process-SD fits reuse the calibration machinery, with two
starts for estimated covariance and checked rebuilding before `sdreport`. All
attempts and report failures remain visible. Summaries are separated by generating
scenario, SD mode and regularity stratum. A well-behaved Hessian or an interior
covariance estimate cannot validate a misspecified observation model. The output
contains pointwise fitted-state coverage/width, recovery, hyperparameters,
optimizer diagnostics, and paired differences from the control. It does not test
forecast/derived-quantity coverage, aging error, temporal observation correlation,
unknown growth/selectivity or a correctly fitted overdispersed likelihood.

The default run generates 15 paired datasets, 30 uncertainty reports and 30
covariance starting-point fits. RDS/checkpoint outputs retain true/pseudo
composition probabilities, uniforms, generated data and seeds. Compact tables
and the plot are reproducible without new dependencies; previous numerical
sources and pilots remain unchanged.

The retained [composition-robustness pilot](robustness-pilot/README.md) includes
all attempts and failed rebuilds. Its plot and paired comparisons omit invalid
optimizer reports while retaining their numerical outcomes in raw tables. Run
`diagnose-robustness.R OUTPUT_DIRECTORY` to reproduce the documented cold/warm
inner-state check; that check does not certify global modes or supply replacement
uncertainty intervals.

## Saved-mode uncertainty reconstruction

```sh
Rscript tests/integration/process-recovery/run-mode-handoff.R /tmp/fims-mode-handoff
```

The [single-case mode/DM diagnostic](mode-handoff-pilot/README.md) reconstructs a
fit from plain fixed/random parameter snapshots and checks objective agreement
before and after state uncertainty estimation. It demonstrates both the prior
multinomial cold-start discrepancy and a matched DM composition model using the
new opt-in production likelihood. Snapshot reports are explicitly local and
retain covariance-bound flags; this is not a public fit-persistence API or a
coverage-validation claim. Existing failed pilot rows remain unchanged.

## Replicated DM validation

The subsequent [matched-DM study](dm-validation-pilot/README.md) compares the
multinomial stress comparator with known/estimated composition concentration,
using four process/latent starts and mode-preserving uncertainty reports. It
retains the prior datasets and explicit failure/boundary denominators.

## M-SD/concentration profiles and convergence

The [targeted profile study](dm-profile-pilot/README.md) preserves the replicated
DM baseline and compares stopping controls/initializations on the same snapshots.
It profiles M innovation SD while reoptimizing composition concentrations, other
SDs and latent states, retaining failed attempts and exposing local optimization
gaps without claiming calibrated profile intervals.
