# Reporting must preserve the ordinary-report layout and ADREPORT ordering.
testthat::skip_if(
  Sys.getenv("RUN_SLOW_TESTS") != "true",
  "Skipping: RUN_SLOW_TESTS is not set to true."
)

test_that("uncertainty reports agree with ordinary reports and numerical derivatives", {
  on.exit(FIMS::clear(), add = TRUE)
  data("data_big", package = "FIMS")
  frame <- FIMS::FIMSFrame(data_big)
  parameters <- FIMS::setup_default_parameters(data = frame)
  input <- FIMS::initialize_fims(parameters, data = frame)
  map <- lapply(input$parameters, function(x) factor(rep(NA, length(x))))
  map$p <- factor(c(1L, rep(NA_integer_, length(input$parameters$p) - 1L)))
  obj <- TMB::MakeADFun(data = list(), parameters = input$parameters,
                       map = map, random = NULL, DLL = "FIMS", silent = TRUE)
  keys <- c(
    "biomass", "expected_recruitment", "mortality_F", "mortality_M",
    "mortality_Z", "numbers_at_age", "proportion_mature_at_age",
    "spawning_biomass", "sum_selectivity", "total_catch_numbers",
    "total_catch_weight", "unfished_biomass", "unfished_numbers_at_age",
    "unfished_spawning_biomass", "spawning_biomass_ratio",
    "agecomp_expected", "agecomp_proportion", "index_expected",
    "index_numbers", "index_numbers_at_age", "index_numbers_at_length",
    "index_weight", "index_weight_at_age", "catch_expected", "catch_numbers",
    "catch_numbers_at_age", "catch_numbers_at_length", "catch_weight",
    "catch_weight_at_age", "lengthcomp_expected", "lengthcomp_proportion",
    "log_index_expected", "log_catch_expected"
  )
  report <- obj$report(obj$par)
  flatten <- function(x) unlist(x[keys], use.names = FALSE)
  counts <- vapply(report[keys], function(x) length(unlist(x)), integer(1))
  sd <- TMB::sdreport(obj, getReportCovariance = FALSE)
  expect_true(sd$pdHess)
  expect_identical(names(sd$value), rep(keys, counts))
  expect_equal(as.numeric(sd$value), flatten(report), tolerance = 1e-10)
  step <- 1e-5
  derivative <- (flatten(obj$report(obj$par + step)) -
                   flatten(obj$report(obj$par - step))) / (2 * step)
  expected_se <- abs(derivative) * sqrt(sd$cov.fixed[1, 1])
  expect_equal(as.numeric(sd$sd), expected_se, tolerance = 1e-5)
})
