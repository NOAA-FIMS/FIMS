data("data_mile1")
data <- FIMSFrame(data_mile1)

fleet1 <- survey1 <- list(
  selectivity = list(form = "LogisticSelectivity"),
  data_distribution = c(
    Index = "DlnormDistribution",
    AgeComp = "DmultinomDistribution"
  )
)

fleets = list(fleet1 = fleet1, survey1 = survey1)

current_parameters <- create_default_parameters(data, fleets = fleets)

modified_parameters_valid = list(
  survey1 = list(
    LogisticSelectivity.slope.value = 2,
    Fleet.log_q.value = -14
  )
)

test_that("update_parameters updates parameters correctly", {

  updated_params <- update_parameters(current_parameters, modified_parameters_valid)

  # Check that parameters are updated correctly
  expect_equal(updated_params$parameters$survey1$Fleet.log_q.value,  -14)
  expect_equal(updated_params$parameters$survey1$LogisticSelectivity.slope.value, 2)

  # Check that modules are unchanged
  expect_equal(updated_params$modules, current_parameters$modules)
})

test_that("update_parameters handles missing parameters", {
  modified_parameters_invalid <- list(
    fleet_invalid = list(
      LogisticSelectivity.slope.value = 2,
      Fleet.log_q.value = -14
    )
  )

  expect_error(update_parameters(current_parameters, modified_parameters_invalid))
})

test_that("update_parameters detects invalid current_parameters format", {
  invalid_current_parameters <- list(
    parameters = list(module1 = list(param1 = 1)),
    extra_field = "unexpected"
  )

  expect_error(
    update_parameters(invalid_current_parameters, modified_parameters_valid),
    "must be a list containing parameters and modules"
  )
})

test_that("update_parameters validates parameter names", {
  invalid_modified_parameters <- list(
    fleet1 = list(nonexistent_param = 10)
  )

  expect_error(
    update_parameters(current_parameters, invalid_modified_parameters),
    "does not exist in"
  )
})

test_that("update_parameters validates parameter length", {
  invalid_modified_parameters <- list(
    fleet1 = list(LogisticSelectivity.slope.value = c(2, 3))  # Mismatched length
  )

  expect_error(
    update_parameters(current_parameters, invalid_modified_parameters),
    "does not match between"
  )
})

test_that("update_parameters validates parameter types", {
  invalid_modified_parameters <- list(
    fleet1 = list(LogisticSelectivity.slope.value = "invalide_type")
  )

  expect_error(
    update_parameters(current_parameters, invalid_modified_parameters),
    "does not match between"
  )
})

