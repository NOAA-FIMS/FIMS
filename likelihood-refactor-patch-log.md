# Likelihood and Distributions Refactor Patch Log

Branch: `codex-likelihood-primitives`

Updated: 2026-06-01

This file tracks the patch-by-patch implementation work for the likelihood/distribution refactor.

## Architecture Direction

The refactor is moving toward a small likelihood-term architecture:

- distributions own density math only
- parameters own transformations
- `ValueRef` reads constants, vectors, data, model outputs, and transformed parameters through one interface
- `LikelihoodTerm` composes observed/target values, expected/location values, scale values, and a distribution kernel
- `Information` stores mirrored likelihood terms beside legacy density components
- `Model::Evaluate()` can opt into mirrored likelihood-term evaluation through `Information::use_likelihood_terms`

Legacy behavior remains the default unless `use_likelihood_terms` is explicitly enabled.

## Patch 1: Likelihood Primitive Scaffolding

Commit: `2d81282c Add likelihood primitive scaffolding`

Added:

- `inst/include/likelihood/transform.hpp`
- `inst/include/likelihood/parameter.hpp`
- `inst/include/likelihood/value_ref.hpp`
- `inst/include/likelihood/likelihood_term.hpp`
- `inst/include/likelihood/likelihood.hpp`
- `tests/gtest/test_likelihood_primitives.cpp`

## Patch 2: Distribution Density Kernels

Commit: `4c7026cd Add distribution density kernels`

Added math-only normal/lognormal kernels:

- `inst/include/distributions/kernels/normal.hpp`
- `inst/include/distributions/kernels/lognormal.hpp`
- `inst/include/distributions/kernels/distribution_kernels.hpp`
- `tests/gtest/test_distribution_kernels.cpp`

## Patch 3: Kernel Bridge for Legacy Functors

Commit: `ac1657b9 Use kernels in density functors`

Updated existing legacy functors to call the new kernels internally:

- `NormalLPDF` uses `kernels::Normal<Type>::log_density`
- `LogNormalLPDF` uses `kernels::LogNormal<Type>::log_density` for data/priors
- `LogNormalLPDF` uses `log_density_log_scale` for random effects, preserving the legacy no-Jacobian behavior

## Patch 4: Store Likelihood Terms in Information

Commit: `c40be3b6 Store likelihood terms in Information`

Added `Information::likelihood_terms` as side-by-side storage for the new representation.

Updated:

- `Information::Clear()`
- `Information::State()`
- `tests/gtest/test_info_likelihood_terms.cpp`

## Patch 5: Mirror Priors as Likelihood Terms

Commit: `db9c950f Mirror priors as likelihood terms`

Mirrored complete legacy normal/lognormal prior density components into `Information::likelihood_terms`.

Model evaluation remained unchanged.

## Patch 6: Mirror Random Effects as Likelihood Terms

Commit: `8fa27c32 Mirror random effects as likelihood terms`

Mirrored complete legacy normal/lognormal random-effect density components into `Information::likelihood_terms`.

Important preserved behavior:

- lognormal random effects use `log_density_log_scale`, matching the legacy path that omits `-log(x)`

## Patch 7: Mirror Data as Likelihood Terms

Commit: `c577da7b Mirror data as likelihood terms`

Mirrored complete legacy normal/lognormal data density components into `Information::likelihood_terms`.

Added:

- optional `LikelihoodTerm::include` predicate
- NA skipping behavior for mirrored data likelihoods

## Patch 8: Evaluate Mirrored Likelihood Terms

Commit: `ac08a4f9 Evaluate mirrored likelihood terms`

Added log-density evaluation helpers on `Information`:

- `EvaluateLikelihoodTerms(type)`
- `EvaluateLikelihoodTerms()`

Model evaluation remained unchanged.

## Patch 9: Source Lookup for Mirrored Terms

Commit: `8d299897 Find mirrored likelihood terms by source`

Added lookup and single-term evaluation helpers:

- `FindLikelihoodTerm(source_id)`
- `FindLikelihoodTerm(source_id, type)`
- `EvaluateLikelihoodTerm(source_id, type)`

## Patch 10: Explicit Negative Log-Likelihood Helpers

Commit: `35a3bdc1 Add mirrored likelihood NLL helpers`

Added explicit NLL sign-convention helpers:

- `EvaluateNegativeLogLikelihoodTerms(type)`
- `EvaluateNegativeLogLikelihoodTerms()`
- `EvaluateNegativeLogLikelihoodTerm(source_id, type)`

## Patch 11: Opt-In Mirrored Model Evaluation

