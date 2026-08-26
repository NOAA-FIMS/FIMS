# What is the feature?

* Adds configurable uncertainty reporting for derived quantities in `CatchAtAge`.
* Users can request one or more population or fleet quantities by exact name:

```r
caa$ReportPopulationDerivedQuantity(
  population$get_id(),
  c("biomass", "spawning_biomass"),
  TRUE
)
```

* Glob patterns can select related quantities:

```r
caa$ReportPopulationDerivedQuantity(
  population$get_id(),
  c("*biomass", "*at_age"),
  TRUE
)
```

* Available quantity names can be discovered with:

```r
caa$GetPopulationDerivedQuantityNames()
caa$GetFleetDerivedQuantityNames()
```

* Derived-quantity estimates and standard errors are included in the normal FIMS estimates and JSON output.

# How have you implemented the solution?

* Added a backend-neutral `DerivedQuantityReportRequest` registry for population and fleet quantities.
* Added exact-name and `*` glob matching, including support for multiple names or patterns in one request.
* Point estimates are always reported using the existing derived-quantity names; custom renaming is not supported.
* Added fixed-effect delta-method and Laplace random-effect uncertainty adapters.
* Added R helpers that extract estimates, Jacobians, covariance matrices, random-effect Hessians, and Laplace adjustments from TMB `ADREPORT`.
* Connected the backend standard errors to `fit_fims()`, `get_estimates()`, `tidy()`, and the stored JSON output.
* Added stable storage for runtime-generated TMB report names.
* Added GoogleTest coverage for backend calculations and request behavior.
* Added `testthat` coverage for:
  * exact and multiple-name requests;
  * single, multiple, and overlapping glob patterns;
  * unmatched-pattern errors;
  * population and fleet name discovery;
  * fixed- and random-effect uncertainty;
  * JSON uncertainty output; and
  * agreement between TMB and backend calculations.

# Does the PR impact any other area of the project, maybe another repo?

* No changes to another repository are required.
* The PR expands the public `CatchAtAge` Rcpp interface with derived-quantity registration and discovery methods.
* FIMS JSON derived-quantity entries now include an `uncertainty` array when standard errors are available. Downstream JSON consumers may need to account for this additional field.
* The uncertainty implementation depends on TMB runtime information, so end-to-end behavior is tested through `testthat`; backend-neutral calculations remain covered by GoogleTest.
