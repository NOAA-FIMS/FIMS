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

test_that("set_derived_quantity_reports registers id-based requests", {
  clear()
  caa <- methods::new(CatchAtAge)

  requests <- tibble::tribble(
    ~component_type, ~component_id, ~quantity_name, ~report_se, ~report_value, ~report_name,
    "population", 1L, "spawning_biomass", TRUE, TRUE, "ssb_pop_1",
    "fleet", 2L, "index_expected", TRUE, FALSE, "idx_fleet_2"
  )

  expect_no_error(
    set_derived_quantity_reports(caa, requests)
  )
  expect_equal(caa$GetDerivedQuantityReportRequestCount(), 2)
})

test_that("set_derived_quantity_reports validates component_type", {
  clear()
  caa <- methods::new(CatchAtAge)

  bad_requests <- tibble::tribble(
    ~component_type, ~component_id, ~quantity_name,
    "model", 1L, "spawning_biomass"
  )

  expect_error(
    set_derived_quantity_reports(caa, bad_requests),
    "component_type"
  )
})
