mode_benchmark <- new.env(parent = asNamespace("FIMS"))
for (file in c("benchmark.R", "sensitivity.R", "information.R", "calibration.R", "robustness.R", "mode-handoff.R"))
  sys.source(testthat::test_path("..", "integration", "process-recovery", file), envir = mode_benchmark)

test_that("mode layout ignores transient registry IDs and keeps owner fields", {
  expect_identical(mode_benchmark$mode_parameter_layout(c("Fleet.1.log_Fmort.8", "Population.3.log_M.9")),
    c("Fleet.log_Fmort", "Population.log_M"))
  expect_error(mode_benchmark$mode_parameter_layout("malformed"))
})

test_that("saved states survive model recreation and preserve state uncertainty", {
  skip_if(Sys.getenv("RUN_SLOW_TESTS") != "true", "Set RUN_SLOW_TESTS=true for mode handoff integration.")
  on.exit(clear())
  fixture <- mode_benchmark$recovery_fixture(4)
  operating <- mode_benchmark$information_operating_models(fixture, 91, 92,
    mode_benchmark$information_design()[4, , drop = FALSE])$models$both
  fitted <- mode_benchmark$mode_fit_snapshot(fixture, operating, .2)
  path <- tempfile(fileext = ".rds")
  on.exit(unlink(path), add = TRUE)
  saveRDS(fitted$snapshot, path)
  snapshot <- readRDS(path)
  clear()
  report <- mode_benchmark$mode_report_snapshot(fixture, operating, snapshot)
  expect_identical(report$diagnostics$status, "consistent_local_report")
  expect_lt(abs(report$diagnostics$objective_difference), 1e-5)
  expect_lt(abs(report$diagnostics$after_difference), 1e-5)
  expect_equal(nrow(report$states), length(snapshot$re))
  expect_match(report$diagnostics$mode_scope, "local")
  full <- mode_benchmark$mode_report_snapshot(fixture, operating, snapshot, skip_delta_method = FALSE)
  expect_equal(report$states$se, full$states$se, tolerance = 1e-6)
  expect_equal(report$fixed_covariance, full$fixed_covariance, tolerance = 1e-6)
  raw <- get_data(operating$data)
  row <- which(raw$type %in% "catch")[1]
  raw$observed[row] <- raw$observed[row] * 1.01
  changed <- operating
  changed$data <- FIMSFrame(raw)
  expect_error(mode_benchmark$mode_report_snapshot(fixture, changed, snapshot), "does not match")
  truncated <- snapshot
  truncated$re <- head(truncated$re, -1)
  expect_error(mode_benchmark$mode_report_snapshot(fixture, operating, truncated), "lengths or values")
  malformed <- snapshot
  malformed$fixed_layout <- rev(c(snapshot$fixed_layout[-1], "wrong.parameter"))
  expect_error(mode_benchmark$mode_report_snapshot(fixture, operating, malformed), "does not match")
})
