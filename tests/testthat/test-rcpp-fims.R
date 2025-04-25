## Setup ----
# Load or prepare any necessary data for testing

## IO correctness ----
test_that("Rcpp interface works for modules", {

  #' @description Test that Rcpp interface works for parameter module
  expect_no_error(parameter <- methods::new(Parameter, .1))

  #' @description Test that Rcpp interface works for recruitment module
  expect_no_error(beverton_holt <- methods::new(BevertonHoltRecruitment))
  expect_equal(beverton_holt$get_id(), 1)
  
  #' @description Test that Rcpp interface works for selectivity module
  expect_no_error(logistic_selectivity <- methods::new(LogisticSelectivity))
  logistic_selectivity$slope[1]$value <- .7
  logistic_selectivity$inflection_point[1]$value <- 5.0
  expect_equal(logistic_selectivity$slope[1]$value, 0.7)
  expect_equal(logistic_selectivity$get_id(), 1)

  #' @description Test that Rcpp interface works for growth module
  expect_no_error(ewaa_growth <- methods::new(EWAAgrowth))
  ewaa_growth$ages$set(0, 1.0)
  ewaa_growth$weights$set(0, 2.5)
  expect_equal(ewaa_growth$ages$get(0), 1.0)
  expect_equal(ewaa_growth$get_id(), 1)

  clear()
})
## Edge handling ----

## Error handling ----