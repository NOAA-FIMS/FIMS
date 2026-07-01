test_that("CatchAtAge derived quantity report requests can be managed", {
  clear()
  caa <- methods::new(CatchAtAge)

  expect_equal(caa$GetDerivedQuantityReportRequestCount(), 0)

  expect_no_error(
    caa$ReportPopulationDerivedQuantity(
      1,
      "spawning_biomass",
      TRUE,
      TRUE,
      "ssb"
    )
  )
  expect_equal(caa$GetDerivedQuantityReportRequestCount(), 1)

  expect_error(
    caa$ReportPopulationDerivedQuantity(
      1,
      "spawning_biomass",
      TRUE,
      TRUE,
      "ssb"
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
      TRUE,
      FALSE,
      "survey_index_expected"
    )
  )
  expect_equal(caa$GetDerivedQuantityReportRequestCount(), 1)
})
