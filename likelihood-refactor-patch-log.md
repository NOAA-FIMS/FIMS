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

Status: local, not committed yet.

Added a focused test that runs a complete multinomial data component through
`SetDataObjects()` and `SetupData()`, then confirms it is not mirrored into
`likelihood_terms` yet. This documents the current boundary while normal and
lognormal are the only mirrored distribution families.

## Current State

The branch currently has a side-by-side likelihood-term architecture:

- legacy behavior remains default
- mirrored likelihood-term behavior is available behind `Information::use_likelihood_terms`
- priors, random effects, and data are mirrored for normal/lognormal distributions
- unsupported distribution families are set up through the legacy path but are
  not mirrored until an explicit likelihood-term helper is added
- model-level opt-in evaluation exists and has focused parity coverage

Current local uncommitted work:

- `tests/gtest/test_info_likelihood_terms.cpp`: patch 16 unsupported boundary test
- `likelihood-refactor-patch-log.md`: patch 16 entry

Note: `docs/likelihoods-distributions-refactor-chat.md` was also updated locally, but `docs/` is ignored by this repository, so this root-level file is the tracked version intended for commits.
