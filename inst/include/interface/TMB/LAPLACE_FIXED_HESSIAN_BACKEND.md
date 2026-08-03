# Pure C++ Laplace Fixed Hessian Notes

## Goal

FIMS needs the fixed-effect covariance for ADREPORT uncertainty without
depending on `obj$he()` or repeated R callbacks. For random-effect models this
requires the Hessian of the Laplace-marginal objective with respect to fixed
effects, not the fixed block of the joint objective Hessian.

## Current TMB surface

For fixed-only models, `obj$he(par.fixed)` calls TMB's model Hessian path.

For random-effect models, `obj$he()` stops with:

```r
stop("Hessian not yet implemented for models with random effects.")
```

The hidden R-side route is:

```r
obj$env$ff(par.fixed, order = 2)
```

However, TMB also has an internal-Laplace mode:

```r
intern_obj <- TMB::MakeADFun(
  data = data,
  parameters = parameters,
  random = random,
  intern = TRUE,
  type = c("ADFun", "Fun", "ADGrad"),
  DLL = DLL
)
intern_obj$env$f(par.fixed, type = "ADGrad", order = 1)
```

`intern = TRUE` applies TMBad's native `laplace` transform, removes the random
parameters from the tape, and leaves a fixed-only marginal objective. TMB's own
documentation says this gives access to an autodiff Hessian with respect to the
fixed effects for random-effect models. This is the closest existing route to a
pure C++ backend: R still constructs the transformed object, but the derivative
calculation is performed by the transformed TMBad tape rather than by repeated R
callbacks to `ff()`.

The relevant `MakeADFun()` logic computes:

```r
n <- length(par)
nr <- length(random)
nf <- n - nr
fixed <- setdiff(1:n, random)
D1h <- h(par, order = 1)
D2h <- h(par, order = 2)
D2f <- f(par, order = 2, cols = random)
D3f <- sapply(random, function(i) {
  f(par, type = "ADGrad", order = 2, rangecomponent = i)
})
I.D2f <- solve(D2f[random, ])
D1eta <- -t(D2f[-random, ] %*% I.D2f)
D3f.D1eta <- D3f %*% D1eta
dim(D3f.D1eta) <- c(n, n, nf)
dim(D3f) <- c(n, n, nr)
D3f.fixed <- D3f[fixed, , ]
D2eta <- sapply(1:nf, function(i) {
  -I.D2f %*% (
    t(D3f.fixed[i, fixed, ]) +
      D3f.D1eta[random, fixed, i] +
      (D3f.fixed[i, random, ] + D3f.D1eta[random, random, i]) %*% D1eta
  )
})
dim(D2eta) <- c(nr, nf, nf)
D2h.fixed <- D2h[fixed, ]
res <- sapply(1:nf, function(i) {
  D2h.fixed[i, fixed] +
    t(D2h.fixed[, random] %*% D1eta[, i]) +
    (t(D2h.fixed[i, random]) +
       t(D2h[random, random] %*% D1eta[, i])) %*% D1eta +
    D1h[, random] %*% D2eta[, , i]
})
```

This is mathematically the quantity FIMS needs for `cov.fixed = solve(res)`.
However, it is only exposed through the R closure.

## Why the existing FIMS C++ fallback is not pure C++

`calculate_fixed_effect_hessian()` is implemented in C++, but it accepts an R
gradient callback. For random-effect models FIMS currently passes a closure
around:

```r
obj$env$ff(x, order = 1)
```

So the finite-difference loop is C++, but every gradient evaluation crosses
back into R and TMB's R-managed `ff()` state.

## Why the joint ADGrad Hessian is not enough

This call can produce a native Hessian-like object without `obj$he()`:

```r
obj$env$f(full_par, type = "ADGrad", order = 1)
```

For random-effect models this is the Hessian of the joint objective with
respect to the full parameter vector. The fixed block is not the Hessian of the
Laplace-marginal objective. It is missing the dependence of the optimized
random effects on the fixed effects and the log-determinant adjustment.

## Existing FIMS native precedent

FIMS already avoids R callbacks for one Laplace derivative piece:

```cpp
TMBADFunLaplaceReverseSweepProvider reverse_provider(adgrad_ptr, parameters);
```

