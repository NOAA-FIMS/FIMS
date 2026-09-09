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

## Local validation

On macOS arm64 with R 4.4 and Apple clang 16:

- The focused XPtr/Quadra suite passes, including catch-at-age objective,
  gradient, fitted-parameter and covariance comparisons against TMB.
- All 70 C++ tests pass; Doxygen builds successfully.
- The optimized source package builds, installs, loads and unloads successfully.
- `R CMD check --no-manual --no-vignettes --no-tests` completes with no errors,
  three warnings and six notes. Warnings concern existing compiler flags and
  omitted vignette output. Notes include long vendored Eigen paths, installed
  size, existing R documentation/global bindings, GNU make, and Quadra stdout.
  The R regression suite is run separately against the optimized package.

The system's default Homebrew compiler could not locate the macOS System
library. Validation used `/usr/bin` ahead of Homebrew on `PATH` and
`SDKROOT=$(xcrun --show-sdk-path)`, without changing the user's compiler settings.

The full R regression suite completes with one pre-existing failure:
`test-integration-fims-estimation-random-effects-without-wrappers.R`, recruitment
`log_r` estimation, raises `NA/NaN gradient evaluation` in `stats::nlminb()` before
reaching its existing skip statement. Running that file against the pre-Quadra
XPtr checkpoint `bb2d5945` reproduces the same warning and error. All other tests
in the full run pass or take their existing skips; three additional warnings
come from the proportion-female test's many-to-many joins. No test expectations
or skip placement were changed to conceal this baseline failure.

Formatting used the available clang-format 21 and styler. CSpell could not run
because neither CSpell nor npm is installed locally; vendored Quadra files are
excluded from the repository spelling scan, and backend names were added to
the project dictionary.
