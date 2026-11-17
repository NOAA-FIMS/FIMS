# M-SD/concentration profiles and convergence refinement

This follows the [replicated DM pilot](../dm-validation-pilot/README.md), committed
in `b835c15`. No historical data generator, acceptance threshold, failed attempt
or pilot is rewritten. No production C++/R code or dependency changes here.

## Reproduce

From repository root, use the same DM-enabled FIMS build as that pilot:

```sh
# Regenerate the prior plain-data/snapshot baseline if needed:
Rscript tests/integration/process-recovery/run-dm-validation.R /tmp/fims-dm-validation 5 18
# Use an empty output directory; the prior RDS supplies exact data and snapshots:
Rscript tests/integration/process-recovery/run-dm-profiles.R /tmp/fims-dm-validation/result.rds /tmp/fims-dm-profiles
Rscript tests/integration/process-recovery/audit-dm-profiles.R /tmp/fims-dm-validation/result.rds /tmp/fims-dm-profiles
Rscript tests/integration/process-recovery/plot-dm-profiles.R /tmp/fims-dm-profiles
```

The driver checks the baseline's source/production hashes against the working
source before fitting. Each reconstructed model checks fixture/data/truth,
parameter layout, vector lengths and R/F/M density ownership before restoring
full-precision parameters. A profile's reduced TMB map is **not** serialized as a
full-model inference snapshot. Unconstrained refits retain the existing checked
saved-mode reporting pathway. Compact tables/configuration/console/figures are
committed; RDS checkpoints and complete fitted vectors regenerate in the output.

## Design fixed before this run

The seven targeted cases are all five strong-DM datasets with concentration
estimated, plus strong multinomial datasets 3 and 4. This includes the preceding
pilot's three selected gradient failures and its 5.152-NLL start discrepancy.
This targeted study is not a fresh random sample or a new calibration experiment.

Each case crosses three starts with two outer-optimizer control sets:

- Saved fitted fixed parameters and saved latent states.
- Saved fixed parameters and default cold latent states.
- Original parameter template/default states, all process SDs starting at 0.2
  and the original estimated concentration starts of 2000.
- Original controls: `iter.max=300, eval.max=600`.
- Tight controls: `iter.max=1000, eval.max=2000, rel.tol=1e-12, x.tol=1e-10`.

Starts and controls are separate factors (42 planned refit attempts). Bounds
remain process SD [0.001,1], concentration [0.1,100000]. The inner optimizer is
unchanged. Acceptance still requires optimizer code zero, finite results,
projected and inner gradient maxima below 0.001, and repeated objective drift
below 1e-5. The lowest valid refit across both control sets is selected; if none
pass, the best available invalid fit is retained and labeled. Seven warm report
attempts remain planned even if a refit errors. Boundary fits stay distinguished
from interior fits; endpoint agreement is not proof of a globally optimal mode.

For the five strong estimated-DM datasets, M SD is fixed at
`0.001, 0.01, 0.025, 0.05, 0.1, 0.2, 0.4`. At every point, **R/F SDs, all four
concentrations and all latent states are reoptimized** using tight controls,
from saved and cold latent starts (70 planned profile attempts). The other fixed
starting parameters come from the selected free refit. Grid points are fitted
independently, without a one-direction warm-continuation schedule.

For fixed M SD `s`, the target is the local minimum over nuisance parameters
of the random-state Laplace negative log likelihood. Thus this is a one-dimensional
profile with concentration traces, not a two-dimensional likelihood surface.
The delta-NLL reference is the best valid *new* free/grid solution in the same
dataset. A grid solution below every free refit exposes an optimization gap;
invalid attempts have no delta NLL. No failed grid point is silently replaced.
The grid does not span the full allowed upper SD bound (1.0) or locate every
turning point. No chi-square cutoff, profile CI, posterior or global-mode claim
is supplied. A future interval procedure needs boundary and coverage validation.

Local fixed-covariance correlations between log-M-SD and log concentrations are
reported only when the selected report is eligible and its diagonal is positive.
Join `correlations.csv` by replicate to `reports.csv` **filtered to
`case == "estimated_dm"`** for the regular/boundary flags:
Wald correlations at a bound do not establish regular identifiability. Profile
movement of concentration is a diagnostic, not proof of a uniquely identified
process/observation-error decomposition.

