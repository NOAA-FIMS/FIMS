test_that("Maturity input settings work as expected", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Create maturity1
  maturity1 <- new(fims$LogisticMaturity)

  maturity1$median$value <- 10.0
  maturity1$median$min <- 8.0
  maturity1$median$max <- 12.0
  maturity1$median$is_random_effect <- TRUE
  maturity1$median$estimated <- TRUE
  maturity1$slope$value <- 0.2

  expect_equal(maturity1$get_id(), 1)
  expect_equal(maturity1$median$value, 10.0)
  expect_equal(maturity1$median$min, 8.0)
  expect_equal(maturity1$median$max, 12.0)
  expect_true(maturity1$median$is_random_effect)
  expect_true(maturity1$median$estimated)
  expect_equal(maturity1$slope$value, 0.2)
  expect_equal(maturity1$evaluate(10.0), 0.5)


  # Create selectivity2
  maturity2 <- new(fims$LogisticMaturity)
  expect_equal((maturity2$get_id()), 2)

  fims$clear()
})
