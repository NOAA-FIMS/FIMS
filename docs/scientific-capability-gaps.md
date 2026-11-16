# FIMS scientific capability gaps

Investigation draft, 2026-11-16. FIMS baseline: `f028bff` on
`investigate/scientific-capability-gaps`. This is a source and documentation audit,
not a numerical certification of the models. No assessment fits or external
framework executables were run. The architecture and roadmap are in
[stochastic-process-architecture.md](stochastic-process-architecture.md) and
[scientific-capability-roadmap.md](scientific-capability-roadmap.md).

Implementation update: IID/AR1 recruitment and a bounded process-aware simulation/
continuation driver, plus initialized fleet log-F random walks, are now present
on this working branch. The matrix below retains the **pre-implementation baseline**
for auditability; see the [implementation section](stochastic-process-architecture.md#first-implementation-increment)
for current scope and validation.

## Interpretation and scope

**Implemented** means an inspected executable pathway exists, without implying
complete scientific validation. **Partial** means a lower-level pathway or some
of the requested scientific behavior exists. **Missing** means the inspected
execution pathway lacks the capability and the relevant source inventory yielded
no alternative implementation. **Unverified** means evidence is insufficient.
A registerable random parameter is not by itself a stochastic model. A smoothing
penalty is not automatically a normalized density integrated by Laplace.

This matrix covers every topic requested in the handoff. It is not a claim to
exhaust every option in six changing frameworks. Reference models are positive
examples only: an omitted framework is **not** a negative finding. Comparisons
are at the cited manual/version, not necessarily at the default configuration.
AMAK is officially named **Assessment Model for Alaska**. Its evidence below is
assessment-specific; it must not be generalized to every AMAK configuration.

FIMS negative findings combine inspection of the population/model loops, R setup,
Rcpp registration, distribution inventory, documentation and associated tests.
Search terms included AR1/ARMA/random walk, movement/migration, season, tagging,
environment/covariate, regime and composition families. Documentation aspirations
and research vignettes were not treated as production features.

## Reference evidence

References were consulted on 2026-11-16. Live documentation can change; pin source
commits and input files before numerical cross-framework comparisons.

| ID | Authoritative reference and scope used |
|---|---|
| W1 | [WHAM set_NAA](https://timjmiller.github.io/wham/reference/set_NAA.html), displayed development version 2.1.0.9012: recruitment-only versus full-state configurations, age/year correlation and variance maps. |
| W2 | [WHAM set_M](https://timjmiller.github.io/wham/reference/set_M.html), displayed version 2.1.0.9011: age/year mortality process options and weight-dependent M. |
| W3 | [WHAM set_ecov](https://timjmiller.github.io/wham/reference/set_ecov.html): covariate observations, latent process, lags, effects on recruitment/M/q/movement. |
| W4 | [WHAM source overview](https://github.com/timjmiller/wham/blob/master/README.md): stochastic selectivity, movement and forecasting scope. |
| W5 | [WHAM changelog](https://timjmiller.github.io/wham/news/index.html): logistic-normal composition correlation. |
| W6 | [Stock and Miller (2021)](https://www.sciencedirect.com/science/article/pii/S0165783621000953): original stochastic transition formulation and simulation comparisons. |
| S1 | [SAM stockassessment manual](https://fishfollower.r-universe.dev/stockassessment/doc/manual.html), version 0.12.0: `sam.fit` configuration (`corFlag`, `obsCorStruct`, `stockRecruitmentModelCode`), `forecast`, `simulate`, `simstudy`, `retro`, and reference-point interfaces. |
| S2 | [NAFO 2018 cod benchmark](https://www.nafo.int/Portals/0/PDFs/sc/2018/scs18-18.pdf), SAM description around printed p. 36: abundance innovations and stochastic F. |
| SS | [Stock Synthesis manual 3.30.26](https://nmfs-ost.github.io/ss3-doc/SS330_User_Manual_release.html), dated 2026-10-29: §§6, 7.3, 7.14–7.20, 8.4–8.5, 14.1. |
| C1 | [Casal2 developer-supported overview](https://casal2.github.io/): generic categories, user-defined annual process schedules, tagging, stochastic projections and simulation. |
| C2 | [Casal2 user manual, NIWA TR139](https://docs.niwa.co.nz/library/public/NIWAtr139.pdf), §7.1: process-associated observations and mortality-block timing. Search-index text was available; direct PDF retrieval failed. Claims from this source have lower retrieval confidence. |
| A1 | [NOAA 2021 Atka mackerel assessment](https://apps-afsc.fisheries.noaa.gov/Plan_Team/2021/BSAIatka.pdf), Appendix 17C: AMAK equations, selectivity regularization, recruitment and fishing effects. |
| A2 | [AMAK official repository](https://github.com/afsc-assessments/AMAK): project identity. Source subdirectories could not be retrieved; source-level generalization remains unverified. |
| J1 | [IMARPE JJM user-guide proposal, SC12-JM08 (2024)](https://www.sprfmo.int/assets/Meetings/02-SC/12th-SC-2024/Jack-Mackerel/SC12-JM08-PER-Joint-Jack-Mackerel-Model-User-Guide.pdf), §§2–3 and control-file description: stock/regime mapping, recruitment and observation equations. This is an explicitly labeled proposal, not a universal specification of every JJM variant. |

## FIMS audit trail

Line numbers below refer to the baseline above. Relative links navigate to files;
`#L` fragments identify source lines on compatible repository viewers.

| ID | Source location | Inspected behavior |
|---|---|---|
| E01 | [inst/include/models/functors/catch_at_age.hpp:313](../inst/include/models/functors/catch_at_age.hpp#L313) | CatchAtAge::CalculateNumbersAA: deterministic cohort survival and plus group. |
| E02 | [inst/include/models/functors/catch_at_age.hpp:682](../inst/include/models/functors/catch_at_age.hpp#L682) | CatchAtAge::CalculateRecruitment: stock-recruit mean and process linkage; terminal extra year uses mean. |
| E03 | [inst/include/population_dynamics/recruitment/functors/log_devs.hpp:21](../inst/include/population_dynamics/recruitment/functors/log_devs.hpp#L21) | LogDevs: mean log recruitment plus deviations. |
| E04 | [inst/include/population_dynamics/recruitment/functors/log_r.hpp:21](../inst/include/population_dynamics/recruitment/functors/log_r.hpp#L21) | LogR: log recruitment supplied directly. |
| E05 | [inst/include/common/information.hpp:284](../inst/include/common/information.hpp#L284) | Information::RegisterRandomEffect: registration of estimable random effects. |
| E06 | [inst/include/common/information.hpp:338](../inst/include/common/information.hpp#L338) | Information::SetupRandomEffects: target and expected-vector links. |
| E07 | [inst/include/distributions/functors/normal_lpdf.hpp:59](../inst/include/distributions/functors/normal_lpdf.hpp#L59) | NormalLPDF::evaluate: independent scalar normal contributions and simulation hooks. |
| E08 | [R/distribution_formulas.R:250](../R/distribution_formulas.R#L250) | R helper restricts target names to log_devs/log_r; Gaussian/lognormal families. |
| E09 | [R/initialize_modules.R:1306](../R/initialize_modules.R#L1306) | initialize_fims recruitment setup explicitly selects Gaussian. |
| E10 | [inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp:485](../inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp#L485) | PopulationInterface registers random log_M; also log_f_multiplier and initial abundance. |
| E11 | [inst/include/interface/rcpp/rcpp_objects/rcpp_fleet.hpp:588](../inst/include/interface/rcpp/rcpp_objects/rcpp_fleet.hpp#L588) | FleetInterface registers annual log_Fmort; log_q also registerable. |
| E12 | [inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp:804](../inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp#L804) | Age-specific selectivity random-effect registration; logistic forms also register parameters. |
| E13 | [inst/include/interface/rcpp/rcpp_objects/rcpp_growth.hpp:1040](../inst/include/interface/rcpp/rcpp_objects/rcpp_growth.hpp#L1040) | VonBertalanffySchnuteGrowthInterface parameter-vector registration. |
| E14 | [inst/include/interface/rcpp/rcpp_objects/rcpp_maturity.hpp:246](../inst/include/interface/rcpp/rcpp_objects/rcpp_maturity.hpp#L246) | LogisticMaturityInterface random-effect registration. |
| E15 | [inst/include/models/functors/catch_at_age.hpp:202](../inst/include/models/functors/catch_at_age.hpp#L202) | CatchAtAge::Prepare transforms log_M, fleet F, q and F multipliers. |
| E16 | [inst/include/models/functors/catch_at_age.hpp:450](../inst/include/models/functors/catch_at_age.hpp#L450) | CatchAtAge::CalculateMortality: sum fleet F times selectivity and population multiplier. |
| E17 | [inst/include/population_dynamics/population/subpopulation.hpp:18](../inst/include/population_dynamics/population/subpopulation.hpp#L18) | Subpopulation is an empty placeholder. |
| E18 | [inst/include/population_dynamics/population/population.hpp:27](../inst/include/population_dynamics/population/population.hpp#L27) | Population: year/age/fleet dimensions and female proportion; no sex/area/season state axes. |
| E19 | [inst/include/population_dynamics/growth/growth_model_adapter.hpp:332](../inst/include/population_dynamics/growth/growth_model_adapter.hpp#L332) | VonBertalanffySchnuteGrowthModelAdapter rejects multiple sexes. |
| E20 | [inst/include/models/functors/catch_at_age.hpp:1349](../inst/include/models/functors/catch_at_age.hpp#L1349) | CatchAtAge::evaluate_length_comp: fixed or growth-derived age-to-length predictions. |
| E21 | [inst/include/models/functors/catch_at_age.hpp:1446](../inst/include/models/functors/catch_at_age.hpp#L1446) | CatchAtAge::evaluate_ageing_error: age-reading-error transformation. |
| E22 | [inst/include/models/functors/catch_at_age.hpp:909](../inst/include/models/functors/catch_at_age.hpp#L909) | CatchAtAge::CalculateIndexNumbersAA: q times selectivity times N, no fractional-year survival. |
| E23 | [inst/include/distributions/distributions.hpp:15](../inst/include/distributions/distributions.hpp#L15) | Distribution inventory: Gaussian, lognormal and multinomial functors. |
| E24 | [R/fimsfit.R:774](../R/fimsfit.R#L774) | fit_fims: TMB objective with random="re". |
| E25 | [R/fimsfit.R:868](../R/fimsfit.R#L868) | fit_fims: sdreport and configurable report covariance. |
| E26 | [inst/include/models/functors/catch_at_age.hpp:1904](../inst/include/models/functors/catch_at_age.hpp#L1904) | CatchAtAge::Report: AD-derived biomass, abundance, mortality and other quantities. |
| E27 | [vignettes/fims-demo-projections.Rmd:22](../vignettes/fims-demo-projections.Rmd#L22) | Documented extended-year projections and target-prior limitations. |
| E28 | [R/fims_retro.R:105](../R/fims_retro.R#L105) | Retrospective peeling interface. |
| E29 | [R/mohns_rho.R:70](../R/mohns_rho.R#L70) | Mohn rho diagnostic. |
| E30 | [vignettes/RD-re-sparsity.Rmd:15](../vignettes/RD-re-sparsity.Rmd#L15) | RTMB proxy study of recruitment parameterization and Hessian sparsity; not production AR1 implementation. |
| E31 | [R/initialize_modules.R:1275](../R/initialize_modules.R#L1275) | Fixed recruitment elements remain in full-vector process density; warning documents inferential effects. |
| E32 | [inst/include/common/model.hpp:67](../inst/include/common/model.hpp#L67) | Model::Evaluate: dynamics precede prior, process and observation densities. |
| E33 | [inst/include/distributions/functors/density_components_base.hpp:32](../inst/include/distributions/functors/density_components_base.hpp#L32) | DensityComponentBase: target/mean links, virtual log-density and simulation flag. |
| E34 | [inst/include/population_dynamics/recruitment/functors/sr_beverton_holt.hpp:49](../inst/include/population_dynamics/recruitment/functors/sr_beverton_holt.hpp#L49) | SRBevertonHolt::evaluate_mean uses first steepness and R0 entries. |
| E35 | [R/initialize_modules.R:1347](../R/initialize_modules.R#L1347) | High-level initializer builds a single population and catch-at-age model. |
| E36 | [inst/include/population_dynamics/recruitment/functors/recruitment_base.hpp:63](../inst/include/population_dynamics/recruitment/functors/recruitment_base.hpp#L63) | RecruitmentBase::Prepare does not call commented-out centering routine. |
| E37 | [inst/include/population_dynamics/growth/growth_model_adapter.hpp:282](../inst/include/population_dynamics/growth/growth_model_adapter.hpp#L282) | Growth parameter correlations for delta-method size variability, distinct from temporal process covariance. |

## Scientific formulations

These equations define the capabilities being compared and proposed validation
targets. They do not imply identical parameterizations across reference models.
Here `t` is year, `a` age, `f` fleet, `k` category and `N` abundance. Gaussian
standard deviations and covariance scales must be distinguished.

- **F1 — IID:** residuals `u_i = x_i - m_i` independently follow
  `Normal(0, sigma_i^2)`. Scalar sigma means a shared scale, not correlation.
- **F2 — AR(1):** `u_t = rho*u_(t-1) + e_t`,
  `e_t ~ Normal(0, sigma_e^2)`, `abs(rho)<1`; stationary initial variance is
  `sigma_e^2/(1-rho^2)`.
- **F3 — Random walk / ARMA:** RW1 has `x_t=x_(t-1)+e_t`; ARMA adds finite
  lagged states and innovations. RW1 requires an explicit initial-level rule.
- **F4 — Multivariate/separable:** `e_t ~ MVN(0,Sigma)`;
  a separable age-year residual has covariance
  `sigma_u^2 * R_year(rho_y) ⊗ R_age(rho_a)` under declared vectorization.
  Nonseparable covariance cannot generally be written as that product.
- **F5 — Survival states:** ordinary ages satisfy
  `log(N[a,t])=log(N[a-1,t-1])-Z[a-1,t-1]+e[a,t]`.
  For plus group A the conditional mean on log scale is
  `log(N[A-1,t-1]*exp(-Z[A-1,t-1])+N[A,t-1]*exp(-Z[A,t-1]))`.
  FIMS implements these transitions with zero innovations after recruitment.
- **F6 — Mortality:** `Z[a,t]=M[a,t]+sum_f F[f,t]*s[f,a,t]*g[t]`;
  stochastic log M or log F needs a density in addition to this transformation.
- **F7 — Biology:** logistic ogives, weight-length relationships and growth
  determine selectivity, maturity and size products. Multiple growth patterns
  require a mixture or partition, not only uncertainty around one curve.
- **F8 — Partitions:** movement uses `N[k',t+1]=sum_k P[k,k',t]*S[k,t]*N[k,t]`
  with row-stochastic P; season duration scales rates by `Delta_t`.
  Movement and mortality order must be declared.
- **F9 — Covariates/regimes:** `link(theta_t)=alpha+beta*E[t-lag]+u_t`;
  uncertain covariates add `E_obs[t] ~ Normal(E[t],s_obs[t]^2)` and a density
  for E. A fixed regime schedule replaces alpha by `alpha[regime(t)]`.
- **F10 — Compositions:** counts may follow `Multinomial(n,p)` or
  `DirichletMultinomial(n,alpha*p)`; logistic-normal models use a nonredundant
  log-ratio vector and covariance. Overdispersion is not synonymous with
  outlier robustness; zero handling and contamination assumptions need choices.
- **F11 — Observation operators:** ageing error maps true ages using an
  error matrix; survey expectation includes `q*s*N*exp(-tau*Z)` when observed
  at fractional year tau. Tag recovery additionally tracks releases, survival,
  reporting and tag loss.
- **F12 — Prediction/inference:** distinguish conditional prediction at fitted
  parameters, parameter uncertainty, latent-state uncertainty, future process
  innovations and future observation error. Retrospective rho is a statistic
  of terminal estimates across peels, not a process model.
- **F13 — Density dependence:** a stock-recruit mean can be Beverton–Holt;
  other biological feedback requires a specified population-derived predictor,
  e.g. `link(theta_t)=alpha+beta*log(B_t)`.

## Gap matrix

Evidence IDs resolve to the inspected locations above. All rows provide the nine
requested fields. Priority is a proposed development priority, not a statement
that a complex model will necessarily improve advice. Implemented rows generally
need maintenance/validation rather than new implementation.

| Capability | Reference models | FIMS status | Evidence | Scientific formulation | Architectural impact | Dependencies | Priority | Validation |
|---|---|---|---|---|---|---|---|---|
| IID recruitment process | WHAM W1; SAM S1 | Implemented | E02–E08 | F1; log deviations or direct log R | Preserve density and parameter registration; clarify scales | None | High | Analytical Gaussian likelihood; equivalence of two recruitment representations |
| Temporal AR(1) recruitment | WHAM W1 | Missing | E07–E09, E30 | F2 on declared recruitment residual | New density plus Rcpp/R process configuration | Initial distribution; scale convention | High | rho=0 equivalence; simulation recovery and coverage |
| Random-walk recruitment/F processes | SAM S1–S2 | Partial | E04, E11; no RW density in E23 | F3 | Reuse target vectors; add proper RW initialization and density | Process configuration and active indices | High | Increment likelihood; initial-level identifiability |
| General ARMA(p,q) | No verified example established in this audit | Missing | E07, E23; no lagged-innovation evaluator found | F3 with AR/MA lags | Additional state and initialization rules | Stable/invertible parameterization | Low | Compare analytical covariance and simulated ACF |
| Age/year-correlated biological effects | WHAM W1–W2 | Partial | E10–E16; independent density E07 | F2/F4 | Existing parameters reusable; covariance and index metadata needed | Proper density and dimension contract | High | Distinct age/year signals on unequal dimensions |
| Separable age-by-year covariance | WHAM W1–W2 | Missing | E07, E18, E23 | F4 | Matrix-aware density and group/axis mapping | Multivariate process and variance conventions | High | Small dense covariance oracle; sparse scaling |
| Nonseparable covariance | No verified example established here | Unverified | E07, E23 show no generic implementation | F4 beyond Kronecker form | Candidate only; avoid arbitrary dense covariance first | Identifiable structured covariance | Low | Positive definiteness and simulation identifiability |
| Stochastic annual fleet F | SAM S1–S2 (richer age-state version) | Partial | E11, E16 | F3/F6 | Annual vector can be reused with new process density | F scale and initialization | High | Known F path recovery; catch consistency |
| SAM-style correlated age-specific F states | SAM S1–S2 | Missing | E11, E16 | F4 on age-specific log-F increments | New state representation or explicit equivalent parameterization | Age mapping and multivariate density | Medium | Match a small SAM configuration; no duplicated F/selectivity scale |
| Latent abundance states beyond recruitment | WHAM W1/W6; SAM S2 | Missing | E01–E02, E18 | F5 | State storage, transition means, densities and reporting | State indexing, plus group, random registration | High | Deterministic limit; cohort and plus-group simulation recovery |
| Separation of process and observation likelihoods | WHAM W6; SAM S1 | Implemented | E06–E07, E32–E33 | F1/F12 | Retain separate objective components | None | High | Objective-component accounting and proper density checks |
| Age/year-varying natural mortality | WHAM W2; JJM J1 | Implemented | E10, E15–E16 | F6 | Existing log-M array; correlated models remain partial above | Consistent age-year layout | High | Nonconstant sentinel values and mortality calculations |
| Flexible/time-varying selectivity | WHAM W4; AMAK A1 | Partial | E12, E16 | F7; AR/RW structure is additional | Existing logistic/double-logistic and age-specific forms; smoothing densities/configuration needed | Scale constraints and process layer | High | Confounding with F; AMAK-like penalties assessed separately from RE integration |
| Time-varying maturity and stochastic biology parameters | WHAM W4 provides selected process examples; no universal maturity claim | Partial | E13–E14; growth constraints E19/E37 | F7 | Registration exists; enforce supported dimensions and add process linkage | Process density; growth cardinality audit | Medium | Parameter changes affect only intended year; integration tests |
| Length compositions and growth-derived age-to-length conversion | SS §7; JJM J1 | Implemented | E19–E20 | F7/F10 | Retain fixed/derived conversion paths | Bin mapping and pooled-growth constraints | Medium | Bin sums, tail bins, fixed versus derived conversion |
| Sex-specific population dynamics | SS §7.5; Casal2 C1 | Missing | E17–E19 | F8 with sex categories | New state axis, sex-specific rates and aggregation | Partition/index contract | Medium | Sex aggregation reproduces pooled model when rates match |
| Spatial movement and connected populations | WHAM W4; Casal2 C1; SS §8.4.3 | Missing | E17–E18, E35 | F8 | Transition operator and connected-state indexing | Partitions, schedule, mass conservation | Medium | Closed-system mass balance and zero-movement limit |
| Seasonal processes | Casal2 C1; SS §7.3 | Missing | E18, E22; annual Evaluate loop | F8 | Explicit duration/order and within-year state schedule | Timing contract | Medium | Substep/annual equivalence under constant rates |
| Multiple growth patterns | SS §§7.21/8.5 (morphs/growth patterns); Casal2 C1 categories | Partial | E13, E19–E20, E37 | F7 mixture/partition | Growth modules exist; no operational within-population pattern partition found | Partitions and growth-observation mapping | Medium | One-pattern limit; mixture moments and composition predictions |
| Density-dependent recruitment | WHAM W1; AMAK A1; JJM J1 | Implemented | E02, E34 | F13 Beverton–Holt | Preserve existing mean interface | None | Medium | Stock-recruit limits and lag convention |
| Density dependence in other biological processes | SS §14.1 | Missing | E15–E19, E34 | F13 | Reusable predictor link and dependency scheduling | Covariate/derived-predictor interface | Low | Zero-effect limit and feedback stability |
| Environmental effects on recruitment/M | WHAM W3; SS §14.1 | Missing | E02, E08, E15, E34 | F9 | Covariate input and predictor links; registration alone insufficient | Link-scale and lag rules | Medium | beta=0 equivalence; simulated effect recovery |
| Environmental effects on growth/movement | SS §14.1 for biology; WHAM W3 for movement | Missing | E13, E17–E19 | F9 | Process-specific predictor hooks; movement first needs states | Growth contract or movement operator | Low | Lag/axis tests; recover known response |
| Covariate observation error | WHAM W3 | Missing | E23, E33; no environmental state/input module found | F9 latent E | Latent covariate states, data density and missing-year handling | Process layer and covariate interface | Medium | Zero observation-error limit; missing-data recovery |
| Explicit productivity regimes | JJM J1 | Missing | E34 uses first R0/steepness entries | F9 regime schedule | Regime mapping for recruitment mean and reference quantities | Mean interface extension; fixed schedule first | Medium | Single-regime equivalence; correct boundary years |
| Multinomial age/length observations | SS §7; JJM J1 | Implemented | E20–E23 | F10 | Existing likelihood and prediction operators | None | Medium | Counts/proportions equivalence and sample-size handling |
| Overdispersed / robust composition alternatives | SS §7.14 (DM); WHAM W5 (logistic normal) | Missing | E23; no DM/logistic-normal functor found | F10 | New likelihood and R parser/interface entries | Zero rules and estimable dispersion | High | Multinomial limit; overdispersion recovery; outlier sensitivity |
| Age-reading error | SS §7.16.2 | Implemented | E21 | F11 error matrix | Preserve ordering with length prediction | Valid stochastic error matrix | Medium | Identity and known misclassification matrix |
| Tagging and recapture | Casal2 C1–C2; SS §7.20 | Missing | E17–E18, E23, E35 | F11 | Release cohorts, tag states, loss/reporting processes and observations | Partitions, scheduling, new likelihoods | Low | Release/recovery conservation and reporting-rate recovery |
| Correlated observation errors | SAM S1; WHAM W5 | Missing | E07, E23 | F4/F10 | Joint likelihood with observed-subset covariance | Correlation and missing-data rules | Medium | Dense MVN oracle; gaps must marginalize missing observations |
| Fractional-year survey timing | Casal2 C2; JJM J1 | Missing | E22 explicitly omits timing factor | F11 | Timing input and observation operator; no full seasonal engine needed initially | Define rate/time units | High | tau=0/0.5/1 with analytical survival |
| Projections with fixed future choices | Casal2 C1; SS §6 | Partial | E27, E31; low-level target tests exist | F12 | Extended years work; high-level target-prior setup incomplete | Explicit future parameter rules | High | Existing projection regressions and target achievement |
| Joint stochastic projections | WHAM W4; Casal2 C1; SAM S1 | Partial | E07, E24–E27, E32 | F12 | Simulation hooks exist; process draw/recompute order and forecast orchestration need work | Process simulation and uncertainty contract | High | Variance decomposition; independent future draws; zero-noise limit |
| MSE support | SAM external FLR interface; see limitations below | Unverified | E07, E24, E27–E29; no closed-loop orchestrator found | F12 repeated observation/assessment/decision cycle | Assessment and simulation are ingredients; no verified end-to-end MSE API | Stochastic forecasts, failure handling, HCR interface | Medium | Closed-loop reproducibility and assessment-failure scenarios |
| Retrospective diagnostics | SAM S1 | Implemented | E28–E29 | F12 peeling and Mohn rho | Maintain and extend configuration preservation | None | Medium | Correct peel years; preserve bin and process metadata |
| Simulation-based validation | SAM S1; Casal2 C1 | Partial | E07 and existing integration tests | F12 repeated simulate/refit | Tests and simulation hooks exist; correlated processes need new recovery suite | Process simulation, reproducible fixtures | High | Bias, coverage, convergence and variance-boundary diagnostics |
| Derived quantities and uncertainty | SAM S1; Casal2 C1 | Implemented | E24–E26 | F12 AD and Laplace/delta approximation | Existing infrastructure; new state/covariance outputs must join reporting | Proper likelihood and converged Hessian | High | Derived-quantity gradients; covariance memory; approximation sensitivity |

## Important boundaries and unresolved evidence

- MSE is more than forecasting. [FLfse](https://github.com/flr/FLfse) documents
  SAM integration with FLR for MSE-related work; this is ecosystem support,
  not evidence that SAM alone provides a complete closed-loop platform.
- No verified ARMA or nonseparable-process example was established for the six
  frameworks. These remain design candidates, not demonstrated competitive gaps.
  SAM's unstructured observation covariance is not evidence of nonseparable
  age-by-year process covariance.
- Growth parameter correlations (E37) feed delta-method size variability. They
  must not be mistaken for a general correlated temporal random-effects model.
- A female proportion in spawning biomass is not sex-specific state dynamics.
  Multiple population IDs in C++ are not evidence of movement between them;
  the R initializer constructs one population (E35).
- `RecruitmentBase::PrepareConstrainedDeviations()` exists, but its call in
  `Prepare()` is commented out (E36). Do not advertise automatic centering.
- Fixed recruitment elements still enter the process density (E31). The warning
  and tests make this an explicit scientific semantics decision, not merely a
  user-interface issue. Excluding a density contribution, fixing a state, and
  conditioning on a known state are different operations.
- The model evaluates dynamics before density simulation hooks (E32/E07).
  Drawing a process afterward does not by itself demonstrate that downstream
  predictions are recomputed from that draw.
- High-level setup only chooses catch-at-age (E35), despite broader model-type
  aspirations in README. Surplus-production implementation was not found in the
  inspected model inventory.

## Existing tests inspected and proposed benchmark practice

Relevant existing files include
[SetupRandomEffects](../tests/gtest/test_information_Information_SetupRandomEffects.cpp),
[cohort transitions](../tests/gtest/test_population_CatchAtAge_CalculateNumbersAA.cpp),
[recruitment processes](../tests/gtest/test_recruitmentProcess_SRBevertonHolt_evaluateProcess.cpp),
[size products](../tests/gtest/test_population_dynamics_size.cpp),
[initialization](../tests/testthat/test-initialize_modules.R),
[projections](../tests/testthat/test-projections-looped.R), and
[growth convergence](../tests/testthat/test-integration-vonb-growth-convergence.R).
Slow integration tests have environment gates; presence does not mean they run
in every check. Tests were read, not executed, during this documentation pass.

Begin numerical comparisons with a small synthetic common model, not default
assessment fits. Match units, recruitment age/lag, plus group, initial states,
variance conventions, likelihood constants, observation aggregation and timing.
Then select a pinned WHAM recruitment-only example and a pinned SAM configuration
with age-specific F and latent abundance as separate benchmarks. AMAK Atka and
JJM examples should be used only after reconciling their penalty weights,
regimes and data conventions. Do not compare AIC across unequal datasets or
objectives with different normalization rules.

Implementation update: natural mortality already supported supplied age-year
values in the audited baseline. The subsequent `mortality_process` interface in
`R/mortality_process.R` and `R/initialize_modules.R` adds opt-in independent
age-specific annual log-M random-walk densities using interleaved series in
`GaussianProcessLPDF`. This is parameter-process inference with a common SD,
not stochastic abundance or correlated age effects. Mortality process simulation
and conditional forecasting are now implemented for this same formulation. See the architecture document for the
formulation, dimensionality and validation limits; baseline evidence above is
retained as the historical audit.

Validation update: process recovery, information sensitivity and pointwise
fitted-state interval screening are now reproducible through
`tests/integration/process-recovery/`, including the ten-replicate-per-setting
calibration pilot. This is partial simulation-based validation under known
biology and matching observation models, not demonstrated operational coverage,
forecast/derived-quantity calibration or general model adequacy. The small-M-SD
boundary cases and Monte Carlo uncertainty remain explicit in the roadmap.