Commit: `861f124b Add opt-in mirrored likelihood evaluation`

Added `Information::use_likelihood_terms`, defaulting to `false`.

Updated `Model::Evaluate()`:

- default path still uses legacy `density_components`
- opt-in path uses mirrored likelihood-term NLL evaluation by category

Also guarded a TMB-only `m->of = this->of` assignment with `#ifdef TMB_MODEL`.

## Patch 12: Mirrored Setup Parity Test

Commit: `28cd1b3d Test mirrored likelihood setup parity`

Added a parity-hardening test that builds legacy density components, calls the real setup methods, enables `use_likelihood_terms`, and checks `Model::Evaluate()` against expected NLL for:

- normal prior
- lognormal prior
- normal random effect
- lognormal random effect without Jacobian
- normal data with NA skip
- lognormal data

## Patch 13: Centralized Term Creation

Commit: `0b047cfb Centralize mirrored likelihood term creation`

Added `Information::AddLikelihoodTerm(...)`.

Refactored prior, random-effect, and data mirroring to use the helper.

## Patch 14: ValueRef Helper Cleanup

Commit: `aece25c8 Document likelihood refactor and clean up value refs`

Added helper methods in `Information`:

- `ObservedValueRef(d)`
- `ExpectedValueRef(d)`
- `ScaleValueRef(normal)`
- `ScaleValueRef(lognormal)`
- `DataIncludePredicate(d)`

Refactored prior, random-effect, and data mirroring to use those helpers.

## Patch 15: Distribution-Specific Mirroring Helpers

Commit: `ec1f6b65 Extract distribution-specific likelihood mirroring helpers`

Added helper methods in `Information`:

- `TryAddNormalLikelihoodTerm(...)`
- `TryAddLogNormalLikelihoodTerm(...)`

Refactored prior, random-effect, and data mirroring to use those helpers. This
keeps the current normal/lognormal support explicit and makes each new
distribution family a small helper-level addition.

## Patch 16: Unsupported Distribution Boundary Test

Commit: `32ed28ce Test unsupported likelihood mirroring boundary`

Added a focused test that runs a complete multinomial data component through
`SetDataObjects()` and `SetupData()`, then confirms it is not mirrored into
`likelihood_terms` yet. This documents the current boundary while normal and
lognormal are the only mirrored distribution families.

## Patch 17: Multinomial Density Kernel

Commit: `16d7e5df Add multinomial density kernel`

Added a math-only multinomial kernel:

- `inst/include/distributions/kernels/multinomial.hpp`
- aggregate include through `distribution_kernels.hpp`
- direct kernel tests for integer and fractional observations

The kernel matches the current FIMS/TMB convention by evaluating the lgamma
form directly and not rounding observations.

## Patch 18: Multinomial Data Mirroring

Commit: `2ef2579e Mirror multinomial data likelihood terms`

Extended `LikelihoodTerm` with an optional row-wise vector density path, leaving
the existing scalar normal/lognormal path unchanged.

Added multinomial data mirroring in `Information`:

- row-wise likelihood-term creation helper
- `TryAddMultinomialDataLikelihoodTerm(...)`
- data setup now mirrors complete multinomial data components

## Patch 19: Likelihood Term Factory Helpers

Commit: `2fc37463 Add likelihood term factory helpers`

Added named factory helpers in `likelihood_term.hpp`:

- scalar likelihood-term construction
- row-wise likelihood-term construction

Refactored `Information::AddLikelihoodTerm(...)` to use these helpers, keeping
term setup centralized and easier to expose through a friendlier interface.

## Patch 20: Rcpp Mirrored Likelihood Controls

Commit: `d1baedc2 Expose mirrored likelihood controls in Rcpp`

Updated the Rcpp `CatchAtAge` interface with mirrored likelihood controls:

- `UseLikelihoodTerms(TRUE/FALSE)`
- `UsesLikelihoodTerms()`
- `LikelihoodTermCount()`
- `LikelihoodTermNames()`
- `LikelihoodTermSourceIds()`
- `LikelihoodTermTypes()`

The opt-in preference is stored on the Rcpp model interface and applied when
the model is rebuilt, so it survives `CreateTMBModel()` clearing `Information`.

Also updated the TMB-model `fims_math::lgamma` wrapper to consider the global
TMB overload, which is needed when the multinomial kernel is instantiated for
TMBad types during an Rcpp package build.

## Patch 21: R Likelihood-Term Helpers

Commit: `bc6684dc Add R likelihood term helper API`

Added friendly R helpers over the Rcpp mirrored-likelihood controls:

