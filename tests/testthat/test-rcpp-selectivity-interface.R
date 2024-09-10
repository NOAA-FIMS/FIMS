test_that("LogisticSelectivityInterface is initialized correctly", {
  # Create parameters for inflection point and slope
  inflection_param <- new(Parameter, 5.0, TRUE)
  slope_param <- new(Parameter, 1.0, TRUE)
  
  # Instantiate the LogisticSelectivityInterface object
  logistic_selectivity <- new(LogisticSelectivity, inflection_param, slope_param)
  
  # Check if inflection_point and slope are set correctly
  expect_equal(logistic_selectivity$inflection_point$value, 5.0)
  expect_equal(logistic_selectivity$slope$value, 1.0)

  # Check if get_id returns a valid ID
  expect_equal(logistic_selectivity$get_id(), as.integer(1))

  # Check if the evaluate method returns the correct logistic function value
  x <- 5.0
  expected_value <- 1 / (1 + exp(-slope_param$value * (x - inflection_param$value)))
  
  expect_equal(logistic_selectivity$evaluate(x), expected_value)
  
  # Test with another value
  x <- 6.0
  expected_value <- 1 / (1 + exp(-slope_param$value * (x - inflection_param$value)))
  
  expect_equal(logistic_selectivity$evaluate(x), expected_value)

  # Set up logistic_selectivity2 and check if id is incremented correctly
  logistic_selectivity2 <- LogisticSelectivity$new(inflection_param, slope_param)
  expect_equal(logistic_selectivity2$get_id(), 2)
  clear()
})

test_that("DoubleLogisticSelectivity input settings work as expected", {
  # Test double logistic
  selectivity <- new(
    DoubleLogisticSelectivity,
    new(Parameter, 10.5, TRUE),
    new(Parameter, 0.2, TRUE),
    new(Parameter, 15.0, TRUE),
    new(Parameter, 0.05, TRUE)
  )

  expect_equal(selectivity$get_id(), 1)
  expect_equal(selectivity$inflection_point_asc$value, 10.5)
  expect_equal(selectivity$slope_asc$value, 0.2)
  # R code that generates true value for the test
  # 1.0/(1.0+exp(-(34.5-10.5)*0.2)) * (1.0 - 1.0/(1.0+exp(-(34.5-15)*0.05))) = 0.2716494
  expect_equal(selectivity$evaluate(34.5), 0.2716494, tolerance = 0.0000001)
  clear()
})

