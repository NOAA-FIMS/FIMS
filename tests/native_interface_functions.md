# Native Interface: New R and C/C++ Functions

## Purpose

The native interface constructs FIMS backend objects without creating Rcpp
module objects such as `methods::new(Fleet)`. R users call ordinary R helper
functions, which coerce their arguments and invoke registered native routines
with `.Call()`.

The native routines are implemented in C++, but use `extern "C"` entry points
and R's `SEXP` API. In this document, “C function” means this C-compatible ABI,
not that the implementation is written in C.

The complete fitted-model example is in `tests/tmb_call.R`.

## How the layers fit together

1. An exported R function validates or coerces R values.
2. The R function calls a registered `fims_call_*` routine with `.Call()`.
3. The C/C++ routine creates matching backend objects and registers them with
   FIMS `Information` singletons.
4. The routine returns an integer handle to R.
5. Later creation calls use those handles to connect selectivity, fleets,
   growth, maturity, recruitment, and populations.
6. The assembled fixed and random parameter vectors are passed to
   `TMB::MakeADFun()`.

Creation is mirrored for the ordinary `double` backend and, when TMB is
enabled, the automatic-differentiation backend. A shared integer ID identifies
the corresponding object in each registry.

## Estimation types

Creation functions accept estimation types as names or integer codes:

| Name | Code | Meaning |
|---|---:|---|
| `constant` | 0 | Not optimized |
| `fixed_effects` | 1 | Included in the fixed parameter vector |
| `random_effects` | 2 | Included in the random-effects vector |

The internal R helper `.map_estimation_type_code()` performs this conversion
and rejects unknown values. A scalar type may be recycled by the native code;
vectors can specify the type element by element where supported.

## Exported R functions

### Selectivity

| R function | Purpose | Native routine |
|---|---|---|
| `selectivity_logistic_create()` | Register logistic selectivity and return its ID | `fims_call_create_logistic_selectivity` |
| `selectivity_logistic()` | Create and immediately evaluate logistic selectivity | `fims_call_logistic_selectivity` |
| `selectivity_double_logistic_create()` | Register double-logistic selectivity and return its ID | `fims_call_create_double_logistic_selectivity` |
| `selectivity_double_logistic()` | Create and immediately evaluate double-logistic selectivity | `fims_call_double_logistic_selectivity` |

The logistic creator accepts an inflection point, slope, and an estimation type
for each parameter. The double-logistic creator accepts ascending and
descending inflection points and slopes, plus their four estimation types.
Slopes are supplied on the natural scale and stored internally as log-slopes.

### Recruitment

| R function | Purpose | Native routine |
|---|---|---|
| `recruitment_beverton_holt_create()` | Register a Beverton-Holt object and return its ID | `fims_call_create_beverton_holt_recruitment` |
| `recruitment_beverton_holt()` | Create and evaluate expected recruitment | `fims_call_beverton_holt_evaluate_mean` |

The creator accepts `logit_steep`, `log_rzero`, optional `log_devs`, and an
estimation type for each parameter group. The evaluator additionally accepts
spawner abundance and unfished spawners per recruit (`phi_0`).

### Maturity

| R function | Purpose | Native routine |
|---|---|---|
| `maturity_logistic_create()` | Register logistic maturity and return its ID | `fims_call_create_logistic_maturity` |
| `maturity_logistic()` | Create and immediately evaluate logistic maturity | `fims_call_logistic_maturity` |

The creator accepts an inflection point, slope, and their estimation types.

### Growth

| R function | Purpose | Native routine |
|---|---|---|
| `growth_ewaa_create()` | Register empirical weight-at-age growth and return its ID | `fims_call_create_ewaa_growth` |
| `growth_ewaa()` | Create and evaluate weight for year/age inputs | `fims_call_ewaa_growth_evaluate` |

The creator accepts ages, flattened weights at age, number of years, and the
weights' estimation type. The evaluator looks up values using year and age.

### Fleet

| R function | Purpose | Native routine |
|---|---|---|
| `fleet_create()` | Register a fleet, optionally linking selectivity | `fims_call_create_fleet` |
| `fleet_prepare()` | Create a fleet and return transformed `Fmort` and `q` values | `fims_call_fleet_prepare` |

`fleet_create()` accepts yearly `log_fmort`, `log_q`, an optional selectivity
ID, an optional flattened age-to-length conversion matrix, and estimation types
for fishing mortality and catchability. It returns a fleet ID used by
`population_create()` and the likelihood builder.

### Population

| R function | Purpose | Native routine |
|---|---|---|
| `population_create()` | Register and link a population | `fims_call_create_population` |
| `population_prepare()` | Create a population and return transformed `M` and fishing multipliers | `fims_call_population_prepare` |

