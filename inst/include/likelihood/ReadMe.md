# Likelihood Module

The likelihood module provides Rcpp-accessible likelihood components that can be
used for data likelihoods, priors, random effects, and penalties. It lives in
parallel to the older distribution module and is intended to make likelihood
objects explicit model components.

## Available Likelihoods

- `NormalLikelihood`
- `LognormalLikelihood`
- `GammaLikelihood`
- `InvGammaLikelihood`
- `MultinomialLikelihood`

All likelihoods derive from `LikelihoodBase` and share these members:

- `observed_values`: `RealVector` used when the likelihood owns its observed
  input directly.
- `expected_values`: `RealVector` for means, probabilities, or expected values.
- `real_input`: linked `RealVector` input, set with `set_real_input()`.
- `nll_components`: per-observation negative log-likelihood contributions.
- `set_real_input(input, role)`: links a `RealVector` as the input.
- `set_parameter_input(input, role)`: links a `ParameterVector` as the input.
- `set_real_expected_input(expected)`: links a `RealVector` as expected values.
- `set_parameter_expected_input(expected)`: links a `ParameterVector` as
  expected values.
- `set_role(role)`: sets the likelihood role without linking a new input.
- `evaluate()`: evaluates the likelihood outside the TMB model.

Supported roles are:

- `"data"`
- `"prior"`
- `"random_effect"` or `"random_effects"` or `"re"`
- `"penalty"`

## R Helper For RealVector Values

The Rcpp interface uses `RealVector`, so values are usually filled with a small
helper:

```r
set_real_vector <- function(x, values) {
  x$resize(length(values))
  for (i in seq_along(values)) {
    x$set(i - 1L, values[i])
  }
  invisible(x)
}
```

## Direct Evaluation

Direct evaluation uses `observed_values` unless an input has been linked with
`set_real_input()` or `set_parameter_input()`.

```r
likelihood <- methods::new(NormalLikelihood)

set_real_vector(likelihood$observed_values, c(1, 2))
set_real_vector(likelihood$expected_values, c(0.5, 1.5))
set_real_vector(likelihood$log_sd, log(2))

likelihood$evaluate()
likelihood$nll_components$toRVector()
clear()
```

## Linked Expected Values

Use `set_real_expected_input()` when observed values are owned by the likelihood
but expected values are produced by a model or derived quantity.

```r
likelihood <- methods::new(LognormalLikelihood)

set_real_vector(likelihood$observed_values, observed_landings)
set_real_vector(likelihood$log_sd, log_sd)
likelihood$set_parameter_expected_input(fleet$log_landings_expected)
```

## Linked RealVector Input

Use `set_real_input()` when the likelihood should read from an existing
`RealVector`. This is useful for linking to derived quantities that have been
registered in `Information::variable_map`.

```r
input <- methods::new(RealVector)
likelihood <- methods::new(NormalLikelihood)

set_real_vector(input, 1)
set_real_vector(likelihood$expected_values, 0)
set_real_vector(likelihood$log_sd, 0)

likelihood$set_real_input(input, "prior")
likelihood$evaluate()
clear()
```

## Linked ParameterVector Input

Use `set_parameter_input()` when the likelihood should act on a model
parameter. This is the preferred pattern for priors.

```r
selectivity <- methods::new(LogisticSelectivity)
selectivity$slope[1]$value <- 1.5
selectivity$slope[1]$estimation_type$set("fixed_effects")

prior <- methods::new(NormalLikelihood)
set_real_vector(prior$expected_values, 1.5)
set_real_vector(prior$log_sd, log(3))
prior$set_parameter_input(selectivity$slope, "prior")
```

When the model is built, `CreateTMBModel()` calls `add_to_fims_tmb()` on live
likelihood objects. The linked frontend `ParameterVector` id is used to resolve
the backend vector from `Information::variable_map`.

## Likelihood Parameters

### NormalLikelihood

Fields:

