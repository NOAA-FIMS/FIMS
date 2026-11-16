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
