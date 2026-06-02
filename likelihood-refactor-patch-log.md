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

Status: local, not committed yet.

Added friendly R helpers over the Rcpp mirrored-likelihood controls:

- `use_likelihood_terms(model, value = TRUE)`
- `uses_likelihood_terms(model)`
- `get_likelihood_terms(model)`

The summary helper returns a data frame with `name`, `type`, and `source_id` so
users do not need to call multiple camel-case Rcpp methods directly.

## Current State

The branch currently has a side-by-side likelihood-term architecture:

- legacy behavior remains default
- mirrored likelihood-term behavior is available behind `Information::use_likelihood_terms`
- priors, random effects, and data are mirrored for normal/lognormal distributions
- multinomial data can be mirrored as row-wise likelihood terms
- unsupported distribution families are set up through the legacy path but are
  not mirrored until an explicit likelihood-term helper is added
- model-level opt-in evaluation exists and has focused parity coverage

Current local uncommitted work:

- `R/likelihood_terms.R`: patch 21 R helper API
- `NAMESPACE`: patch 21 exports
- `R/FIMS-package.R`: patch 21 roxygen export hints
- `tests/testthat/test-likelihood-terms.R`: patch 21 R helper tests
- `likelihood-refactor-patch-log.md`: patch 21 entry

Note: `docs/likelihoods-distributions-refactor-chat.md` was also updated locally, but `docs/` is ignored by this repository, so this root-level file is the tracked version intended for commits.
