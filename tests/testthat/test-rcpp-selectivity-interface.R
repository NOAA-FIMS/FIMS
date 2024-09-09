test_that("Selectivity input settings work as expected", {
  # Create selectivity1
  selectivity1 <- new(LogisticSelectivity)

  selectivity1$inflection_point[1]$value <- 10.0
  selectivity1$inflection_point[1]$min <- 8.0
  selectivity1$inflection_point[1]$max <- 12.0
  selectivity1$inflection_point[1]$is_random_effect <- TRUE
  selectivity1$inflection_point[1]$estimated <- TRUE
  selectivity1$slope[1]$value <- 0.2

  expect_equal(selectivity1$get_id(), 1)
  expect_equal(selectivity1$inflection_point[1]$value, 10.0)
  expect_equal(selectivity1$inflection_point[1]$min, 8.0)
  expect_equal(selectivity1$inflection_point[1]$max, 12.0)
  expect_true(selectivity1$inflection_point[1]$is_random_effect)
  expect_true(selectivity1$inflection_point[1]$estimated)
  expect_equal(selectivity1$slope[1]$value, 0.2)
  expect_equal(selectivity1$evaluate(10.0), 0.5)


  # Create selectivity2
  selectivity2 <- new(LogisticSelectivity)
  expect_equal((selectivity2$get_id()), 2)

  # Test double logistic
  selectivity3 <- new(DoubleLogisticSelectivity)

  selectivity3$inflection_point_asc[1]$value <- 10.5
  selectivity3$slope_asc[1]$value <- 0.2
  selectivity3$inflection_point_desc[1]$value <- 15.0
  selectivity3$slope_desc[1]$value <- 0.05

  expect_equal(selectivity3$get_id(), 3)
  expect_equal(selectivity3$inflection_point_asc[1]$value, 10.5)
  expect_equal(selectivity3$slope_asc[1]$value, 0.2)
  # R code that generates true value for the test
  # 1.0/(1.0+exp(-(34.5-10.5)*0.2)) * (1.0 - 1.0/(1.0+exp(-(34.5-15)*0.05))) = 0.2716494
  expect_equal(selectivity3$evaluate(34.5), 0.2716494, tolerance = 0.0000001)

  clear()
})
