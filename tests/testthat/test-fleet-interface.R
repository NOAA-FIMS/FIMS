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

  # Expect code produces error when ID of selectivity is a character string
  expect_error(fleet1$SetSelectivity("id"))

  fims$clear()
})


test_that("Fleet: SetAgeCompLikelihood works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetAgeCompLikelihood(1))

  fims$clear()
})

test_that("Fleet: SetIndexLikelihood works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetIndexLikelihood(1))

  fims$clear()
})

test_that("Fleet: SetObservedAgeCompData works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetObservedAgeCompData(1))

  fims$clear()
})

test_that("Fleet: SetObservedIndexData works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetObservedIndexData(1))

  fims$clear()
})
