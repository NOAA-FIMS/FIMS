# Six-year recovery pilot

Run: `Rscript tests/integration/process-recovery/run.R /tmp/fims-recovery-pilot 5 6 4301`.
Five datasets (seeds 4301–4305), 14 paired configurations each, 12 ages.
Production revision: `053646a`; benchmark source MD5 and full session/configuration
are in `configuration.R`. FIMS 0.11.0.9000, TMB 1.9.20, R 4.4.3 on Apple ARM64.
The local package was compiled with Apple's clang at `-O1 -g0`.

All 70 fits returned results, with optimizer code zero, small inner/outer
gradients and positive-definite `sdreport` Hessians. Nevertheless, 12 fits hit
the lower SD bound (0.001), leaving 58 qualified under the declared diagnostic
rule. This illustrates why optimizer convergence alone is insufficient.

For joint recruitment/F/M fits, the mortality results averaged over all five
replicates, including boundary fits, were:

| Initial information | Innovation SDs | Mean log-M RMSE | Mean pointwise 95% coverage | Fits with an SD boundary |
|---|---|---:|---:|---:|
| Known first-year anchors | Known | 0.089 | 0.937 | 0/5 |
| Known first-year anchors | Estimated | 0.126 | 0.613 | 2/5 |
| Proper-normal first-year prior | Known | 0.239 | 0.922 | 0/5 |
| Proper-normal first-year prior | Estimated | 0.253 | 0.919 | 4/5 |

Anchored fits score 60 estimated mortality states; proper-initial fits score 72,
including uncertain first-year states. These are different information and
scoring settings, not an isolated comparison of prior forms. Coverage values
are descriptive pilot results, not demonstrated calibration. In particular,
proper-initial intervals can remain broad despite collapsed innovation variance.

The true mortality innovation SD is 0.05. Its joint-fit estimates ranged from
0.001 to 0.243 with known anchors, and from 0.001 to 0.235 with proper
initialization. In the two boundary joint fits with known anchors, mortality
state coverage was 0.017 and 0.050 despite reported positive-definite Hessians.
The retained raw tables show these outcomes rather than dropping them. Very
large log-SD standard errors at boundaries make the associated Wald intervals
unsuitable as evidence of hyperparameter coverage.

Across joint fits, the largest absolute state posterior correlations observed
were 0.560 for recruitment/F, 0.428 for recruitment/M and 0.322 for F/M. These
local, regularized diagnostics do not establish structural identifiability or
explain all variance instability. Even oracle single-process fits sometimes
collapsed F or M variance, so this pilot does not attribute the problem solely
to joint-process confounding.

Median per-fit runtime was 1.013 seconds (maximum 1.622); summed measured fit
time was 74.2 seconds. These timings include uncertainty and covariance
calculation and exclude operating-model simulation and between-fit garbage
collection. They do not establish large-model scalability.

The benchmark checks passed 5 tests / 28 assertions, including the opt-in
estimation/uncertainty integration check. No production code changed.

Next: extend the experiment to longer series, multiple covariance starting
values and mortality-SD likelihood profiles. Compare observation information and
known versus estimated SDs before selecting common annual mortality shocks or
age covariance. Keep covariance parameters fixed when evaluating state recovery
separately; do not treat this short-series pilot as support for freely estimating
all process SDs in an operational assessment.

The compact CSVs retain per-fit diagnostics, metrics, hyperparameters,
correlations and summaries. Full state-level estimates, simulated datasets and
truth are regenerated in `result.rds` by the command above; that binary and the
large pointwise CSV are deliberately not committed.
