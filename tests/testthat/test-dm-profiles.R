profile_benchmark <- new.env(parent = asNamespace("FIMS"))
for (file in c("benchmark.R", "sensitivity.R", "information.R", "calibration.R", "robustness.R",
               "mode-handoff.R", "dm-validation.R", "dm-profiles.R"))
  sys.source(testthat::test_path("..", "integration", "process-recovery", file), profile_benchmark)

test_that("a profile beating a free optimizer remains visible", {
  free <- data.frame(replicate = 1L, case = "estimated_dm", optimizer_valid = TRUE, objective = 12)
  grid <- data.frame(replicate = 1L, optimizer_valid = c(TRUE, TRUE, FALSE), objective = c(10, 11, 8))
  x <- profile_benchmark$dm_profile_summary(free, grid)
  expect_equal(x$reference_objective, rep(10, 3))
  expect_equal(x$delta_nll, c(0, 1, NA))
  expect_equal(x$free_optimization_gap, rep(2, 3))
  free$optimizer_valid <- FALSE; grid$optimizer_valid[] <- FALSE
  x <- profile_benchmark$dm_profile_summary(free, grid)
  expect_true(all(is.na(x$delta_nll)))
  expect_true(all(is.na(x$reference_objective)))
})

test_that("mapped M profiles preserve the objective and unmap nuisance SDs", {
  skip_if(Sys.getenv("RUN_SLOW_TESTS") != "true", "Set RUN_SLOW_TESTS=true for profile integration.")
  on.exit(clear())
  e <- profile_benchmark
  fixture <- e$recovery_fixture(4L)
  model <- e$robustness_operating_models(fixture, 91L)$models$moderate
  fit <- e$dm_fit_snapshot(fixture, model, .2, TRUE, e$dm_composition_spec(model, 2000, TRUE))
  refined <- e$dm_profile_fit(fixture, model, fit$snapshot, fit$point$parameter)
  sd <- refined$point$estimate[refined$point$parameter == "M"]
  mapped <- e$dm_profile_fit(fixture, model, refined$snapshot, fit$point$parameter, fixed_M_sd = sd)
  expect_equal(mapped$point$estimate[mapped$point$parameter == "M"], sd, tolerance = 1e-12)
  expect_lt(abs(mapped$diagnostics$objective - refined$diagnostics$objective), 1e-5)
  expect_true(is.null(mapped$snapshot)) # Reduced maps are not full-model persistence objects.
  expect_true(all(is.finite(mapped$p)))
  owners <- fit$point$parameter
  owners[owners == "M"] <- "temporary"; owners[owners == "R"] <- "M"; owners[owners == "temporary"] <- "R"
  expect_error(e$dm_profile_fit(fixture, model, fit$snapshot, owners), "ownership mismatch")
  expect_error(e$dm_profile_fit(fixture, model, fit$snapshot, fit$point$parameter, fixed_M_sd = 0))
  changed <- fit$snapshot; changed$data$observed[1] <- -123
  expect_error(e$dm_profile_fit(fixture, model, changed, fit$point$parameter), "input mismatch")
  report <- e$mode_report_snapshot(fixture, model, refined$snapshot, TRUE)
  expect_identical(report$diagnostics$status, "consistent_local_report")
})

test_that("all failed refinements retain planned profiles and reports", {
  e <- new.env(parent = asNamespace("FIMS"))
  sys.source(testthat::test_path("..", "integration", "process-recovery", "dm-profiles.R"), e)
  e$recovery_fixture <- function(...) list()
  e$FIMSFrame <- identity
  e$dm_profile_fit <- function(...) stop("deliberate failure")
  cases <- rbind(data.frame(replicate = 1:5, case = "estimated_dm"),
    data.frame(replicate = c(3L, 4L), case = "multinomial"))
  points <- transform(cases, scenario = "strong", parameter = "M", value = 1)
  snapshots <- stats::setNames(rep(list(list(p = 1)), nrow(cases)),
    paste(cases$replicate, "strong", cases$case, sep = "/"))
  baseline <- list(configuration = list(replicates = 5L, years = 18L),
    inputs = rep(list(list(data = list(strong = data.frame()), truth = list())), 5),
    tables = list(points = points), snapshots = snapshots)
  result <- e$run_dm_profiles(baseline, grid = c(.001, .05))
  expect_equal(nrow(result$tables$refits), 42)
  expect_equal(nrow(result$tables$profiles), 20)
  expect_equal(nrow(result$tables$reports), 7)
  expect_equal(nrow(result$tables$metrics), 21)
  expect_false(any(result$tables$reports$eligible))
  expect_true(all(is.na(result$tables$profiles$delta_nll)))
})
