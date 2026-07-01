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

The current implementation still gets raw derivative pieces from TMB's R-facing
API:

- ADREPORT values and full Jacobian come from a TMB `ADFun`.
- Fixed-effect covariance comes from `sdreport`.
- Random-effect Hessian and reverse-sweep adjustments come from TMB environment
  helpers.

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

`NativeTMBADReportDerivativeProvider`
: Placeholder for the future direct TMB path. It stores a native handle but
  throws until FIMS has stable access to native TMB ADREPORT internals.

`ADReportPayloadExtractor`
: Splits full Jacobians into fixed and random blocks, computes random
  covariance from the random Hessian, and assembles fixed, fixed-after-Laplace,
  or Laplace payloads.

`ADReportPayloadUncertaintyCalculator`
: Dispatches assembled payloads to the fixed-effect or Laplace uncertainty
  adapter.

## Future Native TMB Path

The intended replacement path is:

1. Implement `NativeTMBADReportDerivativeProvider::GetExtractionInput()`.
2. Populate `ADReportPayloadExtractionInput` directly from native TMB handles.
3. Keep `ADReportPayloadExtractor` and `ADReportPayloadUncertaintyCalculator`
   unchanged.
4. Remove R-side derivative assembly only after the native provider can produce
   the same payloads as the static provider.

The native provider should be considered complete only when fixed-effect,
fixed-after-Laplace, and random-effect/Laplace ADREPORT tests match TMB
`sdreport` results.
