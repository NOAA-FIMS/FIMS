# Stochastic process architecture for FIMS

Design proposal, 2026-11-16; baseline `f028bff`. No proposed names or configuration
fields in the design sections are guarantees of an existing public API. The first
implementation increment is described below. Evidence IDs refer to the source registry in
[scientific-capability-gaps.md](scientific-capability-gaps.md).

## First implementation increment

The working branch now includes `GaussianProcessDistribution`, a reusable
normalized IID/stationary AR(1) density, and opt-in recruitment setup:

```r
data <- FIMSFrame(data_big)
parameters <- setup_default_parameters(data)
initialized <- initialize_fims(
  parameters,
  data,
  recruitment_process = list(
    type = "ar1", rho = 0.3, estimation_type = "fixed_effects"
  )
)
fit <- fit_fims(initialized)
```

`rho` is initialized on the natural scale and estimated as `atanh(rho)` in the
`unconstrained_rho` parameter. The existing scalar recruitment `log_sd` is the
**innovation log-SD**. `list(type = "iid")` opts into the same component with
constant zero correlation; omitting `recruitment_process` retains the old path.
The lognormal helper is unchanged and is not an AR1 option.

This increment accepts one contiguous annual series of estimated `log_devs` or
`log_r`, with all target rows declaring a Dnorm process. It rejects fixed elements,
multiple SD values, missing/reordered annual targets and unsupported covariance
parameter roles before model creation. Direct log R uses the stock-recruit
log-mean vector; deviations use zero. Existing registration and population
transitions retain ownership of the targets.

The low-level component exposes `evaluate()` and `simulate()` on supplied
`observed_values`/`expected_values`, with `log_sd`, `unconstrained_rho` and
`set_process("iid"/"ar1")`. Its simulation returns a path around supplied means
without mutating a registered model. It does **not** perform unconditional
assessment simulation or stochastic forecasting. Dynamic stock-recruit means must
be recomputed by the trajectory driver described below.

The implementation resides in
[the density](../inst/include/distributions/functors/gaussian_process_lpdf.hpp),
[the Rcpp interface](../inst/include/interface/rcpp/rcpp_objects/rcpp_gaussian_process.hpp),
and [R configuration](../R/gaussian_process.R).
[Focused tests](../tests/testthat/test-gaussian-process.R) cover a dense covariance
oracle, stationary simulation, process-only covariance recovery, IID equivalence
for both recruitment representations, AD gradients, reporting and a small
Laplace-integrated assessment. Full assessment recovery/coverage and broader
performance experiments remain future validation work, not established claims.

### Validation recorded for this increment

- Isolated `R CMD INSTALL` succeeded with the system Apple compiler and a
  temporary Makevars override. The default Homebrew compiler failed on system
  math headers before compiling the change; no repository toolchain settings
  were changed.
- Nine focused R tests passed, with 79 assertions, including correlation
  gradient/curvature checks and 30 seeded process-only recovery fits.
- Existing distribution-helper, distribution-interface, initialization and
  estimate-reporting tests passed. Two slow estimate-reporting tests retained
  their existing CRAN skips.
- A standalone C++17/STD_LIB smoke executable passed density, deterministic
  simulation-scale and invalid-shape checks. A GoogleTest target is registered
  for the new density; the GoogleTest suite itself was not run in this pass.
- Changed R files parse, changed Rd files pass `tools::checkRd()`, local document
  links resolve, and `git diff --check` passes.

These checks establish the implemented pathway and its mathematical behavior;
they do not establish full assessment coverage or production forecast performance.

## Process-aware simulation and continuation increment

`simulate_fims()` now draws recruitment paths, evaluates them through the existing
annual population transitions, and generates observations from those same paths.
`forecast_fims()` uses the same engine with an explicit continuation year:

```r
sims <- simulate_fims(
  parameters, data,
  recruitment_process = list(type = "ar1", rho = 0.4),
  nsim = 20, seed = 19
)

# data and parameters must already contain the desired future horizon and
# fishing-mortality/biology scenarios; this call does not extrapolate inputs.
forecast <- forecast_fims(
  parameters, data,
  recruitment_process = list(type = "ar1", rho = 0.4),
  from_year = 25, nsim = 20, seed = 19
)
```

The example continuation year must be a recruitment year in the supplied model.
Supply fitted parameter values explicitly when forecasting from estimates.
This API does not recover a model from a stored fit's live TMB object, which may
refer to a different active model after another initialization.

Three process modes distinguish scientific operations. `"simulate"` draws a
stationary first residual for a whole path, or conditions an AR1 continuation on
the last supplied residual. `"fixed"` retains supplied recruitment states for
observation-only simulation. `"mean"` uses zero future innovations, propagating
the conditional residual mean. This last option is on the log/residual scale;
it is not an arithmetic mean biomass forecast. With an explicit continuation
year, earlier observations are retained. Whole-series simulation also draws
measurements in the initial model year, conditional on initial abundance.

For direct `log_r`, the native evaluator recomputes the stock-recruit log mean
from simulated spawning biomass at each annual transition before adding the
sampled residual. For `log_devs` the same residual path enters the existing
recruitment calculation. Initial abundance is held fixed. Cohort survival and
plus-group aggregation use the existing implementation; the extra terminal
report year retains its existing biological-mean recruitment rule.

The native evaluator borrows a residual path only during a double-valued model
evaluation, copies the resulting states/expectations, then restores target
values and recomputes the original trajectory. It does not retape the objective,
resize registered parameter vectors, finalize fit interfaces or mutate supplied
observations. Invalid dimensions/nonfinite residuals are rejected before the
path is installed; error cleanup clears the borrowed pointer and restores values.
The initializer's opt-in process sidecar lets the driver reuse the registered
Gaussian component rather than create an unlinked density in the active model.

Observation generation supports normal/lognormal scalar streams and multinomial
age/length compositions with the same fleet-specific bins. Output distinguishes
the model prediction, density location, arithmetic observation mean and draw.
A lognormal model prediction is a median; its arithmetic observation mean
includes the variance correction. Counts retain their observed sample totals;
missing count observations need a declared future design size. Proportions use
the declared sample size. Missing uncertainty formulas are labeled, not replaced
with assumed SDs or sample sizes. Entire compositions are drawn together.