- `expected_values`: mean.
- `log_sd`: log standard deviation.

Equivalent density:

```r
-dnorm(input, mean = expected_values, sd = exp(log_sd), log = TRUE)
```

### LognormalLikelihood

Fields:

- `expected_values`: meanlog.
- `log_sd`: log sdlog.

Equivalent density:

```r
-dlnorm(input, meanlog = expected_values, sdlog = exp(log_sd), log = TRUE)
```

### GammaLikelihood

Fields:

- `expected_values`: mean.
- `log_sd`: log standard deviation.

The backend converts mean and standard deviation to gamma shape and scale:

```r
shape <- (mean / sd)^2
scale <- sd^2 / mean
```

### InvGammaLikelihood

Fields:

- `log_shape`: log inverse-gamma shape.
- `log_scale`: log inverse-gamma scale.

### MultinomialLikelihood

Fields:

- `observed_values`: counts.
- `expected_values`: probabilities.
- `dims`: two-element vector, `c(n_rows, n_cols)`.

For one multinomial row:

```r
likelihood <- methods::new(MultinomialLikelihood)
set_real_vector(likelihood$observed_values, c(2, 1, 1))
set_real_vector(likelihood$expected_values, c(0.5, 0.25, 0.25))
set_real_vector(likelihood$dims, c(1, 3))
likelihood$evaluate()
clear()
```

## Backend Integration

Each Rcpp likelihood interface implements `add_to_fims_tmb()`. This creates
backend likelihood objects for both TMB scalar types and registers them in:

```cpp
Information<Type>::likelihood_components
```

The model evaluates likelihood components after the legacy distribution
components:

```cpp
for (l_it = likelihood_components.begin();
     l_it != likelihood_components.end(); ++l_it) {
  nll_vec[nll_vec_idx] = l_it->second->Evaluate();
  jnll += nll_vec[nll_vec_idx];
}
```

Linked inputs are resolved as follows:

- `set_parameter_input()` uses the frontend `ParameterVector` id as the
  `variable_map` key.
- `set_real_input()` uses the frontend `RealVector` id as the `variable_map`
  key.
- Fixed hyperparameters such as `expected_values`, `log_sd`, `log_shape`, and
  `log_scale` stay local to the likelihood object.

This avoids accidental aliasing with unrelated backend vectors while still
allowing likelihood inputs to link to model parameters or derived quantities.

## Derived Quantity Links

Derived quantities should be registered in `Information::variable_map` with the
frontend `RealVector` id as the key. A likelihood can then use that derived
quantity as its input:

```r
likelihood <- methods::new(LognormalLikelihood)
set_real_vector(likelihood$expected_values, log_expected_value)
set_real_vector(likelihood$log_sd, log_sd)
likelihood$set_real_input(fleet_derived_quantities$log_index_expected, "data")
```

The important contract is that the frontend id used by R must be the same key
used by the backend vector in `variable_map`.

## Simulation

Likelihood functors include `FIMS_SIMULATE_F(this->of)` blocks guarded by
`#ifdef TMB_MODEL`. During simulation, each likelihood writes simulated values
back into its active input source:

- unlinked likelihoods update `observed_values`.
- linked `RealVector` or `ParameterVector` likelihoods update the linked backend
  vector.

This allows TMB simulation to use the same likelihood objects that contribute to
the objective function.

## Testing

Focused tests live in:

- `tests/gtest/test_likelihood_functors.cpp`
- `tests/testthat/test-rcpp-likelihood.R`
- `tests/testthat/test-integration-fims-bayesian-prior-predictive-likelihood.R`

Useful commands:

```sh
Rscript -e 'devtools::test(filter = "rcpp-likelihood")'
Rscript -e 'library(FIMS); testthat::test_file("tests/testthat/test-integration-fims-bayesian-prior-predictive-likelihood.R")'
```

For C++ tests, use the repository gtest workflow:

```r
FIMS:::setup_and_run_gtest()
```
