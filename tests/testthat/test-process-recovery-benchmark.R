recovery_benchmark <- new.env(parent = asNamespace("FIMS"))
sys.source(testthat::test_path("..", "integration", "process-recovery", "benchmark.R"),
           envir = recovery_benchmark)

test_that("recovery metrics use finite estimates and explicit interval denominators", {
  metrics <- recovery_benchmark$recovery_metrics(c(0, 1, 2), c(1, 1, NA), c(1, 0, NA))
  expect_equal(metrics$n, 3)
  expect_equal(metrics$n_estimates, 2)
  expect_equal(metrics$n_intervals, 1)
  expect_equal(metrics$bias, 0.5)
  expect_equal(metrics$rmse, sqrt(0.5))
  expect_equal(metrics$coverage, 1)
  absent <- recovery_benchmark$recovery_metrics(1, NA_real_, NA_real_)
  expect_true(is.na(absent$coverage))
  expect_equal(absent$n_intervals, 0)
})

test_that("recovery design excludes meaningless IID initialization duplicates", {
  design <- recovery_benchmark$recovery_design()
  expect_equal(nrow(design), 14)
  expect_false(any(design$processes == "R" & design$initialization == "normal"))
  expect_equal(sum(design$processes == "R_F_M"), 4)
})

test_that("benchmark initialization separates oracle truth from estimated starts", {
  on.exit(clear())
  fixture <- recovery_benchmark$recovery_fixture(4)
  operating <- recovery_benchmark$recovery_operating_model(fixture, 91)
  design <- data.frame(processes = "R_F_M", initialization = "fixed", sd_mode = "known")
  setup <- recovery_benchmark$recovery_fit_input(fixture, operating, design)
  for (key in c("F", "M")) {
    rows <- fixture$rows[[key]]
    first <- fixture$parameters$timing[rows] == get_start_year(fixture$data)
    expect_equal(setup$parameters$value[rows[first]], operating$truth[[key]][first])
    expect_true(all(setup$parameters$estimation_type[rows[first]] == "constant"))
    expect_equal(setup$parameters$value[rows[!first]], rep(log(0.18), sum(!first)))
  }
  design$initialization <- "normal"
  setup <- recovery_benchmark$recovery_fit_input(fixture, operating, design)
  expect_true(all(setup$parameters$estimation_type[fixture$rows$M] == "random_effects"))
  design$processes <- "F"
  setup <- recovery_benchmark$recovery_fit_input(fixture, operating, design)
  expect_equal(setup$parameters$value[fixture$rows$M], operating$truth$M)
  expect_true(all(setup$parameters$estimation_type[fixture$rows$M] == "constant"))
})

test_that("benchmark summaries retain failures and flag boundary fits", {
  metadata <- data.frame(replicate = 1:3, processes = "M", initialization = "fixed", sd_mode = "estimated")
  diagnostics <- cbind(metadata, error = c(NA, NA, "failed"), convergence = c(0, 0, NA),
    max_outer_gradient = c(0, 0, NA), max_inner_gradient = c(0, 0, NA),
    pd_hessian = c(TRUE, TRUE, NA), sd_boundary = c(FALSE, TRUE, NA), elapsed_seconds = 1)
  result <- list(tables = list(diagnostics = diagnostics, state_metrics = data.frame(), hyperparameters = data.frame()))
  summary <- recovery_benchmark$recovery_summaries(result)
  expect_equal(summary$fit_summary$planned_fits, 3)
  expect_equal(summary$fit_summary$errors, 1)
  expect_equal(summary$fit_summary$qualified_fits, 1)
  expect_equal(summary$fit_summary$boundary_fits, 1)
})

test_that("small recovery benchmark executes estimation and uncertainty", {
  skip_if(Sys.getenv("RUN_SLOW_TESTS") != "true", "Set RUN_SLOW_TESTS=true for the recovery integration check.")
  on.exit(clear())
  design <- data.frame(processes = "F", initialization = "fixed", sd_mode = "known")
  result <- recovery_benchmark$run_recovery_benchmark(1, 4, 202, design)
  diagnostics <- recovery_benchmark$recovery_summaries(result)$diagnostics
  expect_true(is.na(diagnostics$error))
  expect_equal(diagnostics$n_random, 3)
  expect_true(diagnostics$qualified)
  expect_equal(result$tables$state_metrics$n_intervals, 3)
})
