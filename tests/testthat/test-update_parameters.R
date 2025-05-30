# Instructions ----
#' This file follows the format generated by FIMS:::use_testthat_template().
#' Necessary tests include input and output (IO) correctness [IO
#' correctness], edge-case handling [Edge handling], and built-in errors and
#' warnings [Error handling]. See `?FIMS:::use_testthat_template` for more
#' information. Every test should have a @description tag that takes up just
#' one line, which will be used in the bookdown report of {testthat} results.


# update_parameters ----
## setup ----
data <- FIMS::FIMSFrame(data1)

fleet1 <- survey1 <- list(
  selectivity = list(form = "LogisticSelectivity"),
  data_distribution = c(
    Index = "DlnormDistribution",
    AgeComp = "DmultinomDistribution"
  )
)

fleets <- list(fleet1 = fleet1, survey1 = survey1)

current_parameters <- create_default_parameters(data, fleets = fleets)

modified_parameters_valid <- list(
  survey1 = list(
    LogisticSelectivity.slope.value = 2,
    Fleet.log_q.value = -14
  )
)

## IO correctness ----

test_that("update_parameters updates parameters correctly", {
  updated_params <- update_parameters(current_parameters, modified_parameters_valid)

  #' @description Test that parameters are updated correctly
  expect_equal(updated_params$parameters$survey1$Fleet.log_q.value, -14)
  expect_equal(updated_params$parameters$survey1$LogisticSelectivity.slope.value, 2)

  #' @description Test that modules are unchanged
  expect_equal(updated_params$modules, current_parameters$modules)
})

## Edge handling ----

test_that("update_parameters handles missing parameters", {
  modified_parameters_invalid <- list(
    fleet_invalid = list(
      LogisticSelectivity.slope.value = 2,
      Fleet.log_q.value = -14
    )
  )

  #' @description Test that update_parameters(x) handles missing parameters
  expect_error(update_parameters(current_parameters, modified_parameters_invalid))
})

## Error handling ----

test_that("update_parameters detects invalid current_parameters format", {
  invalid_current_parameters <- list(
    parameters = list(module1 = list(param1 = 1)),
    extra_field = "unexpected"
  )
  #' @description update_parameters detects invalid current_parameters format (not a list)
  expect_error(
    update_parameters(invalid_current_parameters, modified_parameters_valid),
    "must be a list containing parameters and modules"
  )
  missing_names_current_parameters <- list(
    parameters = list(list(1))
  )
  #' @description update_parameters(x) detects that list is missing names
  expect_error(
    update_parameters(missing_names_current_parameters),
    "must be a list containing parameters and modules"
  )
})

test_that("update_parameters validates parameter names", {
  invalid_modified_parameters <- list(
    fleet1 = list(nonexistent_param = 10)
  )
  #' @description update_parameters validates parameter names
  expect_error(
    update_parameters(current_parameters, invalid_modified_parameters),
    "does not exist in"
  )
})

test_that("update_parameters validates parameter length", {
  invalid_modified_parameters <- list(
    fleet1 = list(LogisticSelectivity.slope.value = c(2, 3)) # Mismatched length
  )
  #' @description update_parameters validates parameter length
  expect_error(
    update_parameters(current_parameters, invalid_modified_parameters),
    "does not match between"
  )
})

test_that("update_parameters validates parameter types", {
  invalid_modified_parameters <- list(
    fleet1 = list(LogisticSelectivity.slope.value = "invalid_type")
  )
  #' @description update_parameters validates parameter types
  expect_error(
    update_parameters(current_parameters, invalid_modified_parameters),
    "does not match between"
  )
})