- `use_likelihood_terms(model, value = TRUE)`
- `uses_likelihood_terms(model)`
- `get_likelihood_terms(model)`

The summary helper returns a data frame with `name`, `type`, and `source_id` so
users do not need to call multiple camel-case Rcpp methods directly.

## Patch 22: Remove Filesystem Dependency from Logger

Commit: `7cf5802e Remove filesystem dependency from logger`

Removed the C++ `<filesystem>` dependency from `FIMSLog` path metadata handling
to avoid a Windows/Rtools 45 libstdc++ filesystem compile failure.

Added small portable string-based path helpers for:

- current working directory lookup
- separator normalization
- absolute path detection
- dot-dot path cleanup

## Patch 23: R Objective Parity Test

Commit: `5bc8a85d Add mirrored likelihood R parity test`

Added an R-level parity test that:

- initializes a deterministic age-composition model through the real R helpers
- evaluates the legacy objective
- enables mirrored likelihood terms through `use_likelihood_terms()`
- confirms the mirrored objective matches the legacy objective

Also updated the Rcpp `UseLikelihoodTerms()` method to toggle the already-built
`Information` singletons so the R helper works after `initialize_fims()`.

## Patch 24: Legacy Evaluation Deprecation Warning

Commit: `34d34d1b Add likelihood legacy warning and Rcpp example`

Added a once-per-model deprecation warning when `Model::Evaluate()` uses the
legacy `density_components` objective path instead of mirrored likelihood
terms.

The warning points users to `Information::use_likelihood_terms` and the R
helper `use_likelihood_terms(model, TRUE)`.

## Patch 25: Direct Rcpp Likelihood-Term Example

Commit: `34d34d1b Add likelihood legacy warning and Rcpp example`

Added a small R example that calls the Rcpp `CatchAtAge` likelihood-term methods
directly:

- `UseLikelihoodTerms()`
- `UsesLikelihoodTerms()`
- `LikelihoodTermCount()`
- `LikelihoodTermNames()`
- `LikelihoodTermTypes()`
- `LikelihoodTermSourceIds()`

The example intentionally avoids the R helper layer so developers can see the
underlying Rcpp control surface. A test sources the example and checks that it
returns the expected term table shape.

## Patch 26: Direct Rcpp CatchAtAge Example

Commit: `b7648b2f Add direct Rcpp CatchAtAge example`

Added a fuller CatchAtAge example that builds a small model by creating and
linking Rcpp module objects directly:

- data modules for landings, index, and age composition
- recruitment, growth, maturity, selectivity, fleet, and population modules
- linked data likelihood distributions
- a `CatchAtAge` model with mirrored likelihood terms enabled directly through
  `UseLikelihoodTerms(TRUE)`

The example calls `CreateTMBModel()` and retrieves the direct Rcpp parameter
vectors with `get_fixed()` and `get_random()` without using the high-level R
model initialization or fitting wrappers.

## Patch 27: R Likelihood Specification Constructors

Status: local, not committed yet.

Added pure R specification constructors for the next user-facing likelihood
interface:

- `normal(mean, sd)` for normal distribution specs
- `prior(value, distribution)` for prior roles
- `random_effect(distribution)` for random-effect roles
- `fixed_effect(value)` for fixed-effect roles

The existing `lognormal()` and `multinomial()` family constructors now also
carry FIMS distribution spec metadata while preserving their existing `family`
class behavior.

## Patch 28: R Observation Specification Constructors

Status: local, not committed yet.

Added pure R observation specs for the next user-facing model builder:

- `observe_landings(fleet, data, distribution)`
- `observe_index(fleet, data, distribution)`
- `observe_age_comp(fleet, data, distribution)`
- `observe_length_comp(fleet, data, distribution)`

These specs store fleet names, observed data, and distribution choices without
requiring users to call low-level Rcpp linking methods such as
`set_distribution_links()`.

## Patch 29: R Model Component Specification Constructors

Status: local, not committed yet.

Added pure R component specs for the next user-facing model builder:

- `logistic_selectivity(a50, slope)`
- `logistic_maturity(a50, slope)`
- `ewaa_growth(ages, weights)`
- `beverton_holt(log_rzero, steepness, deviations)`
- `fleet(name, selectivity, fishing_mortality)`
- `population(name, ages, years, growth, maturity, recruitment)`
- `catch_at_age(population, fleets, observations, likelihood)`

These specs describe the model graph by names and nested specs, not by Rcpp
object IDs.

## Patch 30: First R Spec Builder Slice

Status: local, not committed yet.

