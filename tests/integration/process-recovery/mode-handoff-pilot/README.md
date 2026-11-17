# Mode-preserving uncertainty handoff and matched DM diagnostic

```sh
Rscript tests/integration/process-recovery/run-mode-handoff.R /tmp/fims-mode-handoff
```

Run from the repository root against the installed DM-enabled FIMS. This is one
controlled 18-year dataset (process seed 4301), not a replicated coverage study.
It recreates the prior strongest overdispersion scenario: independent composition
concentrations 200 and nominal sample size 2000. Catch/survey data and true
recruitment/F/M paths are unchanged. The two cases fit the same data using the
multinomial likelihood or the matching DM likelihood with concentration held at
its known generating value. All three process SDs are estimated from common
start 0.2, with the prior benchmark's [0.001, 1] bounds.

Base production commit: `b3e79b2`. Numerical/driver and relevant production-source
hashes, session versions and inputs are recorded in `configuration.R` and the
regenerated RDS output. The run used R 4.4.3/TMB 1.9.20 on Apple ARM64. No earlier
pilot or benchmark source was rewritten. A final native metadata guard emits
only recognized composition-type enums (or null), so arbitrary low-level notes
cannot corrupt JSON.

## Snapshot contract

`mode_fit_snapshot()` saves full-precision fixed parameters, random-effect states,
reference joint/marginal objectives, parameter layouts, model inputs and optimizer
diagnostics as plain R values. No native model or live TMB object is serialized.
`mode_report_snapshot()` reconstructs a fresh model, checks its data/configuration,
layout and vector dimensions, then optionally restores both fixed and random
vectors. The reference joint objective and marginal objective are checked before
uncertainty estimation; the marginal objective is checked again afterward.
A mismatch returns an explicit status and no replacement state intervals.

Snapshots are experimental benchmark objects for this joint-process model, not
a new public `FIMSFit` persistence API. They require identical input templates
and data. Registry IDs are transient; field/owner ordering is checked and the
reference joint calculation is independently probed. Invalid/truncated snapshots
are rejected rather than recycled into native vectors.

State-only reports use `sdreport(skip.delta.method = TRUE)` to avoid computing
unused ADREPORT-derived-quantity uncertainty. Tests compare random-state SEs and
fixed covariance with the full delta-method call and find agreement. This does
not supply uncertainty for spawning biomass, forecasts or other derived outputs.

## Observed outcomes

| Likelihood | Rebuild start | Marginal difference from saved fit | Status |
|---|---|---:|---|
| Multinomial | Cold default states | +0.567461 | Withheld: mismatch |
| Multinomial | Saved states | 0 | Consistent local report |
| Matched DM, concentration 200 | Cold default states | about 1.3e-10 | Consistent local report |
| Matched DM, concentration 200 | Saved states | 0 | Consistent local report |

All reference joint-objective probes agreed in this check. Successful reports
also agreed after uncertainty estimation, and warm reports retained the saved
states to numerical precision. The cold multinomial mismatch is not hidden by a
larger tolerance. Restoring the saved state fixes this handoff, but it does not
establish a globally preferred conditional solution or a globally correct
Laplace approximation.

The three computed reports give these **single-dataset local** M-state metrics:

| Likelihood/report | M innovation SD | Log-M RMSE | Pointwise interval coverage |
|---|---:|---:|---:|
| Multinomial, warm | 1.0 (upper bound) | 1.215 | 66.7% |
| Matched DM, cold/warm | 0.001 (lower bound) | 0.292 | 75.5% |

The matched observation model reduces state distortion here and removes the
observed cold/warm discrepancy, but neither variance estimate is interior. Both
reports remain nonregular at a covariance bound; the DM result is not nominally
calibrated in this one dataset. Concentration is known, so this is not evidence
for joint assessment-scale dispersion recovery. Full metadata and raw metrics
retain those limitations rather than treating a consistent rebuild as an
inference guarantee.

Endpoint checks also do not certify that every covariance-Hessian perturbation
followed one basin. Alternate starts, profiling and scientifically appropriate
likelihoods remain necessary. Public `fit_fims()` is unchanged; the new pathway
addresses the benchmark's reconstruction handoff and demonstrates the required
state ownership pattern for future generalized persistence.

## Validation and next steps

The final focused run passed 16 tests / 85 assertions, including the density,
existing distribution and handoff tests, with no failures/skips. Handoff checks
cover RDS serialization across native model clearing, layout/input/truncation
rejection, before/after objective agreement and equality of state-only/full
`sdreport` state uncertainty. The production DM upgrade also had a broader
94-test / 554-assertion run with two existing CRAN-gated reporting skips, and a
standalone linked-data C++ smoke test. GoogleTest remains registered but was not
run because its local dependency cache is unavailable.

Next, repeat the matched-likelihood comparison across the existing stress data,
including estimated concentration and multiple covariance/latent starts. Then
assess coverage, dispersion identifiability and derived-quantity/forecast
uncertainty. Mode preservation and a better observation family address different
problems; neither should be substituted for those validation steps.

`result.rds` regenerates the complete snapshots, inputs, truth and reports in the
output directory. Only compact diagnostics, point values, metrics and provenance
are committed here.