## Attribution and files

[SS3's profiling/restart facilities](https://nmfs-ost.github.io/ss3-doc/SS330_User_Manual_release.html)
and [SAM's documented jitter runs](https://fishfollower.r-universe.dev/stockassessment/doc/manual.html)
are conceptual precedents for profiling and start sensitivity (checked 2026-11-17).
The immediate motivation is the reproduced FIMS failures; these controls, TMB
maps, acceptance rules and snapshot experiments are FIMS-specific engineering.
No platform code is copied, and no SS3/SAM executable or equivalent assessment
parameterization is tested. SS3 remains the scientific precedent for the existing
DM family; WHAM/SAM process precedents remain in the maintained change log.

`refits.csv` preserves every control/start, convergence code/message, gradients,
NLL, selection and warnings/errors. `profiles.csv` preserves every fixed-SD/start,
NLL, validity, local reference and free-optimization gap. `points.csv` retains
full selected/profile parameter values, including invalid attempts, so filter by
validity before interpretation (profile keys: replicate/case/fixed SD/start; free
points join the one selected refit per replicate/case). `reports.csv`, `metrics.csv` and
`correlations.csv` retain local uncertainty diagnostics; these are targeted
follow-ups, not revised calibration denominators for the original pilot.
`refit-comparison.csv` is generated by the audit script and preserves the old/new
acceptance flags and within-model objective changes. `audit-output.txt` records
that script's checks.
`configuration.R` includes controls, grid, baseline RDS hash, source/production
fingerprints, starting commit and session information. The console output and
two standalone figures are included with the recorded results.

## Recorded outcomes

The run retained 42 refits, 70 profile attempts and 7 saved-state report attempts. Original attempts and acceptance flags remain in the preceding pilot.

| Controls | Optimizer-valid / 21 | Code-1 messages |
|---|---:|---|
| original | 16 | none |
| tight | 8 | singular convergence (7): 13 |

Best valid free refits across both controls (if none pass, the best invalid result remains labeled). NLL changes compare with the **same model/data** in the original pilot; they are not cross-likelihood comparisons.

| Dataset / likelihood | Old selected valid | New selected valid | Selected controls/start | M SD | NLL change | Max projected gradient |
|---|---|---|---|---:|---:|---:|
| 1 / estimated_dm | TRUE | TRUE | original/saved | 0.0010 | 0.00000000 | 6.91e-05 |
| 2 / estimated_dm | TRUE | TRUE | original/saved | 0.0010 | 0.00000000 | 0.000142 |
| 3 / estimated_dm | FALSE | TRUE | tight/cold | 0.0760 | -0.00000005 | 0.000144 |
| 3 / multinomial | TRUE | TRUE | original/cold | 1.0000 | -0.00000000 | 9.37e-06 |
| 4 / estimated_dm | TRUE | TRUE | original/cold | 0.0224 | -0.00000000 | 0.000139 |
| 4 / multinomial | FALSE | TRUE | original/saved | 1.0000 | -0.00000004 | 4.69e-05 |
| 5 / estimated_dm | FALSE | TRUE | tight/cold | 0.0834 | -0.00000002 | 7.55e-05 |

Local M-SD profiles (true SD 0.05):

| Dataset | Valid profiles / 14 | Delta NLL at 0.05 | Largest saved/cold NLL difference, valid pairs | Free-optimization gap |
|---|---:|---:|---:|---:|
| 1 | 10 | 0.4019 | 1.4e-10 | 0 |
| 2 | 12 | 0.5814 | 2.2e-10 | 0 |
| 3 | 14 | 0.1998 | 2.8e-10 | 0 |
| 4 | 14 | 0.1309 | 2.09e-10 | 0 |
| 5 | 12 | 0.5117 | 1.7e-10 | 0 |

Refined report outcomes:

| Dataset / likelihood | Rebuild status | Eligible | Interior |
|---|---|---|---|
| 1 / estimated_dm | consistent_local_report | TRUE | FALSE |
| 2 / estimated_dm | consistent_local_report | TRUE | FALSE |
| 3 / estimated_dm | consistent_local_report | TRUE | TRUE |
| 4 / estimated_dm | consistent_local_report | TRUE | TRUE |
| 5 / estimated_dm | consistent_local_report | TRUE | TRUE |
| 3 / multinomial | consistent_local_report | TRUE | FALSE |
| 4 / multinomial | consistent_local_report | TRUE | FALSE |

See [M-SD profiles](M-sd-profiles.png) and [concentration traces](concentration-traces.png). Invalid grid values break curves; a free-fit diamond supplies its own local reference. Figures show sampled grid points, not continuous confidence bounds.

## Interpretation and next capability

All seven selected refits and checked warm reports are eligible. The three
previously rejected selected fits now satisfy the unchanged criteria: DM datasets
3/5 select tight/cold fits, and multinomial dataset 4 selects an original-control
saved restart. NLL changes are only about -5.24e-8, -1.86e-8 and -4.47e-8,
respectively. These are stopping/gradient refinements of essentially the same
solutions, not evidence of a materially better scientific model. Original records
remain unchanged. The five refined DM reports average 87.5% M-state coverage;
this is a targeted follow-up on the same five datasets, **not** new replication,
revised original denominators or a calibration certificate. The earlier 81.0%
estimated-DM mean used only three eligible datasets, so it was not an estimate
of the effect of estimating concentration relative to the five-dataset known-DM
mean.

Tight controls are not a default improvement: only 8/21 tight attempts pass,
versus 16/21 original-control attempts. All 13 tight failures return code 1 with
`singular convergence (7)`; some already have small gradients. That optimizer
message alone does not establish statistical rank deficiency. Code-zero original
failures instead miss the projected-gradient threshold. A future general fitting
policy must retain both diagnostics and cannot replace these criteria with
blindly tighter tolerances or treat every code-zero fit as acceptable.

62/70 profile attempts pass; invalid values remain explicit. On paired valid
starts, saved/cold NLL differences are at most 2.8e-10. No sampled profile point
beats the best new valid free fit (reported gaps zero). True M SD 0.05 is only
0.131–0.581 NLL units above the respective best fits. Two DM datasets still favor
the lower SD bound 0.001. Thus the data provide weak discrimination among small
M variances; a bound estimate does not demonstrate that the generating process
has no annual variation. This conclusion is local to the sampled grid and model,
not a calibrated confidence set or global proof.

There is little evidence of a pronounced M-SD/concentration ridge in this local
region: at valid grid points with M SD at most 0.1, each composition concentration
changes by at most **2.32%** relative to its selected free value. Among the three
interior DM reports, absolute local log-M-SD/log-concentration correlations are at
most **0.110**. Bound-report correlations are not used for that statement. These
diagnostics support weak M-variance information in these datasets rather than a
large local tradeoff with concentration; a full surface and broader designs
would be needed for a general claim.

Basin sensitivity remains in strong multinomial dataset 3. Original-control
saved/cold starts find NLL about 7645.667, while its template start finds about
7650.819 and also passes the original checks. The 5.152-NLL split survives; tighter
controls do not establish global optimality or repair observation misspecification.
The correct DM observation family and checked state ownership remain separate
requirements from optimization refinement.

The recommended next capability is **production DM observation simulation**,
using the existing fleet/type concentration configuration, count/sample-size
contract, seeded RNG, structural-zero/missing-row rules and restoration lifecycle.
Validate its sampling moments and limiting behavior independently before using it
for broader simulate/refit studies. Keep M-SD sensitivity and multiple-start
checks; neither a new sampler nor extra covariance/priors should be presented as
a cure for this pilot's weak variance identification. Derived/forecast uncertainty
still requires its own propagation and calibration study. No such production
extension is implemented by this investigation increment.

## Verification

Three tests / 19 assertions passed with `RUN_SLOW_TESTS=true`, without failures,
warnings or skips. They cover absolute mapped/free objective agreement within
1e-5, constant M-SD mapping, ownership/input rejection, full-snapshot reporting,
free-optimization gap detection and all-failed planned rows. The complete pilot
audit checked 42 refits, 70 profiles, seven reports and 21 process-metric rows;
one selection per case; exact mapped M values; valid-only reference construction;
and baseline/execution/production hashes. Plot scripts parse, local links and
whitespace checks pass, and final figure exports were visually checked. No
production code, dependency, public fit policy or historical pilot was changed.
