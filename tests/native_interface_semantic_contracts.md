# Native interface semantic contracts

## Purpose

This document records the scientific and software contracts that must remain
visible across the R, native, C++, and TMB layers after removal of the direct
Rcpp module interface. It is a validation checklist, not a new scientific
specification. If this document conflicts with an established FIMS formulation
or an explicit maintainer decision, that conflict must be resolved rather than
silently choosing one interpretation.

## Cross-layer invariants

- A native object ID identifies the corresponding `double` and automatic-
  differentiation objects. IDs link objects; they are not ages, years, or
  positions in a parameter vector.
- Fixed and random parameter values, names, and ordering must agree between the
  `double` and AD model graphs. Names use
  `Module.object_id.parameter.element` so domain identity survives the native
  boundary.
- Creation routines may mutate registries and parameter stores. Evaluation
  routines must not silently change model configuration.
- `native_clear()` ends the lifetime of all registered model components and
  resets the shared IDs. Handles from before a clear are invalid.
- Flattened arrays retain the ordering defined by their domain component.
  Native conversion must not reorder, normalize, deduplicate, or clamp values.
- Values on natural and transformed scales must remain distinct. In particular,
  `log_Fmort`, `log_q`, `log_M`, `log_rzero`, `log_devs`, `log_init_naa`, and
  likelihood `log_sd` values remain on the log scale. Public selectivity slopes
  are supplied on the natural scale and stored as log-slopes by the native
  selectivity constructor.

## Semantic paths

| Domain concept | R API | Native entry point | Domain implementation | Contract evidence |
|---|---|---|---|---|
| Logistic selectivity | `selectivity_logistic_create()` | `fims_call_create_logistic_selectivity` | `LogisticSelectivity` | Known-value, recycling, constraint, registration, and AD-model tests |
| Double-logistic selectivity | `selectivity_double_logistic_create()` | `fims_call_create_double_logistic_selectivity` | `DoubleLogisticSelectivity` | Known-value, constraint, and registration tests |
| Beverton-Holt recruitment | `recruitment_beverton_holt_create()` | `fims_call_create_beverton_holt_recruitment` | `SRBevertonHolt` | Known-value, deviation, effect-type, and AD-model tests |
| Logistic maturity | `maturity_logistic_create()` | `fims_call_create_logistic_maturity` | `LogisticMaturity` | Known-value, dimension, and registration tests |
| Empirical weight at age | `growth_ewaa_create()` | `fims_call_create_ewaa_growth` | `EWAAgrowth` | Year/age lookup, dimension, and immutability tests |
| Fleet mortality and catchability | `fleet_create()` | `fims_call_create_fleet` | `Fleet` | Natural-scale preparation, linking, and registration tests |
| Population dynamics | `population_create()` | `fims_call_create_population` | `Population` | Natural-scale preparation, component linking, dimensions, and model tests |
| Model graph | `native_create_model()` | `fims_call_create_model` | `CatchAtAge` | Required-component and integration tests |
| Observation likelihoods | `native_build_default_likelihood()` | `fims_call_build_default_likelihood` | FIMS data and density components | Distribution, dimension, objective, report, and derivative tests |
| Parameter priors | `native_add_prior()` | `fims_call_add_prior` | `Dnorm` or `Dlnorm` density component | Target, scale, dimension, and objective tests |

## Statistical and AD validation

Changes on the inference path require more than successful optimization. The
native test suite must verify, for representative models:

1. the objective is finite and its likelihood components have their documented
   meaning;
2. analytical gradients agree with centered finite differences;
3. the Hessian is symmetric and its action agrees with finite differences of
   the gradient;
4. fixed and random classifications preserve the same conditional objective
   when only the classification changes; and
5. parameter and random-effect names align one-to-one with their value vectors.

Cross-interface parity with the removed Rcpp workflow should additionally
compare initial parameters, names, objective components, reports, gradients,
Hessians, optimized values, and convergence at documented tolerances. Rounded
agreement of the final objective alone is not parity evidence.

## Known scope boundary

`native_build_default_likelihood()` currently builds the default catch-at-age
likelihood for exactly one catch fleet and one survey-index fleet. This is an
explicit implementation limitation, not a general statement about the FIMS
scientific model. Expanding or changing that scope requires an explicit model
contract and corresponding scientific, interface, derivative, and regression
tests.