Added `build_fims()` to translate a small declarative CatchAtAge spec into the
current Rcpp module objects. The first builder slice supports:

- EWAA growth
- logistic maturity
- Beverton-Holt recruitment
- logistic selectivity fleets
- landings and index observations
- normal/lognormal data distributions

The builder performs the low-level Rcpp object creation, observed data links,
expected value links, and likelihood-term opt-in internally.

## Patch 31: Clean R CatchAtAge Builder Example

Status: committed and pushed as `859dea2e Add clean R spec builder layer`.

Added an end-to-end example showing the new R specification layer:

- build a `catch_at_age()` spec
- call `build_fims(spec)`
- inspect mirrored likelihood terms with `get_likelihood_terms(built$model)`

The example intentionally avoids low-level Rcpp calls such as
`set_distribution_links()`, `set_observed_data()`, `methods::new()`, and
`UseLikelihoodTerms()`.

## Patch 32: initialize_fims Spec Bridge

Status: committed and pushed as `cc3cbdb8 Add clean spec fit bridge and role helpers`.

Added an `initialize_fims()` bridge for `fims_model_spec` objects. The bridge
calls `build_fims()` and returns the existing `fit_fims()` input shape:

- `parameters = list(p = get_fixed(), re = get_random())`
- `model = built$model`
- `built = built`

Updated the clean R builder example to show:

- `initialized <- initialize_fims(spec)`
- `fit <- fit_fims(initialized, optimize = FALSE)`

## Patch 33: Friendly Parameter Role Names

Status: committed and pushed as `cc3cbdb8 Add clean spec fit bridge and role helpers`.

Added user-facing role helpers for the R specification layer:

- `estimate(value)`: estimated fixed effect
- `constant(value)`: non-estimated parameter
- `random(distribution)`: random effect shorthand

The existing `fixed_effect()` and `random_effect()` helpers remain available for
compatibility and for users who prefer the more explicit statistical wording.
The clean CatchAtAge builder example now uses the friendlier names so users can
see estimated, constant, and random quantities directly in the model spec.

## Patch 34: Model Graph Tibble Primitives

Status: committed and pushed as `6cb84025 Add shared module graph specs`.

Added first-pass graph representation helpers:

- `module(id, spec)`: define a reusable module once
- `ref(id)`: point another module field to a reusable module
- `as_model_graph(x)`: convert module specs into `modules` and `links` tibbles

This patch lets the R layer represent shared submodules without building them
yet. For example, two populations can link to the same `growth.shared` module,
and two fleets can link to the same `selectivity.shared` module. The next patch
can teach the builder to consume this graph and instantiate shared Rcpp objects
once.

## Patch 35: Model Graph Tibble Round Trip

Status: committed and pushed as `cd187fe7 Add model graph tibble round trip`.

Added explicit conversion helpers for model graph tibbles:

- `model_graph_tibbles(graph)`: extract plain `modules` and `links` tibbles
- `model_graph_from_tibbles(modules, links)`: rebuild a graph object from
  those tibbles

Updated `inst/examples/shared_module_graph.R` to show the full workflow:

1. define shared modules
2. create a graph
3. convert the graph to tibbles
4. rebuild the graph from those tibbles

## Patch 36: Shared Module Graph Data Demo

Status: local, not committed yet.

Added data modules to `inst/examples/shared_module_graph.R` so the demo shows:

- shared biological/process modules
- population and fleet modules
- landings and index observation modules
- conversion to `modules` and `links` tibbles
- rebuilding the graph from those tibbles

## Current State

The branch currently has a side-by-side likelihood-term architecture:

- legacy behavior remains default
- mirrored likelihood-term behavior is available behind `Information::use_likelihood_terms`
- priors, random effects, and data are mirrored for normal/lognormal distributions
- multinomial data can be mirrored as row-wise likelihood terms
- unsupported distribution families are set up through the legacy path but are
  not mirrored until an explicit likelihood-term helper is added
- model-level opt-in evaluation exists and has focused parity coverage

Patch 36 local uncommitted work:

- `inst/examples/shared_module_graph.R`: added landings and index modules
- `tests/testthat/test-model-graph.R`: demo coverage for observation modules
- `likelihood-refactor-patch-log.md`: patch 36 entry
  coverage

Note: `inst/examples/catch_at_age_rcpp_interface.R` also has a separate local
edit from earlier work and is intentionally not part of Patch 36.

Note: `docs/likelihoods-distributions-refactor-chat.md` was also updated locally, but `docs/` is ignored by this repository, so this root-level file is the tracked version intended for commits.