All process paths are sampled before observation noise. With the same seed,
toggling observation error therefore leaves trajectories unchanged. An explicit
seed is scoped and restores the caller's RNG state; the return includes seed,
RNG kind, initial RNG state, configuration, dimensions and replicate outputs.
Parameter uncertainty, uncertainty in initial abundance, and other stochastic
processes are explicitly marked excluded. Future catch inputs are observation
designs, not catch constraints solved by this API.

This increment is conditional on supplied parameters and initial states. Joint
parameter/state draws, automatic scenario extrapolation, generalized process
scheduling, and closed-loop MSE are not implemented. Existing TMB simulation
methods are unchanged; the supported entry points are the new R drivers.

### Simulation validation recorded

The final focused regression run passed 26 tests and 193 assertions with no
failures, errors or skips: 73 assertions for process-aware simulation, 79 for the
Gaussian process increment, and 41 existing distribution-helper/interface checks.
A clean isolated package installation succeeded. Changed R/Rd files and local
links were checked; the standalone C++ smoke check includes conditional
continuation. The registered GoogleTest suite was not run.

The simulation checks cover seeded path/noise separation, initial-year observation
draws, conditional prefix preservation, dynamic direct-log-R means, cohort and
plus-group equations, native parameter restoration, composition sample totals,
missing future sampling designs and RNG restoration including an absent caller
seed. This is not a coverage study for fitted assessment parameters.

Returned population and fleet vectors follow their existing native layouts.
`configuration$years` includes the extra terminal population report year;
fleet observation vectors span the configured model years before that extra
report year. Ages, input bin rows and per-observation positions are returned so
predictions and generated observations can be matched without guessing bin order.

## Annual fleet log-F random-walk increment

An opt-in `fishing_process` list now attaches independent random walks to selected
catch fleets. For example, after constructing `parameters` and `data`:

```r
fleet_rows <- parameters$module_name %in% "Fleet" &
  parameters$fleet %in% "fleet1" & parameters$label %in% "log_Fmort"
parameters$estimation_type[fleet_rows] <- "random_effects"
fishing <- list(fleet1 = list(
  type = "rw1", sd = 0.2, estimation_type = "fixed_effects",
  initial = list(type = "normal", mean = log(0.2), sd = 0.5)
))
initialized <- initialize_fims(parameters, data, fishing_process = fishing)
fit <- fit_fims(initialized)

forecast <- forecast_fims(
  parameters, data, recruitment_process = list(type = "ar1", rho = 0.3),
  from_year = 25, nsim = 20, seed = 47,
  fishing_process = fishing, fishing_mode = "simulate"
)
```

The continuation year and all required future biological inputs must already be
in the configuration. Use fitted values explicitly when forecasting at estimates:
update annual states in the parameter tibble and innovation SDs in the fishing
specification. Recruitment correlation is likewise supplied in its process
specification; none of these values are recovered automatically from a stored fit.
The process specification does **not** change target estimation flags. Set
`random_effects` for latent states integrated by TMB, or use `fixed_effects` for
a penalized fixed-effect fit. The scalar innovation log-SD may be held constant
or estimated as a fixed-effect covariance parameter. Each selected fleet has a separate SD unless its specification includes an
`sd_group` string. Fleets with the same group share one innovation SD while
retaining independent innovations and separate initial distributions. Members
must provide identical SD values and estimation types. Estimated groups produce
an initializer `map$p` factor consumed by `fit_fims()`; direct `TMB::MakeADFun()`
callers must pass `map = initialized$map`. Constant groups validate equality but
need no map. Replacing this map discards the sharing; any additional manual maps
must preserve its tied entries. Simulation uses the validated common supplied SD,
not an automatic extraction from a fitted object. Group labels are retained in
simulation configuration, and fitted output retains a parameter row for each
fleet with mapping-aware estimates. Sharing an SD does not model cross-fleet
innovation correlation. No cross-fleet covariance specification is provided.

For example, set `fishing$fleet1$sd_group <- "commercial"` and
`fishing$fleet2$sd_group <- "commercial"` before initialization, with identical
`sd` and `estimation_type` entries. Initial-rule means and SDs may differ.
A singleton group behaves like an ungrouped fleet.

For `x[f,t] = log F[f,t]`, the model is

\[
 x_{f,t} \mid x_{f,t-1} \sim \mathcal N(x_{f,t-1},\sigma_f^2).
\]

Two explicit initial rules are supported:

- `initial = list(type = "normal", mean = m0, sd = s0)` contributes a proper
  `Normal(m0,s0^2)` density for the first log-F state. These initial-distribution
  constants are not estimated. Every target state must be estimated.
- `initial = list(type = "fixed")` conditions on the first supplied log-F value.
  Its parameter row must be `constant`; all later rows must be estimated.
  The initial density contribution is zero because the likelihood is conditional
  on that state. Native registration checks also reject an estimated fixed anchor.

Internal fixed states, missing/reordered years, survey-only fleets, duplicate
parameter-row distributions, nonpositive SDs and unspecified initial rules are
rejected before the active model is cleared. At least two annual states are
required by the high-level fishing interface. A bare unanchored increment penalty
and a stationary AR1 with correlation one are not supported alternatives.

For zero-based times, the proper initial rule gives
`Cov(x_t,x_s)=s0^2+min(t,s)*sigma_f^2`; a fixed anchor sets `s0^2=0`.
Conditional k-year-ahead log-F variance is `k*sigma_f^2`, so uncertainty grows
rather than reverting to a stationary variance. `fishing_mode="mean"` uses zero
innovations and holds the last conditioned log-F level; it is not arithmetic
mean F, since `E(exp(x))` includes a variance correction. A whole-path mean
simulation starts at the declared initial normal mean or fixed anchor.

The native evaluator applies complete fleet log-F paths **before** population
`Prepare()`, mortality, catches and survival. Recruitment means then respond to
the resulting spawning stock during the annual loop. A single bounded evaluation
returns the population, observations and selected-fleet F states; it restores
both recruitment and all changed fleet vectors afterward without resizing
registered storage. Malformed batch paths are validated before any fleet changes.

Simulation draws recruitment paths first, then the selected fleet paths, then
observations. Observation-noise toggles preserve all process paths under the same
seed. `fishing_mode` can differ from recruitment's `process_mode`; `"fixed"`
retains supplied F scenarios. Fixed direct log-R states remain at their supplied
values when F changes, and their realized residuals are recomputed against the
new biological means. Fixed log deviations instead retain their residual values.
Whole-series simulations include initial-year F;
explicit continuations preserve preceding years, and a fixed anchor is always
retained. The return records realized log-F/F, historical/anchored states, process
specifications and whether initial F uncertainty was drawn. Parameter uncertainty
and initial abundance uncertainty remain excluded.

