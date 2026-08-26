test_that("CatchAtAge derived quantity report requests can be managed", {
  clear()
  caa <- methods::new(CatchAtAge)

  expect_equal(caa$GetDerivedQuantityReportRequestCount(), 0)

  expect_no_error(
    caa$ReportPopulationDerivedQuantity(
      1,
      c("spawning_biomass", "biomass"),
      TRUE
    )
  )
  expect_equal(caa$GetDerivedQuantityReportRequestCount(), 2)

  expect_error(
    caa$ReportPopulationDerivedQuantity(
      1,
      "spawning_biomass",
      TRUE
    ),
    "already exists"
  )

  caa$ClearDerivedQuantityReportRequests()
  expect_equal(caa$GetDerivedQuantityReportRequestCount(), 0)
})

test_that("CatchAtAge can request fleet derived quantity reports", {
  clear()
  caa <- methods::new(CatchAtAge)

  expect_no_error(
    caa$ReportFleetDerivedQuantity(
      2,
      "index_expected",
      TRUE
    )
  )
  expect_equal(caa$GetDerivedQuantityReportRequestCount(), 1)
})

test_that("CatchAtAge lists available derived quantity names", {
  clear()
  caa <- methods::new(CatchAtAge)

  expect_contains(caa$GetPopulationDerivedQuantityNames(), "spawning_biomass")
  expect_contains(caa$GetFleetDerivedQuantityNames(), "index_expected")
})
