#' FIMS Testing Criteria (This section should be removed once the test has been
#' created):
#' 
#' These are the three minimum testing criteria for FIMS, which should be 
#' validated for every R function:
#'  
#' 1. Input and output correctness [IO correctness]: ensure that the function 
#'    behaves as expected with correct inputs and returns the expected outputs.
#' 2. Edge case handling [Edge handling]: Validate the function's performance with 
#'    invalid inputs and unusual scenarios.
#' 3. Built-in errors and warnings [Error handling]: confirm that appropriate 
#'    error and warning messages are triggered under exceptional conditions.
#'
#' Example function (This section should be removed once the test has
#' been created):
#'
#' divide_by <- function(dividend, divisor){
#'   if (!is.numeric(dividend)) stop("dividend is not a number.")
#'   if (!is.numeric(divisor)) stop("divisor is not a number.")
#'   result <- dividend / divisor
#' }
#'
#' Note: 
#' The @description tag below can be used to generate a bookdown report of
#' {testthat} results using {testdown}. Currently, the description must be kept
#' on a single line. 

# IO correctness
test_that("divide_by() works with correct inputs", {
  #' @description Test that divide_by(10, 2.5) returns a number of 4.
  expect_equal(
    object = divide_by(10, 2.5),
    expected = 4
  )

  #' @description Test that divide_by(2, 0) returns Inf.
  expect_equal(
    object = divide_by(2, 0),
    expected = Inf
  )
})

# Edge handling
test_that("divide_by() returns correct outputs for edge cases", {
  #' @description Test that divide_by(3) returns an error.
  expect_error(
    object = divide_by(3)
  )
})

# Error handling
test_that("divide_by() returns correct error messages", {
  #' @description Test that divide_by(3, "character") returns expected error.
  expect_error(
    object = divide_by(3, "character"),
    regexp = "divisor is not a number."
  )

  #' @description Test that divide_by("character", 2.5) returns expected error.
  expect_error(
    object = divide_by("character", 2.5),
    regexp = "dividend is not a number."
  )
})