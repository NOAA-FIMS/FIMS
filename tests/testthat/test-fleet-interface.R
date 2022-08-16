test_that("Fleet: selectivity IDs can be added to the
fleet module", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Create selectivity for fleet 1
  selectivity_fleet1 <- new(fims$LogisticSelectivity)
  selectivity_fleet1$median <- 5
  selectivity_fleet1$slope <- 2
  expect_equal((selectivity_fleet1$get_id()), 1)

  # Create selectivity for fleet 2
  selectivity_fleet2 <- new(fims$LogisticSelectivity)
  selectivity_fleet2$median <- 3
  selectivity_fleet2$slope <- 1
  expect_equal((selectivity_fleet2$get_id()), 2)

  # Add selectivity to fleet
  fleet1 <- new(fims$Fleet)
  fleet2 <- new(fims$Fleet)
  
  # Expect code produces no output, error, message, or warnings
  expect_silent(fleet1$SetSelectivity(selectivity_fleet1$get_id()))
  expect_silent(fleet2$SetSelectivity(selectivity_fleet2$get_id()))

  # Expect code produces error when adding selectivity_fleet2 to fleet1
  expect_error(fleet1$SetSelectivity(selectivity_fleet2$get_id()))

  # Expect code produces error when ID of selectivity is not an integer
  expect_error(fleet$SetSelectivity(1.2))

  # Expect code produces error when ID of selectivity does not exist
  expect_error(fleet$SetSelectivity(5))

  # Expect code produces error when ID of selectivity is a character string
  expect_error(fleet$SetSelectivity("id"))
})
