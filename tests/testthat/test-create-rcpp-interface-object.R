test_that("create_fims_rcpp_interface() works when evaluate_parameter is not null", {
  expect_snapshot_output(
    FIMS::create_fims_rcpp_interface(
      interface_name = "DnormDistributionsInterface",
      model = "Dnorm",
      base_class = "DistributionsInterfaceBase",
      container = "distribution_models",
      parameters = c("x", "mean", "sd"),
      evaluate_parameter = "do_log",
      evaluate_parameter_type = "bool"
    ),
    cran = FALSE,
    variant = NULL
  )
})

test_that("create_fims_rcpp_interface() works when evaluate_parameter is null", {
  expect_snapshot_output(
    FIMS::create_fims_rcpp_interface(
      interface_name = "LogisticSelectivityInterface",
      model = "LogisticSelectivity",
      base_class = "SelectivityInterfaceBase",
      container = "selectivity_models",
      parameters = c("slope", "median"),
      evaluate_parameter = NULL,
      evaluate_parameter_type = NULL
    ),
    cran = FALSE,
    variant = NULL
  )
})
