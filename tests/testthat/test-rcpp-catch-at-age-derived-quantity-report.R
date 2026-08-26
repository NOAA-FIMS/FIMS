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

  expect_setequal(
    caa$GetPopulationDerivedQuantityNames(),
    c(
      "biomass", "expected_recruitment", "mortality_F", "mortality_M",
      "mortality_Z", "numbers_at_age", "proportion_mature_at_age",
      "spawning_biomass", "sum_selectivity", "total_landings_numbers",
      "total_landings_weight", "unfished_biomass",
      "unfished_numbers_at_age", "unfished_spawning_biomass"
    )
  )
  expect_setequal(
    caa$GetFleetDerivedQuantityNames(),
    c(
      "agecomp_expected", "agecomp_proportion", "catch_index",
      "index_expected", "index_numbers", "index_numbers_at_age",
      "index_numbers_at_length", "index_weight", "index_weight_at_age",
      "landings_expected", "landings_numbers", "landings_numbers_at_age",
      "landings_numbers_at_length", "landings_weight",
      "landings_weight_at_age", "lengthcomp_expected",
      "lengthcomp_proportion", "log_index_expected",
      "log_landings_expected"
    )
  )
})
