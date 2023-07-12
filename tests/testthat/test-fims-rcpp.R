test_that("Rcpp interface works for modules", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  expect_no_error(parameter <- new(fims$Parameter, .1))
  expect_no_error(beverton_holt <- new(fims$BevertonHoltRecruitment))
  expect_no_error(logistic_selectivity <- new(fims$LogisticSelectivity))
  expect_no_error(ewaa_growth <- new(fims$EWAAgrowth))
  logistic_selectivity$slope$value <- .7
  logistic_selectivity$median$value <- 5.0

  expect_equal(logistic_selectivity$slope$value, 0.7)
  expect_equal(logistic_selectivity$get_id(), 1)
  ewaa_growth$ages <- 1.0
  ewaa_growth$weights <- 2.5
  expect_equal(ewaa_growth$ages, 1.0)

  # check IDs for additional modules
  expect_equal(ewaa_growth$get_id(), 1)
  expect_equal(beverton_holt$get_id(), 1)

  fims$clear()
})
