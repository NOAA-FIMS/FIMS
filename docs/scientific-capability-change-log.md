# Scientific capability change log and platform inspiration

Updated 2026-11-17. Scope: `investigate/scientific-capability-gaps`, baseline
`f028bff`. This is the maintained outline of this investigation's changes, their
scientific precedents, and their current limits. The [gap matrix](scientific-capability-gaps.md)
retains the original baseline audit; the [architecture](stochastic-process-architecture.md)
and [roadmap](scientific-capability-roadmap.md) provide formulation and priorities.

## Attribution rules

“Inspired by” identifies a documented scientific precedent or comparison that
informed the work. It does **not** mean code was copied, the reference platform's
parameterization was reproduced, or numerical equivalence was demonstrated.
Where the historical record establishes only a conceptual relationship, the
entry says so. FIMS-specific engineering and findings must not be assigned an
external origin merely because another platform has related functionality.

Implemented means an executable pathway exists within the stated scope;
benchmark means validation/diagnostic code, not a new assessment capability;
planned means no implementation claim. Tests and pilots are evidence, not
operational certification. No external framework executables were compared in
these increments. Earlier pilot results and their source fingerprints are retained.

## Reference platforms and their role

References below were checked on 2026-11-17; live documentation may change.
The original audit preserves its earlier versions/retrieval qualifications.

