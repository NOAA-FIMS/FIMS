# Quadra with the XPtr interface

Models are assembled with the existing `create_*()`, setter, and
`initialize_fims()` functions. `CreateTMBModel()` registers the same modules
with TMB and Quadra. TMB remains the default backend.

```r
input <- initialize_fims(data = FIMSFrame(data_big))
initial <- quadra_evaluate(input$parameters$p, input$parameters$re)
fit <- fit_fims(input, backend = "quadra", quadra_method = "joint")
```

`quadra_evaluate()` returns the joint negative log likelihood and its gradient,
ordered as fixed effects followed by random effects. `quadra_fit()` exposes
Quadra fitting without constructing a `FIMSFit`. All parameter vectors use the
ordering returned by `get_fixed()` and `get_random()`.

`quadra_method = "joint"` optimizes fixed and random effects together. Parameter
standard errors come from the inverse joint Hessian via `quadra_sdreport()`.
Derived-quantity standard errors are not included. TMB supplies model reports,
with random effects held at the joint estimates.

`quadra_method = "laplace"` integrates random effects and optimizes the resulting
marginal objective. This differs statistically from a joint fit. Laplace
standard errors and model reports currently use TMB at the Quadra estimates;
`quadra_sdreport()` itself always describes the joint Hessian.

Parameter maps are not yet supported by the Quadra fitting wrapper. Rebuild
with `CreateTMBModel()` after changing module values or structure. `clear()`
invalidates modules and releases both backends. Laplace reconstructs all Quadra
scalars from the XPtr modules on each temporary graph and restores the persistent
graph afterward, so subsequent evaluation and fitting remain valid.

The vendored Quadra source comes from `dev-native-quadra` at
`994a25e49d24835d726f85b75e2562b6eb6b19c4`. The native `.Call` module registry was
not imported. XPtr registration supplies the backend model instead.

Validation includes analytic Gaussian objectives, gradients, Hessians and
fits; comparison against TMB; random effects; repeated evaluations; model
rebuilding and clearing; and evaluation after Laplace fitting.
