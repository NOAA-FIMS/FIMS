test_that("Maturity input settings work as expected", {

  # Create maturity1
  maturity1 <- new(LogisticMaturity)

  maturity1$inflection_point$value <- 10.0
  maturity1$inflection_point$min <- 8.0
  maturity1$inflection_point$max <- 12.0
  maturity1$inflection_point$is_random_effect <- TRUE
  maturity1$inflection_point$estimated <- TRUE
  maturity1$slope$value <- 0.2

  expect_equal(maturity1$get_id(), 1)
  expect_equal(maturity1$inflection_point$value, 10.0)
  expect_equal(maturity1$inflection_point$min, 8.0)
  expect_equal(maturity1$inflection_point$max, 12.0)
  expect_true(maturity1$inflection_point$is_random_effect)
  expect_true(maturity1$inflection_point$estimated)
  expect_equal(maturity1$slope$value, 0.2)
  expect_equal(maturity1$evaluate(10.0), 0.5)


  # Create selectivity2
  maturity2 <- new(LogisticMaturity)
  expect_equal((maturity2$get_id()), 2)

  clear()
})