This provider receives `obj$env$ADGrad$ptr` once and calls `EvalADFunObject()`
from C++ to perform reverse sweeps. A pure backend fixed-Hessian API should use
the same pattern: R can pass TMB external pointers and index vectors once, but
the Hessian calculation should not call R functions during derivative work.

## Likely upstream API request

The smallest useful TMB API would expose the internal-Laplace marginal tape, or
its fixed Hessian, for an existing `MakeADFun()` object without needing to build
a second R object:

```cpp
SEXP EvalLaplaceFixedHessian(
    SEXP adfun_ptr,
    SEXP adgrad_ptr,
    SEXP adhess_ptr,
    SEXP parameters,
    SEXP random_indices,
    SEXP control);
```

Where:

- `adfun_ptr` is the joint objective tape.
- `adgrad_ptr` is the ADGrad tape used for reverse sweeps.
- `adhess_ptr` is the sparse Hessian tape created by `MakeADHessObject`.
- `parameters` is the full parameter vector with random effects at the inner
  optimum.
- `random_indices` are zero- or one-based, but the API should state which.
- `control` carries the same Newton/Laplace options used by `ff()`.

An even cleaner R-facing addition would be:

```r
obj$env$laplaceFixedHessian(par.fixed)
```

backed by native code rather than the current R implementation of
`ff(order = 2)`.

## FIMS-side bridge sketch

If TMB exposes the native evaluator, FIMS could add:

```cpp
Rcpp::NumericMatrix calculate_laplace_fixed_hessian_native(
    Rcpp::NumericVector parameters,
    Rcpp::IntegerVector random_indices,
    SEXP adfun_ptr,
    SEXP adgrad_ptr,
    SEXP adhess_ptr,
    Rcpp::List control) {
  SEXP hessian = EvalLaplaceFixedHessian(
      adfun_ptr,
      adgrad_ptr,
      adhess_ptr,
      parameters,
      random_indices,
      control);
  return Rcpp::as<Rcpp::NumericMatrix>(hessian);
}
```

Then `calculate_tmb_fixed_hessian()` can try this before central differencing:

```r
hessian_fixed <- calculate_laplace_fixed_hessian_native(
  parameters = obj$env$last.par.best,
  random_indices = as.integer(obj$env$random - 1L),
  adfun_ptr = obj$env$ADFun$ptr,
  adgrad_ptr = obj$env$ADGrad$ptr,
  adhess_ptr = obj$env$ADHess$ptr,
  control = obj$env$inner.control
)
```

The exact object paths depend on what TMB chooses to expose. Today `ADHess` is
local to `sparseHessianFun()` and not a durable public slot on `obj$env`.

## Current FIMS prototype bridge

FIMS now includes an exposed C++ helper that creates and caches a TMB object
from MakeADFun arguments:

```r
adreport_obj <- InitializeTMBFunction(list(
  data = obj$env$data,
  parameters = obj$env$parameters,
  type = "ADFun",
  ADreport = TRUE,
  DLL = as.character(obj$env$DLL[[1]]),
  silent = obj$env$silent
))
```

Implementation details:

- `InitializeTMBFunction()` calls `TMB::MakeADFun()` from C++ via Rcpp.
- The returned SEXP/list is preserved in a native cache to keep the handle
  available for backend routines.
- `GetInitializedTMBFunction()` and `ClearInitializedTMBFunction()` expose the
  cached object lifecycle.

This does not yet expose native internal-Laplace fixed-Hessian evaluation from
TMB itself, but it gives the backend stable access to initialized TMB objects
without repeatedly rebuilding closures at each call site.

## Current blocker

TMB has the native internal-Laplace transform, but FIMS cannot currently ask an
existing outer `MakeADFun()` object for "the corresponding internal-Laplace
fixed Hessian" as a stable C++ call. The practical FIMS prototype is to build a
second `intern = TRUE` object and evaluate its ADGrad Hessian before falling
back to finite differencing `ff(order = 1)`.

In TMBad builds, direct `EvalADFunObject(..., order = 2)` is not the same
general-purpose full-Hessian path available in the older CppAD branch, so a
native solution should be based on the transformed internal-Laplace tape.
