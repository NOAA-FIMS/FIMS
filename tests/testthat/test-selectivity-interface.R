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
})
