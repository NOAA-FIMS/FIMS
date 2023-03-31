test_that("Fleet: selectivity IDs can be added to the
fleet module", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Create selectivity for fleet 1
  selectivity_fleet1 <- new(fims$LogisticSelectivity)
  expect_equal((selectivity_fleet1$get_id()), 1)

  # Create selectivity for fleet 2
  selectivity_fleet2 <- new(fims$LogisticSelectivity)
  expect_equal((selectivity_fleet2$get_id()), 2)

  # Add selectivity to fleet
  fleet1 <- new(fims$Fleet)
  fleet2 <- new(fims$Fleet)

  # Expect code produces no output, error, message, or warnings
  expect_silent(fleet1$SetSelectivity(selectivity_fleet1$get_id()))
  expect_silent(fleet2$SetSelectivity(selectivity_fleet2$get_id()))

  # Expect code produces error when adding selectivity_fleet2 to fleet1
  expect_warning(
    fleet1$SetSelectivity(selectivity_fleet2$get_id()),
    regexp = paste0("Selectivity has been set already.")
  )

  # Expect code produces error when ID of selectivity is a character string
  expect_error(fleet1$SetSelectivity("id"))

  fims$clear()
})


test_that("Fleet: SetAgeCompLikelihood works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetAgeCompLikelihood(1))

  expect_warning(
    fleet$SetAgeCompLikelihood(2),
    regexp = paste0("Age composition likelihood has been set already.")
  )

  fims$clear()
})

test_that("Fleet: SetIndexLikelihood works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetIndexLikelihood(1))

  expect_warning(
    fleet$SetIndexLikelihood(2),
    regexp = paste0("Index likelihood has been set already.")
  )

  fims$clear()
})

test_that("Fleet: SetObservedAgeCompData works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetObservedAgeCompData(1, as.matrix(sample.int(100, 12))))

  expect_warning(
    fleet$SetObservedAgeCompData(2, as.matrix(sample.int(100, 12))),
    regexp = paste0("Observed age composition data have been set already.")
  )

  fims$clear()
})

test_that("Fleet: SetObservedIndexData works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetObservedIndexData(1, rnorm(30, 200, 50)))

  expect_warning(
    fleet$SetObservedIndexData(2, rnorm(30, 200, 50)),
    regexp = paste0("Observed index data have been set already.")
  )

  fims$clear()
})
