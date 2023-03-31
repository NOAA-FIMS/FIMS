test_that("Selectivity input settings work as expected", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Create selectivity1
  selectivity1 <- new(fims$LogisticSelectivity)

  selectivity1$median$value <- 10.0
  selectivity1$median$min <- 8.0
  selectivity1$median$max <- 12.0
  selectivity1$median$is_random_effect <- TRUE
  selectivity1$median$estimated <- TRUE
  selectivity1$slope$value <- 0.2

  expect_equal(selectivity1$get_id(), 1)
  expect_equal(selectivity1$median$value, 10.0)
  expect_equal(selectivity1$median$min, 8.0)
  expect_equal(selectivity1$median$max, 12.0)
  expect_true(selectivity1$median$is_random_effect)
  expect_true(selectivity1$median$estimated)
  expect_equal(selectivity1$slope$value, 0.2)
  expect_equal(selectivity1$evaluate(10.0), 0.5)


  # Create selectivity2
  selectivity2 <- new(fims$LogisticSelectivity)
  expect_equal((selectivity2$get_id()), 2)

  # Test double logistic
  selectivity3 <- new(fims$DoubleLogisticSelectivity)

  selectivity3$median_asc$value <- 10.5
  selectivity3$slope_asc$value <- 0.2
  selectivity3$median_desc$value <- 15.0
  selectivity3$slope_desc$value <- 0.05

  expect_equal(selectivity3$get_id(), 3)
  expect_equal(selectivity3$median_asc$value, 10.5)
  expect_equal(selectivity3$slope_asc$value, 0.2)
  # R code that generates true value for the test
  # 1.0/(1.0+exp(-(34.5-10.5)*0.2)) * (1.0 - 1.0/(1.0+exp(-(34.5-15)*0.05))) = 0.2716494
  expect_equal(selectivity3$evaluate(34.5), 0.2716494, tolerance = 0.0000001)

  fims$clear()
})
