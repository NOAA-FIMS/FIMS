calibration_benchmark <- new.env(parent = asNamespace("FIMS"))
for (file in c("benchmark.R", "sensitivity.R", "information.R", "calibration.R"))
  sys.source(testthat::test_path("..", "integration", "process-recovery", file),
    envir = calibration_benchmark)

test_that("calibration intervals score coverage, width and standardized error", {
  metrics <- calibration_benchmark$calibration_intervals(c(0, 0), c(0, 2), c(1, 1))
  expect_equal(metrics$coverage, .5)
  expect_equal(metrics$mean_width, 2 * qnorm(.975))
  expect_equal(metrics$mean_squared_standardized_error, 2)
  absent <- calibration_benchmark$calibration_intervals(0, 0, NA_real_)
  expect_equal(absent$n_intervals, 0)
  expect_true(is.na(absent$coverage))
  expect_true(is.na(absent$mean_width))
})

test_that("coverage summaries use replicate fractions and preserve nonregular fits", {
  diagnostics <- data.frame(true_M_sd = .05, replicate = 1:3, sd_mode = "estimated",
    error = c(NA, NA, "failed"), optimizer_valid = c(TRUE, TRUE, FALSE),
    pd_hessian = c(TRUE, TRUE, NA), boundary = c(FALSE, TRUE, NA),
    M_boundary = c(FALSE, TRUE, NA), regular = c(TRUE, FALSE, FALSE), sdreport_error = NA_character_)
  metrics <- data.frame(true_M_sd = .05, replicate = 1:2, sd_mode = "estimated",
    process = "M", coverage = c(0, 1), rmse = c(.1, .2), mean_width = c(.3, .4))
  hyper <- data.frame(true_M_sd = .05, replicate = 1:2, sd_mode = "estimated",
    process = "M", covered = TRUE, estimate = .05, truth = .05)
  result <- calibration_benchmark$calibration_summaries(list(diagnostics = diagnostics,
    state_metrics = metrics, hyperparameters = hyper))
  expect_equal(result$fit_summary$planned_fits, 3)
  expect_equal(result$fit_summary$errors, 1)
  expect_equal(result$fit_summary$regular_fits, 1)
  all <- result$state_summary[result$state_summary$stratum == "all", ]
  expect_equal(all$mean_coverage, .5)
  expect_equal(all$coverage_mcse, .5)
  expect_equal(all$interval_fits, 2)
  regular <- result$state_summary[result$state_summary$stratum == "regular", ]
  expect_equal(regular$mean_coverage, 0)
  expect_true(is.na(regular$coverage_mcse))
  all <- result$hyperparameter_summary[result$hyperparameter_summary$stratum == "all", ]
  expect_equal(all$coverage, 1)
  expect_equal(all$interval_fits, 2)
  expect_lt(all$coverage_lower, 1)
  expect_gt(all$coverage_lower, 0)
  failed <- calibration_benchmark$calibration_summaries(list(diagnostics = diagnostics[3, ],
    state_metrics = data.frame(), hyperparameters = data.frame()))
  expect_equal(failed$fit_summary$errors, 1)
  expect_equal(nrow(failed$state_summary), 0)
})

test_that("true mortality SD variation keeps process innovations paired", {
  on.exit(clear())
  fixture <- calibration_benchmark$recovery_fixture(4)
  design <- calibration_benchmark$information_design()[4, , drop = FALSE]
  fixture$sigma[["M"]] <- .025
  low <- calibration_benchmark$information_operating_models(fixture, 91, 92, design)
  fixture$sigma[["M"]] <- .1
  high <- calibration_benchmark$information_operating_models(fixture, 91, 92, design)
  expect_identical(low$truth$R, high$truth$R)
  expect_identical(low$truth$F, high$truth$F)
  ages <- get_n_ages(fixture$data)
  low_m <- matrix(low$truth$M, nrow = ages)
  high_m <- matrix(high$truth$M, nrow = ages)
  expect_identical(low_m[, 1], high_m[, 1])
  expect_equal((low_m[, -1] - low_m[, -ncol(low_m)]) / .025,
    (high_m[, -1] - high_m[, -ncol(high_m)]) / .1, tolerance = 1e-12)
})

test_that("calibration rebuilds selected fits and reports known/estimated uncertainty", {
  skip_if(Sys.getenv("RUN_SLOW_TESTS") != "true", "Set RUN_SLOW_TESTS=true for calibration integration.")
  on.exit(clear())
  result <- calibration_benchmark$run_calibration_benchmark(replicates = 1, years = 4,
    mortality_sds = .05, starts = .1)
  expect_equal(nrow(result$tables$diagnostics), 2)
  expect_true(all(is.na(result$tables$diagnostics$error)))
  expect_equal(result$tables$diagnostics$n_outer, c(0, 3))
  expect_true(all(abs(result$tables$diagnostics$objective_rebuild_difference) < 1e-5))
  expect_equal(nrow(result$tables$state_metrics), 6)
  m <- result$tables$state_metrics[result$tables$state_metrics$process == "M", ]
  expect_equal(m$n_intervals, c(48, 48))
  expect_true(all(is.na(result$summaries$state_summary$coverage_mcse)))
})
