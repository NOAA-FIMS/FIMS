# Derived quantity uncertainty reporting

This branch adds end-to-end uncertainty reporting for derived model quantities. A caller can request a named population or fleet quantity from a `CatchAtAge` model, optionally give it a user-facing output name, and receive its estimate and standard error alongside the fitted parameters.

The branch is based on commit `4e805bf0` and contains two feature commits:

- `760ff5ed` adds derived quantity report requests, backend-neutral delta-method types, TMB integration, R output reshaping, and tests.
- `6c3b01e1` moves the ADREPORT payload calculations into FIMS backend adapters and adds support for fixed- and random-effect payloads.

## User-facing workflow

Register a report request after constructing the model and before calling `CreateTMBModel()`:

```r
caa <- methods::new(CatchAtAge)
caa$AddPopulation(population$get_id())

caa$ReportPopulationDerivedQuantity(
  population$get_id(),       # component ID
  "spawning_biomass",        # quantity stored by the model
  TRUE,                       # calculate/report its standard error
  TRUE,                       # report its point estimate
  "requested_ssb"            # name used in output
)

CreateTMBModel()
fit <- fit_fims(..., get_sd = TRUE)
```

The requested values flow through TMB `ADREPORT`, and the usual reshaped FIMS estimates contain rows such as:

```text
label          estimation_type  estimate  uncertainty
requested_ssb  derived_quantity  ...       ...
```

They can therefore be selected from a fitted `FIMSFit` object in the same way as other derived quantities:

```r
ssb <- tidy(fit, parameters = "derived_quantity") |>
  dplyr::filter(
    term == "requested_ssb"
  ) |>
  dplyr::select(term, estimate, std.error)
```

The precise accessor containing the underlying reshaped table depends on the calling workflow; `tidy()` is the high-level interface for a `FIMSFit` object.

Fleet quantities use the corresponding method:

```r
caa$ReportFleetDerivedQuantity(
  survey_fleet$get_id(),
  "index_expected",
  TRUE,
  TRUE,
  "survey_index_expected"
)
```

Use `*` in a quantity name to request every matching derived quantity. For
example, this reports `biomass`, `spawning_biomass`, `unfished_biomass`, and
`unfished_spawning_biomass` when they are available:

```r
caa$ReportPopulationDerivedQuantity(
  population$get_id(),
  "*biomass",
  TRUE,
  TRUE,
  "biomass"
)
```

Wildcard results receive distinct names by appending the matched quantity to
the requested report name, such as `biomass.spawning_biomass`. If no explicit
report name is supplied, each match receives its ordinary generated name. A
pattern that matches no available quantities produces an error. Wildcards can
appear anywhere, so `unfished_*` and `*landings*` are also valid.

The full method arguments are:

```text
ReportPopulationDerivedQuantity(component_id, quantity_name,
                                report_se, report_value, report_name)
ReportFleetDerivedQuantity(component_id, quantity_name,
                           report_se, report_value, report_name)
```

Requests can be inspected or cleared while configuring a model:

```r
caa$GetDerivedQuantityReportRequestCount()
caa$ClearDerivedQuantityReportRequests()
```

Report names must be unique within a model. An omitted/empty name is generated as `model.<model_id>.<component_type>.<component_id>.<quantity_name>`. An unknown component ID or quantity name fails when the model attempts to report it.

## What happens during fitting

When `fit_fims(..., get_sd = TRUE)` runs, FIMS now:

1. asks TMB for an `sdreport`;
2. extracts the ADREPORT estimates, Jacobians, and fixed-effect covariance;
3. adds the random-effect Hessian, covariance, and Laplace-adjusted fixed Jacobian when random effects are present;
4. passes that structured payload to the C++ uncertainty adapter; and
5. inserts the backend estimates and standard errors into the ordinary reshaped FIMS output.

Three payload paths are supported:

- `fixed`: the ordinary delta method, using the ADREPORT Jacobian and fixed-effect covariance;
- `laplace`: combines the adjusted fixed-effect contribution with the conditional random-effect contribution;
- `fixed_after_laplace`: used when a model has random effects but the reported quantity has zero derivative with respect to them.

For a quantity vector \(g\), Jacobian \(J\), and covariance \(V\), the fixed-effect calculation is the element-wise square root of the diagonal of \(J V J^T\). Small negative variances caused by floating-point error are clamped to zero; materially negative variances produce an error.

## Low-level R helpers

The branch exports three Rcpp-backed functions. Most FIMS users do not need to call these directly, because fitting and output reshaping invoke them automatically.

For a fixed-effect delta-method calculation, matrices must be flattened in row-major order:

```r
estimate <- c(total = 5)
jacobian <- matrix(c(2, 3), nrow = 1)       # dg/d(theta)
covariance <- matrix(c(0.25, 0, 0, 1), 2)  # Cov(theta)

se <- calculate_derived_quantity_se(
  estimate,
  as.numeric(t(jacobian)),
  as.numeric(t(covariance)),
  n_parameters = 2L
)
# sqrt(2^2 * 0.25 + 3^2 * 1) = sqrt(10)
```

`calculate_derived_quantity_laplace_se()` accepts flattened adjusted-fixed and random Jacobians plus their covariance matrices. `calculate_adreport_payload_se()` is the structured alternative used internally:

```r
payload <- list(
  method = "fixed",
  estimate = c(total = 5),
  jacobian = matrix(c(2, 3), nrow = 1),
  fixed_covariance = matrix(c(0.25, 0, 0, 1), 2)
)

calculate_adreport_payload_se(payload)
```

Internal R helpers build this payload directly from `TMB::MakeADFun()` and `TMB::sdreport()` objects, preserving mapped parameters and distinguishing fixed from random effects.

## C++ architecture

The implementation separates reporting policy from uncertainty backends:

- `DerivedQuantityReportRequest` describes the model/component IDs, quantity name, output name, and whether values and SEs are requested.
- `DerivedQuantityReportRegistry` owns requests, supplies stable default names, rejects empty quantity names and duplicate report names, and supports clearing/counting.
- `DerivedQuantityEstimate` is a backend-neutral result containing estimates, SEs, and dimension metadata.
- `DeltaMethodUncertaintyCalculator` provides the backend-neutral fixed-effect calculation and validates all input dimensions.
- TMB adapters translate fixed-effect and Laplace ADREPORT inputs into the common result type.
- `FisheryModelBase` stores population and fleet derived quantities and reports registered requests after calculations are complete.

`CatchAtAge` now carries requests from its Rcpp interface into the templated model, and invokes requested reporting for both simulation/value evaluation and TMB estimation.

## Tests added

The new tests cover:

- request registration, default naming, duplicate rejection, lookup, and clearing;
- fixed-effect delta-method calculations and dimension/error validation;
- TMB fixed-effect and Laplace uncertainty adapters;
- Rcpp request management for population and fleet quantities;
- standalone TMB ADREPORT payload extraction and backend agreement;
- end-to-end `CatchAtAge` spawning-biomass estimates and SEs with fixed and random effects; and
- propagation of derived-quantity estimates and uncertainties into reshaped FIMS output.
