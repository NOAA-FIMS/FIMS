# TMB ADREPORT Derived Quantity Uncertainty

This directory contains the backend boundary for calculating standard errors for
derived quantities reported through TMB `ADREPORT`.

The design intentionally separates four jobs:

1. A derivative provider supplies raw ADREPORT derivative ingredients.
2. The payload extractor assembles those ingredients into a fixed-effect or
   Laplace payload.
3. The payload uncertainty calculator dispatches the payload to the correct
   backend uncertainty adapter.
4. The common delta-method calculator performs the final standard error math.

## Current Flow

The current implementation is a hybrid native/R path:

- ADREPORT values and full Jacobian are evaluated through the native TMBad
  `ADFun` external pointer when available, with the R-facing `$fn`/`$gr` path as
  a fallback.
- Fixed-effect covariance is calculated by inverting a fixed-effect Hessian in
  the backend. For fixed-effect-only models, the Hessian is obtained through
  TMB's model-DLL fixed-Hessian path (`obj$he()`), with backend central
  differencing of the fixed-effect gradient as a fallback. For random-effect
  models, the Hessian is calculated in the backend by central differencing
  TMB's Laplace marginal gradient, with the same `optimHess()` path used by
  `sdreport` as a fallback; `sdreport$cov.fixed` remains a fallback.
- Random-effect Hessian is extracted from the native ADGrad Hessian block when
  available, with TMB's sparse Hessian helper as a fallback.
- Reverse sweeps use the native TMBad `ADGrad` external pointer when available,
  with the R-facing ADGrad function as a fallback. The Hessian solve and
  fixed-Jacobian adjustment assembly are owned by the backend calculator.

R passes those raw pieces to `assemble_adreport_payload()`. The Rcpp bridge then
wraps them in `StaticADReportDerivativeProvider` and calls
`ADReportPayloadExtractor`. From that point on, payload assembly and uncertainty
calculation are owned by the FIMS backend.

## Backend Types

`ADReportPayloadExtractionInput`
: Raw derivative ingredients. Matrices are row-major. `random_indices` are
  zero-based C++ positions in the full parameter vector.

`ADReportDerivativeProvider`
: Interface for objects that can provide `ADReportPayloadExtractionInput`.

`StaticADReportDerivativeProvider`
: Provider for the current R/TMB path. It simply returns an already assembled
  derivative bundle.

`NativeTMBADReportHandle`
: Typed C++ contract for native objects that can expose
  `ADReportPayloadExtractionInput`.

`TMBADFunADReportHandle`
: Header-only implementation of `NativeTMBADReportHandle` for an ADFun-like
  ADREPORT object. It evaluates the report values and row-major Jacobian from
  the native function, carries the fixed-effect covariance, and can pass through
  random-effect Hessian and fixed-Jacobian adjustment inputs when those have
  been computed elsewhere.

`NativeTMBADReportDerivativeProvider`
: Provider for the future direct TMB path. The raw pointer constructor remains a
  placeholder for low-level TMB handles and throws when evaluated. The typed
  `NativeTMBADReportHandle` constructor delegates extraction to the native
  handle and can already feed `ADReportPayloadExtractor`.

`ADReportPayloadExtractor`
: Splits full Jacobians into fixed and random blocks, computes random
  covariance from the random Hessian, and assembles fixed, fixed-after-Laplace,
  or Laplace payloads.

`ADReportPayloadUncertaintyCalculator`
: Dispatches assembled payloads to the fixed-effect or Laplace uncertainty
  adapter.

## Future Native TMB Path

The intended replacement path is now:

1. Build or retrieve a native TMB ADREPORT ADFun.
2. Wrap it in `TMBADFunADReportHandle`.
3. Keep `ADReportPayloadExtractor` and `ADReportPayloadUncertaintyCalculator`
   unchanged.
4. Move the fixed covariance, random Hessian, and reverse-sweep adjustment
   extraction behind native handle implementations.
5. Remove R-side derivative assembly only after the native provider can produce
   the same payloads as the static provider.

The native provider should be considered complete only when fixed-effect,
fixed-after-Laplace, and random-effect/Laplace ADREPORT tests match TMB
`sdreport` results.