| Reference | Verified precedent used here | Attribution limit |
|---|---|---|
| [WHAM numbers-at-age configuration](https://timjmiller.github.io/wham/reference/set_NAA.html) | Recruitment-only random effects, IID/year-AR1 options and variance maps motivate the initial process work. | FIMS does not thereby implement WHAM's full latent abundance/survival model, stock/region layout or age/year covariance options. |
| [WHAM mortality configuration](https://timjmiller.github.io/wham/reference/set_M.html) | Estimable/stochastic mortality and shared parameter configurations provide the biological-process precedent. | FIMS's independent age-specific annual RW1 is its own scoped formulation, not a claim to reproduce every WHAM M option or a documented WHAM RW1 parameterization. |
| [SAM stockassessment manual](https://fishfollower.r-universe.dev/stockassessment/doc/manual.html) | Log-F random walks, variance coupling (`keyVarF`), simulation/re-estimation (`simstudy`) and forecasts provide scientific/methodological precedents. | FIMS's fleet-level annual F and SD groups are simpler than SAM's state configuration. FIMS has not reproduced SAM's full log-N/log-F assessment. |
| [Stock Synthesis manual, composition errors/data weighting](https://nmfs-ost.github.io/ss3-doc/SS330_User_Manual_release.html) | Dirichlet–multinomial composition options motivate overdispersion stress testing and the new likelihood. | The generic total concentration `kappa` and FIMS interface are not presented as an implementation of SS3's parameter interface or all composition-weighting options. |
| [Casal2](https://casal2.github.io/), [AMAK](https://github.com/afsc-assessments/AMAK), [JJM guide proposal](https://www.sprfmo.int/assets/Meetings/02-SC/12th-SC-2024/Jack-Mackerel/SC12-JM08-PER-Joint-Jack-Mackerel-Model-User-Guide.pdf) | Included in the six-framework gap investigation and broader roadmap. | No current production increment is attributed specifically to these three. Assessment-specific/proposal evidence must not be generalized. See the audit's detailed source qualifications. |
| Existing FIMS/TMB architecture | Parameter registration, linked vectors, AD/Laplace fitting, factor maps and reporting are the implementation foundation. | This is existing computational infrastructure, not a newly imported fisheries platform. No new dependency or Quadra backend was introduced. |

## Implemented capability changes

The initial combined commit contains several increments; those entries share a
commit ID rather than implying a finer history that does not exist.

| Change / commit | What changed and present scope | Inspiration / implementation origin | Main evidence and remaining limits |
|---|---|---|---|
| Six-framework investigation — `9ea49e1` | Added scientific gap matrix, process architecture and implementation roadmap. | Comparison with WHAM, SAM, SS3, AMAK, Casal2 and JJM; FIMS source inspection determined its actual baseline. | [Gap matrix](scientific-capability-gaps.md). A documentation audit is not numerical cross-platform validation. |
| IID/AR1 recruitment — `9ea49e1` | Reusable normalized Gaussian process density; opt-in recruitment deviations/direct log-R links, stationary AR1 initialization, innovation SD and transformed correlation. | WHAM recruitment/correlation precedent; existing FIMS recruitment targets and density registration supplied the architecture. | [Kernel](../inst/include/distributions/functors/gaussian_process_lpdf.hpp), [R connection](../R/gaussian_process.R), [tests](../tests/testthat/test-gaussian-process.R). No latent older-age abundance or general ARMA. |
| Recruitment simulation and conditional forecasts — `9ea49e1` | Seeded process paths, dynamics recomputation and observation draws within a supplied horizon. | WHAM/SAM simulation and forecast precedents; scheduling/RNG ownership are FIMS-specific design. | [Driver](../R/simulate_fims.R), [tests](../tests/testthat/test-process-simulation.R). No automatic posterior parameter draw or management feedback loop. |
| Annual fleet log-F RW1 — `9ea49e1` | Independent fleet random walks, explicit fixed/proper initial rules, inference and trajectory simulation. | SAM log-F process is the primary scientific precedent; annual scalar fleet states and initialization contract are FIMS-specific. | [Configuration](../R/fishing_process.R), [tests](../tests/testthat/test-fishing-random-walk.R). No correlated age-specific SAM F-state reproduction. |
| Shared fleet innovation SDs — `9ea49e1` | Explicit `sd_group` labels tie SD parameters through TMB maps; paths remain separate. | Conceptually related to SAM variance coupling/WHAM variance maps; actual map construction and reporting adapt existing FIMS interfaces. | [Configuration/map helper](../R/fishing_process.R). No shared innovations or cross-fleet covariance implied. |
| Age-specific mortality RW1 inference — `85d4c9a` | Independent annual log-M chains on the existing age-year grid, common innovation SD, fixed/proper initialization; interleaved-series kernel support. | WHAM supplies the time-varying-M precedent; FIMS indexing and the independent RW1 formulation are scoped design choices. | [R connection](../R/mortality_process.R), [tests](../tests/testthat/test-mortality-random-walk.R). No common annual shock, cohort process or age correlation. |
| Mortality simulation and forecasts — `053646a` | Applies full M grids before dynamics, conditions historical cells, restores native targets and records path metadata. | General WHAM/SAM process-aware prediction precedent; native restoration and scheduling respond to FIMS's model lifecycle. | [R driver](../R/simulate_fims.R), [native evaluator](../inst/include/interface/rcpp/rcpp_objects/rcpp_models.hpp). Supplied parameters/horizon, not full assessment uncertainty propagation. |
| DM composition likelihood — `b3e79b2` | Opt-in fixed/estimated concentration per fleet/type, normalized row likelihood, zero/missing handling, AD and fleet reporting; multinomial default retained. | SS3 composition-overdispersion precedent. Kernel, concentration convention and initializer override are FIMS-specific implementations. | [Kernel](../inst/include/distributions/functors/dirichlet_multinomial_lpmf.hpp), [configuration](../R/composition_likelihood.R), [tests](../tests/testthat/test-dirichlet-multinomial.R). Inference only; production DM observation simulation and broad dispersion calibration remain open. |
| Saved-mode uncertainty handoff — `3ff7ea9` | Plain fixed/random snapshots, exact input/layout checks and objective checks before/after state-only uncertainty reports; metadata JSON guard. | FIMS-specific response to a reproduced cold-rebuild failure, using existing TMB facilities. No external platform provenance is claimed for this fix. | [Snapshot helper](../tests/integration/process-recovery/mode-handoff.R), [pilot](../tests/integration/process-recovery/mode-handoff-pilot/README.md). Benchmark pathway, not a new public fit-persistence API; local consistency does not prove a global mode. |

## Validation and diagnostic upgrades

The simulation/re-estimation approach has a general precedent in SAM/WHAM.
The particular designs, pairing, thresholds and failures below arose from this
FIMS investigation; they are not claimed as copied framework diagnostics.

| Upgrade / commit | Purpose and inspiration | Recorded outcome and limitation |
|---|---|---|
| Paired recovery/identifiability — `1cffc3e` | SAM-style simulate/re-estimate precedent; FIMS-specific oracle/process/initialization comparisons. | [Pilot](../tests/integration/process-recovery/pilot/README.md): 70 fits, 12 SD-boundary cases. Small correct-model screening, not established coverage. |
| Longer horizons, starts and profiles — `50b323e` | FIMS-specific follow-up to variance instability; generic likelihood profiling/multiple-start practice rather than a specific platform feature. | [Pilot](../tests/integration/process-recovery/sensitivity-pilot/README.md): 132 attempts, 129 passing checks; longer series did not consistently resolve M variance. |
| Observation-information sensitivity — `a91bc36` | FIMS-specific factorial experiment motivated by the preceding results; common errors/nested samples control comparisons. | [Pilot](../tests/integration/process-recovery/information-pilot/README.md): 94/96 attempts passed; richer compositions improved state recovery, without proving SD calibration. |
| Interval calibration / true-SD sensitivity — `1b54c03` | General simulation-validation precedent; FIMS-specific paired intervals, MCSEs, boundary strata and SD settings. | [Pilot](../tests/integration/process-recovery/calibration-pilot/README.md): 60 uncertainty reports; small-SD boundaries remain. Pointwise fitted-state coverage, not forecast/derived-quantity calibration. |
| Composition misspecification / mode diagnosis — `72adf28` | SS3's overdispersion option motivated the DM stress hierarchy; FIMS failures motivated the cold/warm investigation. | [Pilot](../tests/integration/process-recovery/robustness-pilot/README.md): known-SD M coverage fell from about 95% to 57% in the strongest case; estimated-SD inference became unreliable. Five-replicate stress evidence. |
| Matched-DM / saved-mode comparison — `3ff7ea9` | Combines the SS3-inspired likelihood family with the FIMS-specific handoff fix. | [Pilot](../tests/integration/process-recovery/mode-handoff-pilot/README.md): one dataset shows numerical handoff consistency and reduced state distortion, but both covariance fits remain at bounds. Not replicated calibration. |

## Next task: replicated matched-DM validation

**Planned; not implemented by this documentation update.** Reuse the existing
composition stress datasets rather than changing their truth or deleting failed
outcomes.

1. Compare the multinomial fit with DM fits at known and estimated concentration.
2. Repeat across datasets, overdispersion strengths and covariance/inner-state starts.
3. Use saved-mode reports, retaining cold/warm disagreements, boundaries and failed reports.
4. Measure concentration/process-SD recovery, state error and interval coverage with
   explicit denominators and replicate-level Monte Carlo uncertainty.
5. Decide whether evidence supports extending the production simulator to DM errors;
   keep forecast/derived-quantity uncertainty as a separate validation target.

This follows the scientific precedent of SS3 composition errors and SAM/WHAM
simulation validation, but the benchmark design is FIMS-specific. No additional
covariance, partition, environmental or backend feature is claimed complete.

## Maintenance contract

For every completed scientific or architectural upgrade on this investigation
branch, update this file in the same commit:

- State the implemented scope, benchmark-only status or remaining proposal.
- Record the known commit, source/test/experiment links, API/formulation changes
  and material validation limits. A current upgrade can be identified by its
  path/title until its hash is available; pin that hash at the next update.
- Identify the reference platform and authoritative source, or explicitly say
  FIMS-specific / no direct attribution. Do not invent historical inspiration.
- Preserve earlier evidence and failed cases; distinguish conceptual precedent,
  direct adaptation and demonstrated numerical equivalence.
- Update the next-task section when priorities change. Keep the baseline audit
  separate from current implementation status.
