test_that("Maturity input settings work as expected", {
  # Create maturity1
  maturity1 <- new(LogisticMaturity)

  maturity1$inflection_point[1]$value <- 10.0
  maturity1$inflection_point[1]$min <- 8.0
  maturity1$inflection_point[1]$max <- 12.0
  maturity1$inflection_point[1]$is_random_effect <- TRUE
  maturity1$inflection_point[1]$estimated <- TRUE
  maturity1$slope[1]$value <- 0.2

  expect_equal(maturity1$get_id(), 1)
  expect_equal(maturity1$inflection_point[1]$value, 10.0)
  expect_equal(maturity1$inflection_point[1]$min, 8.0)
  expect_equal(maturity1$inflection_point[1]$max, 12.0)
  expect_true(maturity1$inflection_point[1]$is_random_effect)
  expect_true(maturity1$inflection_point[1]$estimated)
  expect_equal(maturity1$slope[1]$value, 0.2)
  expect_equal(maturity1$evaluate(10.0), 0.5)


  # Create selectivity2
  maturity2 <- new(LogisticMaturity)
  expect_equal((maturity2$get_id()), 2)

  clear()
})