The new density uses the existing Gaussian component, parameter registration,
TMB differentiation and report infrastructure. Process SDs are reported under the
owning fleet. The existing parameter JSON formatter rounds numeric parameter
values; mathematical likelihood/gradient checks use unrounded native values.
Random walks on mortality/selectivity and additional covariance structures remain separate future increments.

### Random-walk validation recorded

The final focused suite passed 39 tests and 302 assertions with no failures,
errors or skips. This includes 109 random-walk assertions plus the existing
Gaussian-process, trajectory-simulation and distribution checks. A broader run
also exercised initialization and estimate reporting; it passed with the two
existing CRAN-gated reporting skips. A clean isolated package build succeeded.

Validation includes a dense random-walk covariance oracle, fixed-anchor increment
likelihood, growing variance and conditional continuation, innovation-SD recovery
over repeated paths, AD gradient/curvature checks, and latent F recovery in a
small assessment with known biology and precise catch data. Tests also cover
state restoration, two independent fleet walks, native rejection of an estimated
fixed anchor, fleet-owned SD reporting, and fixed direct log-R under stochastic F.
A standalone C++17 smoke executable passed; the registered GoogleTest suite was
not run. Full assessment coverage and broad runtime scaling remain unverified.

## Decision proposed for review

Add a reusable, normalized Gaussian process density that attaches to existing
parameter vectors. Begin with IID and stationary AR(1) recruitment; keep the
current registration and TMB integration. Make process membership and initial
conditions explicit before adding random walks, multivariate covariance or latent
abundance. Preserve current behavior when the new process is not requested.

