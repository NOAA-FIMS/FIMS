# Scientific capability roadmap

First drafted 2026-11-16; current branch status updated 2026-11-17. This orders
proposed work and records completed increments; it is not authorization for
unrelated production work. See the [gap matrix](scientific-capability-gaps.md)
for baseline evidence, [process design](stochastic-process-architecture.md) for
mathematical contracts, and the maintained [change/inspiration log](scientific-capability-change-log.md)
for current scope, commit history and platform attribution.

## Current increment

Implemented on this branch: opt-in IID/AR1 recruitment, process-aware simulation
and conditional forecasting, annual fleet log-F random walks and shared innovation
SD groups, independent age-specific annual log-M random walks and mortality-aware
simulation, and opt-in Dirichlet-multinomial composition inference. A benchmark
snapshot pathway preserves local fitted modes across uncertainty reconstruction.
Selectivity process extensions, multivariate covariance, latent older-age abundance
states, generalized partitions and a new backend are not completed by these changes.

Recovery, profile, information, calibration and misspecification pilots are
recorded with explicit limitations. The current single-case matched-DM result
is not evidence of replicated dispersion recovery or calibrated intervals.
Replicated matched-DM validation is now complete as a limited pilot: 160 starts,
40 selected fits, 80 cold/warm reports. Observation-family improvements coexist
with variance bounds, gradient failures and alternate basins. **Next: profile
M-SD/concentration identifiability and refine convergence**, preserving the
original thresholds and failures. The [ledger](scientific-capability-change-log.md#next-task-m-sdconcentration-identifiability-and-convergence)
and [pilot report](../tests/integration/process-recovery/dm-validation-pilot/README.md)
outline the evidence and scope. Upgrades are committed after validation at the user's request;
pushes remain separate authorized actions.

## Prioritization

Prefer broadly useful features with a clear scientific interpretation, a testable
small increment and good fit to FIMS interfaces. Distinguish implementation cost
from scientific importance. A high-priority state-space model can still follow a
smaller process-density extension because it needs that foundation.

Cost estimates below are relative, not calendar promises. None assume that every
assessment has enough information to estimate another variance or correlation.

| Order | Work package | Importance / generality | Relative cost | Architecture fit | Depends on | Exit evidence |
|---|---|---|---|---|---|---|
| 0 | Set process semantics: working scales, means, initial distributions, membership and fixed years | High / all stochastic models | Small–medium | Existing density/registration boundary | Review of this design | Examples distinguish integration, conditioning and absent density terms; no ambiguous variance fields. |
| 1 | Reusable IID/AR1 Gaussian density; opt-in recruitment configuration | High / recruitment, later biology and F | Medium | Strong: existing log_r/log_devs and DensityComponentBase | 0 | Normalized likelihood, AD checks, rho=0 compatibility, recovery and full-model scaling. |
| 2 | Process-aware simulation and stochastic forecasts | High / forecasts and validation | Medium–large | Existing simulation hooks; orchestration needed | 1 and simulation semantics | Every observation draw traces to the sampled biological trajectory; uncertainty components reported separately. |
| 3 | RW1 and grouped effects for fleet F, log M and selectivity | High / broad | Medium | Existing target registration | 1; 2 for validation | Proper initial-level treatment; parameter-sharing tests; confounding diagnostics. |
| 4 | Multivariate and separable age-year covariance | High / broad | Medium–large | Density extensions plus explicit axis metadata | 3; validated indexing | Dense covariance oracle, dimension checks, positive-definite parameterization and measured factorization costs. |
| 5 | Latent abundance-at-age transition model | High / state-space assessments | Large | Requires state storage and transition changes | 1–4 as needed, not every covariance option mandatory | Cohort/plus-group recovery, deterministic-mode equivalence, pinned WHAM/SAM comparisons. |
| Parallel A | Fractional-year survey timing | High / surveys | Small–medium | Local observation-operator extension | Defined year/rate units | Analytical tau=0/0.5/1 results and unchanged default timing. |
| Parallel B | Dirichlet-multinomial inference implemented; broader validation/simulation remain | High / integrated assessment | Medium | Existing likelihood/parser extension points | Composition count/sample-size contract | Multinomial limit, dispersion recovery, zero handling and sensitivity. |
| Later 1 | Environmental predictors, then covariate observation error | Medium / ecosystem applications | Medium–large | Needs shared predictors and latent covariate module | Process layer; lag and missing-year semantics | beta=0 limit; covariate/state recovery and predictive checks. |
| Later 2 | Fixed productivity regimes and richer biological means | Medium / changing productivity | Medium | Recruitment mean extension | Explicit regime maps and reference-point definitions | One-regime limit, boundary-year checks, identifiable parameters. |
| Later 3 | Sex/area/season partitions and movement | Medium overall; high for particular stocks | Large | Empty Subpopulation and pooled-growth constraints need substantial work | State/index contract and process schedule | Mass conservation, pooled limits and consistent observation aggregation. |
| Later 4 | Multiple growth patterns and tagging | Stock-dependent | Large | Needs partition and observation additions | Later 3, cohort bookkeeping | Mixture/recovery predictions, tag loss/reporting identifiability. |
| Later 5 | Closed-loop MSE interface | Medium / management use | Large | Build on assessment and forecasting; consider external orchestration | 2, reproducible simulation, HCR/failure contracts | Repeated observe-assess-decide-simulate cycles and failure scenarios. |
| Deferred | General ARMA, nonseparable covariance, alternative backend | Unproven need in this comparison | Potentially large | Reassess after simpler processes | Specific scientific use case and benchmarks | Evidence that added flexibility solves a demonstrated problem. |

Parallel A/B are independent work packages, not instructions to spawn agents or
make concurrent implementation changes. They need separate scoped changes and
validation; neither requires waiting for a complete latent-abundance engine.

## Original proposed implementation scope (historical)

The first scientific extension is an opt-in stationary AR(1) recruitment process
built on a reusable density. Include IID as its exact rho=0 baseline, a stationary
initial distribution, explicit innovation SD, transformed correlation, and
configuration for an existing target and declared mean.

Start with contiguous annual series and no internal fixed AR1 states. Reject
unsupported membership patterns clearly in the new pathway. Keep legacy IID
behavior, including its documented fixed-element warning, until a separate
scientific decision changes it. Include a simulation/recovery harness sufficient
to validate the new process; a general stochastic forecast API is package 2.

The initial change should not add latent older-age abundance, rewrite the event
schedule, replace existing distributions, alter recruitment defaults or introduce
Quadra. Those are different increments with different validation requirements.

Suggested sequence within the scoped change:

1. Specify configuration and examples for deviations versus direct log-R targets.
2. Add mathematical density and independent analytical checks.
3. Connect Rcpp registration and opt-in R initialization; validate dimensions and ownership.
4. Verify legacy IID/projection compatibility and seeded process simulation.
5. Run recovery and scaling experiments; report limitations before widening scope.

Review the concrete API and variance/mean convention before production edits.
The investigation documents were committed with the initial process upgrades in `9ea49e1`.

## Benchmark and validation plan

Use three levels of evidence:

| Level | Benchmark | Quantities to compare |
|---|---|---|
| Mathematical | Small IID/AR1/RW/MVN distributions with independent covariance calculations | Log likelihood including constants, gradients, Hessians, simulated moments. |
| Synthetic assessment | Shared annual age-structured model with matched recruitment, F, M, observations and plus group | Parameter/state recovery, derived SSB/F, interval coverage, convergence and factorization costs. |
| Framework example | Pinned WHAM recruitment-only case, then a matched SAM state/F case; AMAK/JJM cases later | Matched model predictions/objectives before comparing fitted advice quantities. |

Do not promise identical estimates from different default configurations.
Reconcile recruitment age/lag, survey timing, variance scales, initial abundance,
penalties, data weights, zero rules and terminal reporting first. Archive source
versions, input transformations and random seeds. Report failed fits alongside
successful fits, plus Monte Carlo uncertainty for estimated coverage and bias.

Assess confounding explicitly: F versus selectivity scale, M versus survival
innovations, recruitment mean versus autocorrelation, and process versus
observation variance. Do not initially estimate every stochastic component in one
assessment. Add one process at a time with sensitivity analyses.

## Review decisions and remaining research

The documentation recommends innovation SD for 1D AR1, explicit process membership,
proper initial states and a density-based first increment. Before implementation,
review the arithmetic-mean/median recruitment convention and whether the first
scientific example should use deviations or direct log R. Preserve both existing
representations for compatibility and measure their full-model behavior.

The six-framework matrix is complete for the requested topic list, but individual
reference coverage is deliberately uneven where primary evidence is unavailable.
AMAK source directories were inaccessible during this audit; inspect a pinned
source snapshot before claiming general process support. The JJM guide is an
IMARPE proposal; validate the operational configuration/version with its source.
Casal2 timing details came from indexed manual text after direct PDF retrieval
failed. No verified general ARMA or nonseparable-process reference was established.
These limitations should be resolved before expanding those claims, rather than
filling the matrix with inferred functionality.

Quadra remains a future backend question. Investigate it only after defining a
backend-neutral process contract and obtaining measured TMB bottlenecks; assess
AD, sparse factorization and Laplace requirements without adding a dependency to
this investigation.

Shared fleet innovation SD groups now use validated `sd_group` labels and the existing TMB factor-map machinery. This reduces covariance parameter dimension without coupling latent trajectories. No new native backend or dependency is required.

Natural mortality inference now supports independent age-specific annual log-M
random walks with one common innovation SD and explicit fixed/proper-normal
initialization. The existing age-year Population vector and TMB registration
are reused. Mortality-aware simulation and conditional
forecasting are also implemented; common annual shocks, age covariance and selectivity processes
remain open. Assessment-scale recovery, identifiability and scaling benchmarks
are still required.

Mortality simulation now applies complete sampled grids before dynamics, with
conditional history, explicit process modes and atomic restoration. The next
scientific design decision is common annual mortality shocks versus correlated
age-specific innovations; assessment recovery and identifiability benchmarks
should guide that choice before more covariance structure is added.

The first recovery/identifiability benchmark is implemented in
`tests/integration/process-recovery/`. Five six-year datasets and 14 paired fit
configurations produced 70 fits, with 12 SD-boundary cases despite optimizer
convergence and positive-definite reported Hessians. Joint mortality SD estimates
were unstable; known-SD state recovery was substantially more reassuring than
free covariance estimation. The retained pilot includes all fits and separates
boundary results. Before adding mortality covariance, extend this experiment to
longer horizons, multiple starts and mortality-SD profiles, and vary observation
information. Five replicates do not validate interval coverage or scaling.

The longer-series/multiple-start/profile extension is now complete (see
`tests/integration/process-recovery/sensitivity-pilot/README.md`). It retained
132 attempts at 6/18 years, with 129 passing optimizer checks and at least two
valid starts per selected case. Valid objective spreads were below 0.00004,
yet two of three 18-year datasets still estimated M innovation SD at the lower
bound under each initialization setting. The first longer dataset had an
interior optimum but broad relative likelihood support near the lower bound.
This small experiment supports observation-information and process-SD sensitivity
experiments next; it does not support adding unrestricted covariance merely to
address variance instability. No coverage or general scaling claim follows.

The paired observation-information pilot is complete in
`tests/integration/process-recovery/information-pilot/`. Three shared 18-year
trajectories were fitted under baseline/improved survey and composition designs.
94 of 96 attempts passed optimizer checks, and every selected case had a valid
start. With proper initial uncertainty, composition size 2000 versus 200 reduced
mean log-M state RMSE by about 21%; combined precision/sample-size improvements
reduced it by about 23%. Survey precision alone changed mean RMSE little, and
variance estimates remained design/replicate dependent. Profiles were sharper
under combined observations but favored below-truth SDs in the profiled dataset.
Next, assess calibration and process-SD sensitivity with more replicates under
informative composition designs before adding mortality covariance complexity.
This is not evidence of operational coverage or a sampling-cost recommendation.

The replicated interval-calibration pilot is complete in
`tests/integration/process-recovery/calibration-pilot/`. Thirty informative,
18-year datasets span true M SDs 0.025/0.05/0.10. All 60 uncertainty reports
succeeded; 58/60 covariance starts passed checks, with a valid selected start for
every estimated fit. Known-SD mean M coverage was 93.3–94.5%; estimated-SD coverage
was 92.7–93.7%, with replicate-level MCSEs around 1–1.5 percentage points. Paired
differences were modest relative to Monte Carlo uncertainty. Five of ten fits
at the smallest true M SD reached the lower bound; larger SDs had no boundaries.
These results support continued sensitivity testing rather than a claim of
operational calibration. Next validation targets include more replication,
misspecification and derived-quantity/forecast uncertainty, before unrestricted
mortality covariance or assessment recommendations.

The composition-misspecification pilot is complete in
`tests/integration/process-recovery/robustness-pilot/`. With the fitted multinomial
size held at 2000, controlled extra composition variation reduced known-SD mean
M coverage from 94.8% to 89.8%/57.4% across five paired 18-year datasets. Estimated
process SDs absorbed observation variation; the strongest case reached the
benchmark M-SD upper bound and yielded no optimizer-valid uncertainty reports.
Four cold covariance-point rebuilds failed, while one surviving numerical report
had invalid optimization. A diagnostic reproduced a cold/warm inner-solution
difference at matching covariance values. These outcomes strengthen the priority
of a correctly specified overdispersed composition likelihood, with dispersion/
multinomial-limit/zero-bin/AD validation, and a separate mode-consistent uncertainty
handoff investigation. Production DM support remains missing; no new production
code or dependencies were introduced in the stress benchmark.

An opt-in Dirichlet-multinomial composition likelihood is now implemented for
inference, with one fixed/estimated concentration per fleet/type and normalized
row contributions. Existing sample-size formulas and multinomial defaults remain
unchanged. Missing rows, structural zeros, concentration registration and fleet
reporting are covered. Production DM simulation, a dedicated dispersion-sharing
interface, broad assessment recovery/coverage and extreme-concentration accuracy
remain separate extensions. A mode-consistent benchmark uncertainty handoff is
now evaluated on the existing stress data; a local report must not be mistaken
for a global solution or calibrated intervals.


The saved-mode benchmark pathway now serializes plain fixed/random vectors,
checks inputs/layouts and validates marginal objectives before and after state
uncertainty estimation. On one strongest-stress dataset, warm reconstruction
reproduces the multinomial local fit while cold reconstruction differs by 0.567
NLL units. A matching fixed-concentration DM fit agrees cold/warm and reduces
log-M RMSE, but its M SD reaches the lower bound and pointwise coverage is only
75.5% in that dataset. The numerical handoff is addressed locally; broad
calibration, joint dispersion estimation and global-mode behavior remain open.
Next, repeat the matched-likelihood stress comparison across replicates and
latent/covariance starts before extending uncertainty to forecasts/derived outputs.


The replicated matched-DM pilot now fits the five existing datasets per stress
level using four covariance/latent starts. 134/160 attempts and 37/40 selected
optimizers pass checks; all saved-state reports reconstruct consistently, but
only 36/40 meet the inference eligibility checks. Strong known-DM M coverage is
87.5% over five eligible datasets; the paired improvement versus multinomial is
28.7 pp (MCSE 10.2 pp, three pairs). Strong estimated-DM has only three eligible
reports and one paired comparison, so it cannot establish calibrated uncertainty
or the cost of estimating concentration. A 5.152-NLL valid-start spread confirms
remaining basin sensitivity. Next, profile coupled process/observation variances
and refine optimizer convergence while retaining the original failed attempts.
Production DM sampling is a separate extension; more covariance is not justified
as a cure for weak variance identification. Full results and concentration-owner
summaries are in `tests/integration/process-recovery/dm-validation-pilot/`.
