test_that("Fleet: selectivity IDs can be added to the
fleet module", {

  # Create parameters for inflection point and slope
  inflection_param <- Parameter$new(5.0, TRUE)
  slope_param <- Parameter$new(1.0, TRUE)

  # Create selectivity for fleet 1
  selectivity_fleet1 <- new(LogisticSelectivity, inflection_param, slope_param)
  expect_equal((selectivity_fleet1$get_id()), 1)

  # Create selectivity for fleet 2
  selectivity_fleet2 <- new(LogisticSelectivity, inflection_param, slope_param)
  expect_equal((selectivity_fleet2$get_id()), 2)

  # Add selectivity to fleet
  fleet1 <- new(Fleet)
  fleet2 <- new(Fleet)

  # Expect code produces no output, error, message, or warnings
  expect_silent(fleet1$SetSelectivity(selectivity_fleet1$get_id()))
  expect_silent(fleet2$SetSelectivity(selectivity_fleet2$get_id()))

  # Expect code produces error when ID of selectivity is a character string
  expect_error(fleet1$SetSelectivity("id"))

  clear()
})


