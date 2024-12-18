test_that("Fleet: selectivity IDs can be added to the
fleet module", {
  # Create selectivity for fleet 1
  selectivity_fleet1 <- methods::new(LogisticSelectivity)
  expect_equal((selectivity_fleet1$get_id()), 1)

  # Create selectivity for fleet 2
  selectivity_fleet2 <- methods::new(LogisticSelectivity)
  expect_equal((selectivity_fleet2$get_id()), 2)

  # Add selectivity to fleet
  fleet1 <- methods::new(Fleet)
  fleet2 <- methods::new(Fleet)

  # Expect code produces no output, error, message, or warnings
  expect_silent(fleet1$SetSelectivity(selectivity_fleet1$get_id()))
  expect_silent(fleet2$SetSelectivity(selectivity_fleet2$get_id()))

  # Expect code produces error when ID of selectivity is a character string
  expect_error(fleet1$SetSelectivity("id"))

  clear()
})

test_that("Fleet: SetObservedAgeCompData works", {
  fleet <- methods::new(Fleet)
  expect_silent(fleet$SetObservedAgeCompData(1))
  expect_equal(fleet$GetObservedAgeCompDataID(), 1)
  clear()
})

test_that("Fleet: SetObservedIndexData works", {
  fleet <- methods::new(Fleet)
  expect_silent(fleet$SetObservedIndexData(1))
  expect_equal(fleet$GetObservedIndexDataID(), 1)
  clear()
})