This addresses a verified gap while avoiding a new state-transition engine in the
first increment. An AR(1) density alone will not make FIMS equivalent to WHAM or
SAM. Their latent survival/F models require further changes; compare the
[WHAM NAA options](https://timjmiller.github.io/wham/reference/set_NAA.html) and
[SAM configuration manual](https://fishfollower.r-universe.dev/stockassessment/doc/manual.html).

## Current integration points

| Responsibility | Existing component | Proposed relationship |
|---|---|---|
| Parameter ownership | Biological/fleet vectors; Rcpp variable objects | Keep ownership there; process binds to stable vector IDs and explicit indices. |
| Optimization role | `RegisterParameter` / `RegisterRandomEffect` (E05) | Register each estimable scalar once, independently of density membership. |
| Linking | `Information::variable_map` / `SetupRandomEffects` (E06) | Validate IDs and sizes; resolve target and mean before evaluation. |
| Log density | `DensityComponentBase::evaluate()` (E33) | Add process implementations through this extension point. Return log density; `Model::Evaluate` negates it. |
| Biological means | `CalculateRecruitment`, `LogR`, `LogDevs` (E02–E04) | Use declared mean/residual semantics; do not infer semantics from vector names. |
| Objective order | Dynamics, then prior/process/data densities (E32) | Preserve estimation order; introduce a separate dependency-aware simulation schedule. |
| Integration/uncertainty | `MakeADFun(random="re")`, `sdreport`, ADREPORT (E24–E26) | Retain TMB and propagate new hyperparameters/state outputs. |
| R setup | Recruitment helper and hard-coded Gaussian choice (E08–E09) | Add an explicit opt-in process specification without silently reinterpreting old configurations. |

The string `input_type="random_effects"` selects a density pathway. It does not
by itself establish that every linked scalar was registered for integration.
Existing fixed recruitment elements can still contribute to the density (E31).
That behavior must remain visible and must not be accidentally changed by a new
mask or helper.

## Mathematical contract

### IID and stationary AR(1)

Let `x_t` be the target on its declared working scale, `m_t` its declared mean,
and `u_t=x_t-m_t`. IID contributions are

\[
\ell_{IID}=\sum_t\log\mathcal N(u_t;0,\sigma_t^2).
\]

The first AR(1) implementation should use a shared **innovation** standard
deviation `sigma_e`:

\[
 u_1\sim\mathcal N\left(0,\frac{\sigma_e^2}{1-\rho^2}\right),\qquad
 u_t\mid u_{t-1}\sim\mathcal N(\rho u_{t-1},\sigma_e^2).
\]

Thus `sigma_u=sigma_e/sqrt(1-rho^2)` is the marginal SD. Store/estimate
`log_sigma_e` and an unconstrained correlation parameter mapped by `tanh`.
Never silently substitute TMB's unit-marginal AR1 scale for this innovation scale.
[TMB's AR1 documentation](https://kaskr.github.io/adcomp/classdensity_1_1AR1__t.html)
describes a stationary, unit-variance construction; an adapter needs the explicit
scale conversion. Direct conditional evaluation is a useful transparent first
implementation and independent validation oracle.

Include normalizing constants and the initial-state density. At `rho=0`, the
model must reduce to IID with the same scale and objective, including constants.
A one-element series is valid and uses only the initial density. Reject empty
active series unless explicitly represented as an absent process.

For annual recruitment, distinguish:

1. Deviations: `x=log_devs`, `m=0`, with biological recruitment formed downstream.
2. Direct states: `x=log_r`, `m=log_expected_recruitment`, so the AR(1) is on
   stock-recruit residuals, not automatically on raw log recruitment.
3. A random walk or AR(1) around a constant recruitment level is a different
   scientific model and must be configured separately.

Beverton–Holt is nonlinear and its log mean depends on past spawning biomass.
Neither representation guarantees a sparse full assessment Hessian. The existing
research vignette (E30) motivates measurement, not a universal performance claim.

### Random walks and irregular intervals

RW1 is `x_t | x_(t-1) ~ Normal(x_(t-1),sigma_e^2)` with an explicit initial-level
choice: fixed/conditioned initial level or a proper initial density. A bare
increment penalty is rank deficient as a prior over all levels. Adding a sum-to-zero
constraint changes the model and parameter dimension; it is not a numerical fix
to apply silently. `rho=1` is not a stationary AR(1) configuration.

For an integer gap d under a stationary annual AR(1):

\[
 u_{t+d}\mid u_t\sim\mathcal N\left(\rho^d u_t,
 \sigma_e^2\sum_{j=0}^{d-1}\rho^{2j}\right).
\]

A future irregular-interval implementation should use the finite sum or a stable
near-boundary calculation. A missing observation normally leaves its latent year
in the process. An intentionally absent process year may require a gap transition
or a segment restart; deleting it and treating the neighbors as consecutive is
incorrect. Reject unsupported irregular schedules in the first increment.

### Multivariate and separable extensions

For annual vector states, `x_t-x_(t-1) ~ MVN(0,Sigma)` supports correlated F
increments. Specify positive SDs and a positive-definite correlation factor,
preferably through a Cholesky parameterization. Independently bounding pairwise
correlations does not ensure a valid multivariate correlation matrix.

For an `A x T` age-year residual matrix U, one possible convention is

\[
\operatorname{vec}(U)\sim\mathcal N\left(0,
 \sigma_u^2(R_{year}\otimes R_{age})\right),
\]

with age varying fastest within year. Both correlation matrices have unit
diagonal, so `sigma_u` is a marginal SD. An alternative double-innovation scale
would satisfy
` sigma_e = sigma_u*sqrt((1-rho_age^2)*(1-rho_year^2)) `.
Choose and record one convention; do not expose ambiguous `sigma` fields.
Separate recruitment from older-age survival variance/correlation when justified.
Do not infer state order from variable names: FIMS has several flattened arrays
and loops; mapping must be verified using nonconstant values on unequal axes.

Separable structure is a practical extension, not a justification for an arbitrary
dense `AT x AT` covariance. A nonseparable model should be admitted only with a
scientific need, identifiable parameterization and scaling evidence.

### Log-scale means and constraints

`log R ~ Normal(log f(SSB),v)` makes `f(SSB)` the conditional median unless a
mean correction is applied. For a desired arithmetic mean, the log location
needs the appropriate `-v/2`; for AR(1) the conditional variance differs from the
stationary marginal variance. Define whether biological parameters describe
conditional means, marginal means or medians before adding corrections.

Estimate positive rates on log scales and bounded ogives on appropriate working
scales. A likelihood written for a different random variable may require a
Jacobian; do not double-count one merely because a biological output is
exponentiated. The inference variable and density measure must be documented.

`RecruitmentBase` includes a centering routine whose invocation is commented out
(E36). Do not rely on automatic sum-to-zero behavior. If constrained processes
are later supported, use a valid reduced-dimensional parameterization or an
explicitly conditioned distribution with the correct normalization.

## Process specification and lifecycle

Proposed internal metadata, translated from an additive R configuration:

| Field | Required meaning |
|---|---|
| Target binding | Stable vector ID, process working scale, scalar index map and owning module ID. |
| Shape and labels | Ordered years/ages/groups; axis order, dimensions and segment identifiers. |
| Mean binding | Zero, explicit scalar/vector, or a resolved derived-vector ID. Check broadcast rules. |
| Process family | IID, stationary AR1, and later RW1/MVN/separable. |
| Hyperparameters | Transformed SD/correlation IDs; fixed or estimated, with sharing maps. Start with fixed-effect covariance hyperparameters. |
| Membership | Which states participate in the stochastic model; independent of estimation type. |
| Conditioning | Which participating states are known/fixed and how initial states are treated. |
| Prediction | Future continuation, fixed scenario values, or segment restart, explicitly selected. |
| Reporting | Process log likelihood, residuals/innovations, natural-scale covariance parameters, dimensions. |

First-stage implementation should support one contiguous series with no internal
fixed states under AR(1), or fail clearly. Mixed-state conditional distributions,
gaps and segment restarts need dedicated semantics/tests before activation.
This restriction applies only to the new opt-in process and does not remove the
legacy IID behavior.

Initialization must validate target and mean IDs, shapes, index ranges, finite
initial values, positive scales, parameter sharing and distribution ownership.
No scalar should be registered twice because two process views reference it.
Reject unintended overlapping densities; allow an explicit observation likelihood
and process prior on the same state. Fixed hyperparameters must remain fixed.
Allocate/resize owning vectors before storing scalar addresses in registration;
freeze their dimensions for the lifetime of the TMB tape.

Evaluation must be repeatable without changing parameter values or resizing
registered vectors. Shape/index branching uses fixed metadata. Operations on
estimable values use `Type` and AD-compatible math; no conversion of a parameter
to `double` and no numerical branching that freezes a parameter-dependent path.
Keep log-density outputs consistent with the existing sign convention.

## Architectural alternatives

| Option | Advantages | Limitations | Recommendation |
|---|---|---|---|
| Add named density subclasses behind `DensityComponentBase` | Small additive change; reuses objective, links and registration | Needs shared metadata/validation to avoid duplication | First increment; factor shared validation rather than refactor all distributions. |
| Introduce a separate process subsystem with a density adapter | Clear state, mean, covariance and simulation responsibilities | More lifecycle and interface changes before first science result | Evolve toward this if multiple process families justify it. |
| Embed stochastic logic independently in each biological module | Direct access to local state | Duplicated covariance/initialization; inconsistent configuration | Avoid as the common framework. |
| Full event/state-transition engine immediately | Supports partitions and seasonal latent states coherently | Large scientific and architectural migration | Separate later design, with compatibility tests. |

A small reusable process view can supply metadata and conditional calculations to
density subclasses. This does not require making every biological module inherit
from a new process base or changing current public interfaces.

## Latent abundance and plus groups

FIMS currently computes older-age N deterministically (E01). A full state-space
extension must make log N at selected ages/years an owned parameter vector, then
compute transition means from previous states. It must not overwrite latent N
with deterministic predictions before evaluating the data likelihood.

For ordinary ages, transition log mean is `logN[a-1,t-1]-Z[a-1,t-1]`. For the plus
group use a stable log-sum-exp of the two surviving cohorts. The single resulting
plus-group innovation is applied **after** summing survivors. Putting independent
errors on both incoming cohorts is a different model.

Decide initial abundance priors, recruitment overlap and terminal extra-year
reporting (E02) explicitly. Match the current deterministic model when process
noise is disabled through a structural mode, not by evaluating a singular normal
at `sigma=0`. State arrays require approximately `A*T` latent scalars instead of
approximately T recruitment scalars; additional sex/area/season axes multiply
that dimension. Keep observed states, predicted transition means and report-only
terminal states distinct.

Local state transitions connect adjacent years, and the plus group links two
parents. Age correlation widens those neighborhoods. Dense covariance, shared
latent means, biological feedback and observation aggregation can add coupling;
inspect the actual full objective's sparsity rather than the process prior alone.

## Simulation, forecasting and uncertainty

The existing `Model::Evaluate()` computes dynamics before density simulations
(E32/E07). That order needs explicit handling for unconditional process
simulation. Propose a simulation driver with dependency stages:

1. Select parameter/initial-state draw and scenario rules.
2. Generate process states/innovations in dependency order; latent covariates
   precede biological responses, and annual abundance precedes later survival.
3. Recompute growth, mortality, abundance and derived observation expectations
   from the generated process path.
4. Simulate observations conditional on those expectations.
5. Return states, observations, seeds, configuration and process diagnostics.

For state-feedback processes these stages interleave by time step; drawing a
whole process after a completed deterministic run is insufficient. Conditional
simulation at fitted latent states should remain available as a distinct mode.

Forecasts must distinguish fixed-parameter/process-only draws, latent-state
uncertainty, parameter uncertainty and observation predictive uncertainty. Joint
parameter/state draws must respect dependence; independently drawing marginal
SEs loses covariance. Future innovations should be independent across replicates
and correlated across time according to the process. Excluding future fixed
deviations from a fitted density is not the same as conditioning a latent future
state on an observed value of zero.

Existing `sdreport`/ADREPORT support is useful but is an approximation around a
converged fit. Preserve gradient/Hessian checks and validate coverage through
simulation. `getReportCovariance=FALSE` reduces derived-output covariance memory;
it does not remove the latent-state factorization cost.

## Scalability and backend boundary

For an isolated scalar AR(1) chain, conditional terms give tridiagonal precision
and linear-time banded factorization. That bound does not automatically hold for
an integrated assessment. Measure tape size, latent dimension, Hessian nonzeros,
Cholesky fill, peak memory, objective/gradient time and complete fit time.
Use representative year/age grids and distinguish process-only from full-model
costs. Do not benchmark only repeated evaluation at the optimum.

Centered state and noncentered innovation parameterizations can have different
sparsity and conditioning. Retain the existing `log_r`/`log_devs` alternatives for
comparison. Weakly informed states can favor a noncentered representation; choose
using both convergence and total cost rather than claiming one is universally
superior.

TMB remains the implementation backend. The semantic boundary should describe
process shape, conditional densities and simulation without exposing TMB types in
R configuration. A narrow backend adapter can later implement supported density
operations. **Quadra is only a future candidate**: its AD compatibility, sparse
factorization, Laplace support and licensing/API fit have not been evaluated here.
No Quadra dependency, integration promise or implementation work is proposed.

## Validation and acceptance for the first extension

| Check | Acceptance evidence |
|---|---|
| Mathematical likelihood | Independent dense multivariate-normal oracle for short AR1 series, including constants and stationary first term. |
| AD behavior | Finite-difference gradient/Hessian comparisons away from boundaries; no parameter-to-double conversions. |
| IID compatibility | rho=0 agrees with existing IID objective and fitted results within stated numerical tolerance. |
| Initial/membership semantics | Length one, invalid dimensions, duplicate registration, unsupported gaps/internal fixed states and legacy fixed-year warnings tested explicitly. |
| Parameterization/scales | Simulated innovation and marginal variances match the declared convention; constant versus stock-dependent mean kept distinct. |
| Simulation | Reproducible seeded paths; observations generated from the same sampled path; empirical autocorrelation and variance. |
| Scientific recovery | Repeated simulated assessments over positive/negative/zero rho, weak/strong observation information and near-boundary variance. Report bias, coverage, convergence and failures. |
| Performance | Compare process-only and full-model sparsity/cost on increasing year counts; report unfavorable results too. |
| Compatibility | Current IID recruitment and projection tests pass without opting into a new process. No new dependency or changed default. |

Numerical tolerances and simulation replicate counts should be selected before
implementation based on platform precision and Monte Carlo uncertainty. Treat
failure near variance/correlation boundaries as a scientific diagnostic, not an
occasion to silently clamp estimates.

### Shared innovation SD group validation

The isolated package installation passed. The final fleet-process and TMB-map
regression run passed 33 tests / 291 assertions with no failures, errors or
skips. Checks cover reduced parameter dimension, automatic-differentiation
agreement, expansion of a changed shared SD into both registered parameters,
per-fleet fitted reporting, separate simulated paths, constant groups, and
pre-initialization rejection of inconsistent groups. An earlier combined
fleet/recruitment/simulation run passed 32 tests / 272 assertions. These checks
do not establish assessment-scale coverage or scalability. Existing diagnostic
fit warnings in map regression fixtures are expected and do not establish
convergence of those deliberately constrained test fits.

## Age-specific natural mortality random walks: inference increment

`initialize_fims(..., mortality_process = ...)` can now attach independent
annual random walks to the existing Population `log_M` age-by-year grid. This
uses the existing vector registration, transformations and survival equations;
it does not introduce latent abundance states or an alternative mortality
component. It leaves defaults unchanged.

```r
m_rows <- which(parameters$module_name == "Population" &
                parameters$label == "log_M")
parameters$estimation_type[m_rows] <- "random_effects"
first_year <- parameters$timing[m_rows] == get_start_year(data)
parameters$estimation_type[m_rows[first_year]] <- "constant"
mortality <- list(type = "rw1", sd = 0.1, estimation_type = "fixed_effects",
                  initial = list(type = "fixed"))
initialized <- initialize_fims(parameters, data, mortality_process = mortality)
```

For calendar age `a` and year `t`, `x[a,t] = log M[a,t]` and

\[
x_{a,t} - x_{a,t-1} \sim N(0,\sigma_M^2),\qquad t>1.
\]

Innovations are independent across ages and years, with a common supplied or
estimated innovation SD. The fixed initial rule conditions on every age's
first-year log-M. Alternatively, `initial = list(type = "normal", mean = ...,
sd = ...)` places independent proper normal distributions on all first-year
log-M values with common fixed mean and SD; every target must then be estimated.
Internal constant states, incomplete or reordered grids, and duplicate density
declarations are rejected before clearing the active model. State estimation
flags remain explicit: random effects use TMB Laplace integration, fixed effects
use a penalized joint fit. The density does not register states a second time.

The reusable Gaussian kernel now supports `n_series` interleaved independent
series. With age-fast, year-major storage the predecessor index is `i - A`,
where `A` is the number of ages. This avoids imposing spurious transitions
between adjacent ages. It applies the initial rule to the first `A` entries,
and native registration checks that every fixed initial anchor is constant.
The default `n_series = 1` preserves existing recruitment and fishing behavior.
The Rcpp `set_series_count()` method configures this dimension; multi-series
random-walk JSON includes `n_series` metadata. The model continues to exponentiate
all supplied log-M values before mortality and survival calculations.

This formulation is a calendar-age process, not a cohort process or a common
annual mortality shock. Those models need different indexing/covariance choices.
A common innovation SD does not imply common innovations. The conditional
zero-innovation path is a log-scale mean and a natural-scale median. Prior
precision is block-tridiagonal after ordering by age; biological likelihoods,
recruitment feedback and plus-group parents add posterior dependencies.
With `A` ages and `T` years, fixed initialization adds `A(T-1)` estimated states;
proper initialization adds `AT`. The one common covariance parameter couples
those chains in the Hessian. Mortality/F/recruitment confounding means that
assessment-scale recovery and sensitivity to the initial rule remain necessary
before operational use.

The initial inference increment has now been extended to mortality-aware
simulation and forecasting, described below. No Quadra dependency or other
dependency has been introduced.

Validation for the mortality inference increment: an isolated clean package
installation passed. The final combined mortality/fishing/recruitment/simulation
suite passed 38 tests / 310 assertions with no failures, errors or skips. Tests
cover independently computed age-specific increment and initial densities,
analytic innovation-SD derivatives and finite differences, interleaved simulation
covariance, conditional zero-innovation continuation, pre-clear validation,
native protection of all initial anchors, reporting, age-year mortality values,
cohort survival, both plus-group contributions, and finite evaluation of a small
Laplace-integrated assessment. A standalone C++ interleaved-process smoke test
passed. A corresponding GTest case was added, but the GTest suite was not run
because the existing local GoogleTest dependency cache is unavailable. These
checks do not establish operational assessment recovery, coverage or scaling.


## Mortality-aware simulation and conditional forecasting

`simulate_fims()` and `forecast_fims()` accept `mortality_process` using the
same specification as inference, and `mortality_mode` (`simulate`, `fixed`, or
`mean`, defaulting to `process_mode`). The returned `natural_mortality` table
records age, year, log-M, M and conditioning flags for every modeled cell.
Configuration records the specification, mode, first simulated mortality year
and whether proper initial mortality uncertainty was drawn.

```r
projection <- forecast_fims(
  parameters, data, recruitment_process = list(type = "iid"),
  from_year = get_start_year(data) + 2L, nsim = 20, seed = 42,
  process_mode = "fixed", observation_error = FALSE,
  mortality_process = mortality, mortality_mode = "simulate"
)
projection$replicates[[1]]$natural_mortality
```

 Initial abundance
remains fixed; supplied hyperparameters are used, without posterior parameter
uncertainty or automatic extraction of fitted covariance values.

All recruitment, fishing and mortality paths are drawn before observation noise.
Toggling observation error therefore preserves trajectories with a common seed.
An explicit seed restores the caller's RNG state. Without a mortality process,
existing simulation behavior is retained. Additional processes change the RNG
stream for subsequent observation draws, as expected.

A continuation retains complete historical year blocks at every age and starts
at the requested modeled `from_year`; the initial model year is valid when a
mortality process is configured. Zero innovations continue each age's last
historical log-M independently. A whole-series fixed initialization retains the
first-year supplied values; a proper-normal initialization draws each age's
first state independently from its specified initial distribution. `mean` uses
that initial log-scale mean, and `fixed` keeps all supplied states. These log-scale
means correspond to M medians, not arithmetic means of M or biomass.

The new native `evaluate_all_process_paths()` validates the entire batch before
changing any targets. It writes M and F before preparation, exponentiation,
mortality, recruitment means, catches and survival are recomputed. Full owned
vectors are restored without resizing on success or an exception, followed by a
baseline evaluation. Older native path-evaluation methods delegate with an empty
mortality path, preserving their call signatures. Fixed direct log-R remains
fixed even when sampled mortality changes the biological recruitment mean;
reported residuals are recomputed against the new mean. Recruitment deviations
retain their usual interpretation under changing biological means.

The configured horizon, biological inputs and fleet scenarios must already be
provided. Mortality is calendar-age specific with independent innovations and a
common innovation SD; this does not add age correlation, common annual shocks,
management feedback, or posterior state/parameter sampling.

Mortality simulation validation: clean native package installation and R/Rd
checks passed. The final combined process suite passed 44 tests / 366 assertions
with no failures, errors or skips. Added tests cover seeded reproducibility,
observation-noise isolation, distinct replicate mortality paths, fixed and proper
initial rules, per-age conditional log-scale continuation, retained historical
observations, supplied fixed mortality paths, rate transformation, cohort survival
and both plus-group parents, native restoration, rejected malformed/overflowing
paths with no partial F update, joint M/F simulation with fixed direct log-R,
and RNG/parameter preservation after rejected simulation. No operational
assessment recovery, interval coverage or large-model scaling claim is made.

## Recovery and identifiability screening

The standalone benchmark in `tests/integration/process-recovery/` evaluates
recruitment, F and M alone against oracle trajectories, then all three jointly,
with known/estimated SDs and known/proper first-year states. It records native
full-precision state modes and Laplace/Gaussian marginal uncertainty, log-SD
intervals, optimizer/inner gradients, Hessian status, variance boundaries,
posterior state correlations, failures, fit dimensions and runtime. State metrics
exclude constant anchors, and coverage Monte Carlo SEs use replicate fractions
rather than pretending age-year cells are independent. Tests check metric
denominators, failure/boundary accounting, initialization and actual estimation.

The retained five-replicate, six-year pilot has 70 fits and 12 lower-bound SD
estimates. Joint known-anchor M coverage averaged 0.937 with known SDs versus
0.613 with estimated SDs, including boundary fits. Proper-initial fits have a
different scored state set and more initial uncertainty. These are descriptive
pilot outcomes, not coverage validation or proof of structural confounding.
The next investigation should use longer series, multiple starts and SD profiles
before adding common mortality shocks or correlated age innovations. No production
interfaces, implementations or dependencies changed for this benchmark upgrade.

### Longer-series, multiple-start and profile investigation

The `sensitivity.R` benchmark extension keeps the first pilot's source and
checksum unchanged. It fits joint recruitment/F/M models from common SD starts
0.025, 0.1 and 0.35 at 6 and 18 years, with both initial-information settings.
For the first replicated dataset per horizon it computes a mortality-SD profile:

\[
\ell_p(\sigma_M)=\min_{\log\sigma_R,\log\sigma_F}
\left[-\log\int p(y\mid u)\,p(u\mid\sigma_R,\sigma_F,\sigma_M)\,du\right],
\]

using TMB's Laplace approximation over all latent process states. Mapping the
mortality covariance parameter fixes only that SD; nuisance covariance parameters
and all states are reoptimized at every grid point and start. An objective-equivalence
test verifies that mapped/free objectives agree at matching covariance values.
Relative NLL references the best valid free or grid solution for the same data
and initial-information setting. No asymptotic confidence cutoff is imposed.

All attempts remain visible. Candidate selection requires code-zero optimization,
small latent-state gradients and small box-projected covariance gradients, while
retaining raw gradients and variance-boundary flags. This permits legitimate
boundary optima in the likelihood investigation without calling their intervals
regular. Unlike the first recovery pilot, these fits do not calculate `sdreport`
or interval coverage. Same seed labels at different horizons do not generate
nested trajectory prefixes, and three shared SD starts do not exhaust nuisance
parameter combinations or establish global optimization. Checkpoints preserve
completed datasets; no production interfaces or dependencies change.

The retained sensitivity pilot evaluated 132 starts/profile attempts (129 passed
optimizer checks). Every selected free/profile case had at least two valid
starts; maximum valid objective spread was below 0.00004. Mortality SD still
reached the lower bound in two of three 18-year datasets under each initial
setting. For the first longer dataset, the anchored profile's lower-bound point
was 0.905 NLL units above the optimum and the proper-initial profile's was 0.181.
These broad profiles and start agreement suggest weak variance information in
these datasets, not a demonstrated improvement from additional covariance.
Three replicates and one profiled dataset per horizon do not establish boundary
frequencies or coverage. Details, all attempt outcomes and a visually inspected
profile PNG are retained in `tests/integration/process-recovery/sensitivity-pilot/`.
Validation passed 9 tests / 51 assertions, including the two opt-in integration
checks. Production code and the original pilot source remain unchanged.

### Paired observation-information experiment

The `information.R` benchmark extension varies survey log-SD (0.20 versus 0.05)
and age/length composition sample size (200 versus 2000) in a two-by-two design.
Catch log-SD stays at 0.1. One recruitment/F/M trajectory is generated per
replicate and reused exactly across designs, with known biology and matching
observation likelihoods in every fit. Catch observations are identical. Survey
observations use `y_d = exp(mu + s_d z)` with the same standard-normal `z` for
all designs. Each composition has an independent categorical pool; the first
200 draws form the smaller sample and the first 2000 form the larger sample.
Each prefix is marginally multinomial with the declared size. Pairing is across
hypothetical designs, not additional correlated observations within a fit.

The default experiment has three 18-year datasets, two common covariance starts,
and both first-year information settings. Baseline and combined-improvement
designs receive reoptimized mortality-SD profiles for dataset 1; intermediate
designs receive free fits. Selection reuses the sensitivity benchmark, retaining
all attempts and explicit valid-fit denominators. Every observation dataset has
its own profile likelihood reference; raw objectives across designs are not
compared. RDS outputs preserve truth, all datasets, errors/pools and seeds.
Previous sources and pilots remain unchanged; no production code or dependencies
are added. These optimistic sampling designs do not account for composition
overdispersion, aging error, unknown selectivity or observation costs, and do
not establish interval coverage.

The retained observation-information pilot has 96 attempts (94 passed optimizer
checks) and 48 valid selected free/profile cases. Under proper initialization,
larger compositions reduced mean log-M state RMSE by about 21% across three
paired 18-year datasets; combined survey/composition improvements reduced it by
about 23%, while survey precision alone changed it little. The combined-data
profile was sharper but favored below-truth mortality SD in dataset 1. State
recovery, covariance recovery and interval calibration therefore remain separate
validation targets. All-attempt tables, paired estimates, profile plots and
limitations are in `tests/integration/process-recovery/information-pilot/`.
Validation passed 13 tests / 83 assertions, including all three benchmark
integration checks. Both PNGs were visually inspected. No production code or
previous pilot source was changed.

### Interval calibration across true mortality SDs

The `calibration.R` extension uses the informative combined observation design,
proper-normal initial uncertainty and true M innovation SDs 0.025, 0.05 and 0.10.
Ten datasets per setting are the default, with shared standardized process
innovations across settings. Each dataset compares all covariance SDs fixed at
truth against all three estimated from two starts. The comparison therefore
includes uncertainty/confounding in recruitment and F SDs, not only mortality SD.

A selected covariance point is rebuilt and its marginal objective checked before
`sdreport`. State modes and Laplace/Gaussian marginal SEs give pointwise intervals,
coverage, width and standardized error diagnostics. Coverage is averaged within
replicate and its MCSE uses replicate fractions, respecting dependence among
age-year cells. Diagnostics separate optimizer validity, Hessian status,
any-covariance/M boundaries and a regular reporting stratum. All numerical
intervals remain visible; neither boundary fits nor failures are silently
classified as successful uncertainty estimates. Regular-only results are selected
outcomes and do not establish unconditional calibration. Hyperparameter interval
coverage includes explicit denominators and exact binomial uncertainty bounds on
the coverage proportion, while boundary Wald intervals remain nonregular.

This is a joint-generative latent-state uncertainty screening experiment, not a
fixed-state frequentist guarantee. More replication, misspecification and
unknown observation/biological parameters remain separate validation needs.
Previous sources/pilots and production code are unchanged.

The retained calibration pilot generated 30 datasets and all 60 known/estimated
uncertainty reports succeeded. 58 of 60 covariance starts passed optimizer checks,
and every selected fit was valid. Covariance-point rebuild differences were below
0.0000000021 NLL units. Known-SD M state coverage averaged 93.3–94.5%, versus
92.7–93.7% when SDs were estimated. Paired differences were small relative to their
Monte Carlo uncertainty; ten replicates per setting do not establish nominal
coverage. At true M SD 0.025, five of ten estimates reached the lower bound, while
no boundaries occurred at 0.05 or 0.10. Boundary log-SD intervals had superficially
high apparent coverage and are kept separate from regular outcomes. Tables and
a visually inspected plot are in `tests/integration/process-recovery/calibration-pilot/`.
Validation passed 17 tests / 114 assertions, including four integration checks.
Production code and earlier numerical-source checksums remain unchanged.

### Observation misspecification and conditional-solution sensitivity

The robustness benchmark adds independent Dirichlet–multinomial composition
generation but retains the fitted multinomial n=2000 likelihood. Concentrations
infinity/2000/200 imply variance inflation 1/about 2/about 10.95. Shared latent
truth and random variates control comparisons; the sampler's mean and full
covariance are tested independently. Five paired 18-year datasets gave known-SD
M coverage of 94.8%/89.8%/57.4%. Moderate extra variation also inflated estimated
process variance, despite regular optimizer/Hessian diagnostics.

The strong estimated-SD case had no valid uncertainty reports: four cold rebuilds
failed objective agreement, and one numerical report had invalid optimization.
All selected outcomes reached the covariance bound. A saved-inner-state diagnostic
reproduced one reference objective while a cold state start differed by 0.567 NLL
units, with small inner gradients and agreement of the joint objective at a common
reference point. This is evidence of conditional-solution sensitivity, not proof
of a global optimum or permission to relax validation thresholds. A reusable
uncertainty pathway should preserve modes and expose alternate solutions, while
a robust observation likelihood should be validated on the same stress data.
Details and all failed/numerical outcomes are retained in the robustness pilot.
Validation passed 22 tests / 194 assertions; production interfaces are unchanged.

## Opt-in Dirichlet-multinomial composition inference

`initialize_fims(..., composition_likelihood = ...)` now supports a scalar
concentration for each selected fleet and composition type:

```r
composition <- list(fleet1 = list(
  age_comp = list(type = "dirichlet_multinomial", concentration = 200,
                  estimation_type = "fixed_effects"),
  length_comp = list(type = "dirichlet_multinomial", concentration = 200)
))
input <- initialize_fims(parameters, data, composition_likelihood = composition)
```

The default estimation type is constant. The input uncertainty formulas remain
`dmultinom(prob = ..., size = ...)`: they supply nominal sample sizes for
converting proportions to counts. The explicit initializer option selects the
likelihood. Each selected fleet/type shares its concentration across years;
different fleet/type entries are independent parameters. Users can compose TMB
maps if sharing is scientifically justified. No additional dispersion grouping
interface is provided. Counts supplied in number units retain their row totals.
The default NULL option preserves the existing multinomial path and JSON shape.

For integer counts, `q ~ Dirichlet(kappa*p)` and `x|q ~ Multinomial(n,q)` give

\[
\log L=\log\Gamma(n+1)-\sum_j\log\Gamma(x_j+1)
 +\log\Gamma(\kappa)-\log\Gamma(\kappa+n)
 +\sum_j[\log\Gamma(\kappa p_j+x_j)-\log\Gamma(\kappa p_j)].
\]

`DirichletMultinomialLPMF` implements this normalized density behind the existing
composition interface. An explicit native setter selects the backend before
model creation; switching an already-created multinomial density requires
reinitialization and is rejected otherwise. Concentration is positive via
`exp(log_concentration)` and may be constant or an estimated fixed effect.
It is registered once, supports TMB automatic differentiation, and is reported
under the fleet using its observed-data ID, with `composition_type` metadata.
Derived expected-probability vectors are not mistaken for owning parameters.

A missing cell skips its entire composition row, matching existing conventions.
Structural p=0/x=0 contributes zero without a pseudocount, while positive counts
at p=0 have zero probability. Invalid counts, dimensions and probability sums
are rejected. Pointwise reporting allocates each row's log density equally to
its bins, so summing bins counts that row exactly once. Fractional counts use
the gamma extension as a weighted likelihood; only integer counts define a
normalized sampling PMF. No rounding or implied sample-size estimation is added.

Large concentration tends to the multinomial, but direct gamma differences and
vanishing dispersion information require numerical care at extreme values.
Tests exercise concentrations through 1e7 for small-count limit checks; this is
not an accuracy guarantee at arbitrary counts/concentrations. Practical bounded
optimization and checks for weak concentration information remain necessary.
The present increment supports inference. Production `simulate_fims()` does not
yet accept this option or generate DM observations; the validated benchmark
sampler supplies such data for testing. Assessment-scale dispersion/coverage
recovery is distinct from the standalone concentration-recovery checks.

DM validation: clean package installation passed. The focused production-density
and existing distribution tests passed 13 tests / 72 assertions after the final
lifecycle guard, with no failures/skips. A broader run passed 94 tests / 554
assertions with two existing CRAN-gated reporting skips. Checks include a
beta-binomial oracle, full small-count normalization, multinomial limit,
structural support, dimensions, independent concentration recovery, exact
assessment likelihood differences, AD gradients/Hessian, reporting ownership and
pre-clear validation. A standalone C++ linked-data/missing-row smoke test passed;
a GTest case is registered, but GoogleTest was not run because its local dependency
cache is unavailable. These checks do not establish assessment-scale calibration.


## Saved-mode benchmark uncertainty handoff

`mode-handoff.R` adds a fresh-model report pathway storing only fixed/random
vectors, reference objectives, transient-ID-independent layouts and exact model
inputs. RDS round trips across native model clearing are tested. Reconstruction
rejects changed inputs/layouts or truncated/nonfinite vectors, restores the saved
states, probes the joint objective at a common point and checks marginal objective
agreement before/after `sdreport`. Cold discrepancies remain explicit with no
substitute intervals. State-only reporting skips unused ADREPORT delta-method
calculations; state SEs and fixed covariance agree with the full call in tests.

The retained single-case comparison reproduces the strong multinomial cold/warm
difference (0.567 NLL units). Warm reconstruction is consistent; a matched DM model
with known concentration 200 also agrees from cold/warm starts. M-state RMSE falls
from 1.215 to 0.292 in that one case, but its estimated innovation SD moves from
the upper to lower benchmark bound and local pointwise coverage remains 75.5%.
This is neither global-mode validation nor replicated calibration. Reference-point
agreement does not certify every Hessian perturbation or alternate mode. The
public fit API is unchanged; this benchmark pathway demonstrates mode ownership
and checked handoff without rewriting historical pilot outcomes.

The final focused run passed 16 tests / 85 assertions with no failures/skips;
metadata, source hashes, serialization and local objective differences are
retained in `tests/integration/process-recovery/mode-handoff-pilot/`. The remaining
validation targets are replicated matched-likelihood fits, joint concentration
recovery, alternate latent/covariance starts and derived/forecast interval coverage.
