# Scientific capability roadmap

Investigation draft, 2026-11-16. This orders proposed work; it does not authorize
production changes. See the [gap matrix](scientific-capability-gaps.md) for
baseline evidence and [process design](stochastic-process-architecture.md) for
mathematical contracts. No new dependency, commit, push or PR is part of this pass.

## Current increment

The opt-in IID/AR1 density and recruitment connection are implemented on this
branch, with focused mathematical, simulation and integration tests. Package 1
remains subject to review and wider assessment recovery/coverage and scaling
validation. Package 2 now has a bounded implementation: Gaussian recruitment
path simulation/continuation, recomputed trajectories and normal/lognormal/
multinomial observations at supplied parameters and initial states. Joint
parameter/state uncertainty and generalized scheduling remain open. Package 3
now includes independent annual fleet log-F random walks, proper/fixed initial
rules, inference, process-aware simulation and shared innovation SD groups.
Mortality and selectivity targets remain open. Packages 4 onward are not implemented.
The initial investigation and recruitment/fishing upgrades were committed in
`9ea49e1`; subsequent upgrades are committed after validation at the user’s request.

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
| Parallel B | Dirichlet-multinomial composition likelihood | High / integrated assessment | Medium | Existing likelihood/parser extension points | Composition count/sample-size contract | Multinomial limit, dispersion recovery, zero handling and sensitivity. |
| Later 1 | Environmental predictors, then covariate observation error | Medium / ecosystem applications | Medium–large | Needs shared predictors and latent covariate module | Process layer; lag and missing-year semantics | beta=0 limit; covariate/state recovery and predictive checks. |
| Later 2 | Fixed productivity regimes and richer biological means | Medium / changing productivity | Medium | Recruitment mean extension | Explicit regime maps and reference-point definitions | One-regime limit, boundary-year checks, identifiable parameters. |
| Later 3 | Sex/area/season partitions and movement | Medium overall; high for particular stocks | Large | Empty Subpopulation and pooled-growth constraints need substantial work | State/index contract and process schedule | Mass conservation, pooled limits and consistent observation aggregation. |
| Later 4 | Multiple growth patterns and tagging | Stock-dependent | Large | Needs partition and observation additions | Later 3, cohort bookkeeping | Mixture/recovery predictions, tag loss/reporting identifiability. |
| Later 5 | Closed-loop MSE interface | Medium / management use | Large | Build on assessment and forecasting; consider external orchestration | 2, reproducible simulation, HCR/failure contracts | Repeated observe-assess-decide-simulate cycles and failure scenarios. |
| Deferred | General ARMA, nonseparable covariance, alternative backend | Unproven need in this comparison | Potentially large | Reassess after simpler processes | Specific scientific use case and benchmarks | Evidence that added flexibility solves a demonstrated problem. |

Parallel A/B are independent work packages, not instructions to spawn agents or
make concurrent implementation changes. They need separate scoped changes and
validation; neither requires waiting for a complete latent-abundance engine.

## First proposed implementation scope

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
are reused. The next increment is mortality-aware simulation and conditional
forecasting; common annual shocks, age covariance and selectivity processes
remain open. Assessment-scale recovery, identifiability and scaling benchmarks
are still required.