`population_create()` accepts `log_m`, `log_f_multiplier`, and `log_init_naa`,
their estimation types, optional maturity/growth/recruitment IDs, and a vector
of fleet IDs. This is the point where the separately created process objects
are joined into a population graph.

### Information diagnostics

| R function | Result | Native routine |
|---|---|---|
| `native_information_parameter_counts()` | Counts of registered parameters, random effects, and names | `fims_call_information_parameter_counts` |
| `native_information_model_counts()` | Counts of models, populations, fleets, density components, and data objects | `fims_call_information_model_counts` |

These helpers are useful for tests that verify registration and model assembly.

## Model-level native routines

The model-level routines currently have no exported R convenience wrappers, so
`tests/tmb_call.R` invokes them directly with `.Call()`.

### `fims_call_information_clear()`

Clears the `double` and automatic-differentiation `Information` stores and
resets backend ID counters. Call it before constructing each independent model;
otherwise objects and parameters from the previous model can remain registered.

### `fims_call_create_model()`

Creates the native `CatchAtAge` model container, connects registered
populations and fleets, initializes derived quantities, and finalizes the model
graph. At least one population must already exist. In the current native
workflow this call must occur before `fims_call_build_default_likelihood()`.

### `fims_call_build_default_likelihood()`

Builds the data objects, distributions, and links used by the comparison model.
It takes 14 arguments:

1. fishing fleet ID;
2. survey fleet ID;
3. landings observations;
4. landings coefficient of variation;
5. landings age compositions;
6. landings length compositions;
7. survey index observations;
8. survey coefficient of variation;
9. survey age compositions;
10. survey length compositions;
11. recruitment log standard deviation;
12. number of years;
13. number of ages;
14. number of lengths.

This is currently a model-specific convenience builder rather than a general
API for arbitrary likelihood graphs.

### Parameter accessors

| Native routine | Return value |
|---|---|
| `fims_call_information_get_fixed()` | Numeric vector of registered fixed-effect starting values |
| `fims_call_information_get_random()` | Numeric vector of registered random-effect starting values |
| `fims_call_information_get_parameter_names()` | Character vector in parameter registration order |

The fixed and random vectors become the `p` and `re` entries supplied to
`TMB::MakeADFun()`.

## Minimal model-construction pattern

```r
library(FIMS)

.Call("fims_call_information_clear", PACKAGE = "FIMS")

selectivity_id <- selectivity_logistic_create(
  inflection_point = 2,
  slope = 1,
  inflection_point_estimation_type = "fixed_effects",
  slope_estimation_type = "fixed_effects"
)

fleet_id <- fleet_create(
  log_fmort = rep(log(0.2), n_years),
  log_q = log(1),
  selectivity_id = selectivity_id,
  log_fmort_estimation_type = "fixed_effects"
)

# Create growth, maturity, and recruitment in the same way, then link their
# returned IDs and fleet_id through population_create().
population_id <- population_create(
  log_m = rep(log(0.2), n_years * n_ages),
  log_f_multiplier = rep(0, n_years),
  log_init_naa = rep(log(1000), n_ages),
  log_init_naa_estimation_type = "fixed_effects",
  maturity_id = maturity_id,
  growth_id = growth_id,
  recruitment_id = recruitment_id,
  fleet_ids = fleet_id
)

.Call("fims_call_create_model", PACKAGE = "FIMS")

# Add data and distributions with fims_call_build_default_likelihood(), then:
parameters <- list(
  p = .Call("fims_call_information_get_fixed", PACKAGE = "FIMS"),
  re = .Call("fims_call_information_get_random", PACKAGE = "FIMS")
)

obj <- TMB::MakeADFun(
  data = list(),
  parameters = parameters,
  DLL = "FIMS",
  silent = TRUE
)
```

The abbreviated example leaves `maturity_id`, `growth_id`, `recruitment_id`,
and likelihood inputs undefined intentionally. See `tests/tmb_call.R` for the
complete runnable configuration.

## Adding another native function

The existing modules follow the same implementation pattern:

1. Declare the `extern "C" SEXP` entry point in
   `inst/include/interface/call/<module>.hpp`.
2. Store native objects in a typed singleton registry in
   `inst/include/interface/call/<module>_registry.hpp`.
3. Implement R-to-C++ conversion, validation, registration, and return-value
   construction in `src/call_<module>.cpp`.
4. Register the routine name, function pointer, and exact argument count in
   `CallEntries` in `inst/include/interface/TMB/init_tmb.hpp`.
5. Add an R wrapper in `R/<module>_call.R`, using explicit coercion and
   `PACKAGE = "FIMS"`.
6. Export and document the R wrapper, then add tests for values, invalid input,
   ID linking, and parameter registration.

The registered argument count must match the native signature exactly. Because
dynamic symbol lookup is disabled with `R_useDynamicSymbols(dll, FALSE)`, an
unregistered routine cannot be called by name.
