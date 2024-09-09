test_that("Rcpp interface works for modules", {
  expect_no_error(parameter <- new(Parameter, .1))
  expect_no_error(beverton_holt <- new(BevertonHoltRecruitment))
  expect_no_error(logistic_selectivity <- new(LogisticSelectivity))
  expect_no_error(ewaa_growth <- new(EWAAgrowth))
  logistic_selectivity$slope[1]$value <- .7
  logistic_selectivity$inflection_point[1]$value <- 5.0

  expect_equal(logistic_selectivity$slope[1]$value, 0.7)
  expect_equal(logistic_selectivity$get_id(), 1)
  ewaa_growth$ages <- 1.0
  ewaa_growth$weights <- 2.5
  expect_equal(ewaa_growth$ages, 1.0)

  # check IDs for additional modules
  expect_equal(ewaa_growth$get_id(), 1)
  expect_equal(beverton_holt$get_id(), 1)

  clear()
})
