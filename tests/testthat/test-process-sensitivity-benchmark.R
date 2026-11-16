sensitivity_benchmark <- new.env(parent = asNamespace("FIMS"))
for (file in c("benchmark.R", "sensitivity.R")) {
  sys.source(testthat::test_path("..", "integration", "process-recovery", file),
    envir = sensitivity_benchmark)
}

test_that("box stationarity distinguishes legitimate boundaries from violations", {
  lower <- log(0.001)
  project <- sensitivity_benchmark$sensitivity_projected_gradient
  expect_equal(project(c(lower, 0, -1), c(2, -3, 0.5)), c(0, 0, 0.5))
  expect_equal(project(c(lower, 0), c(-2, 3)), c(-2, 3))
  expect_equal(project(numeric(), numeric()), numeric())
})

test_that("multi-start profile summaries keep failures and use the best valid reference", {
  attempts <- data.frame(years = 6, replicate = 1, seed = 91, initialization = "fixed",
    profile_sd = c(NA, NA, 0.05, 0.05), start_sd = c(.025, .1, .025, .1),
    error = c(NA, NA, NA, "failed"), objective = c(11, 10, 9.8, 2),
    convergence = c(0, 0, 0, 1), max_projected_gradient = 0, max_inner_gradient = 0)
  summary <- sensitivity_benchmark$sensitivity_summaries(attempts)
  expect_equal(nrow(summary$attempts), 4)
  expect_equal(sum(summary$attempts$valid), 3)
  free <- summary$best[is.na(summary$best$profile_sd), ]
  profile <- summary$best[!is.na(summary$best$profile_sd), ]
  expect_equal(free$objective, 10)
  expect_equal(free$objective_spread, 1)
  expect_equal(free$delta_nll, 0.2)
  expect_equal(profile$objective, 9.8)
  expect_equal(profile$valid_starts, 1)
  expect_equal(profile$delta_nll, 0)
  attempts$convergence[] <- 1
  failed <- sensitivity_benchmark$sensitivity_summaries(attempts)$best
  expect_false(any(failed$valid))
  expect_true(all(is.na(failed$delta_nll)))
})

test_that("fixing mortality SD preserves the same integrated likelihood", {
  on.exit(clear())
  fixture <- sensitivity_benchmark$recovery_fixture(4)
  operating <- sensitivity_benchmark$recovery_operating_model(fixture, 91)
  free <- sensitivity_benchmark$sensitivity_objective(fixture, operating, "fixed", 0.1)
  p <- free$input$parameters$p
  p[free$positions] <- log(fixture$sigma)
  free_value <- as.numeric(free$obj$fn(p))
  n_random <- length(free$input$parameters$re)
  profile <- sensitivity_benchmark$sensitivity_objective(fixture, operating, "fixed", 0.1, 0.05)
  expect_length(profile$obj$par, 2)
  expect_length(profile$input$parameters$re, n_random)
  expect_equal(profile$input$parameters$p[profile$positions[["M"]]], log(0.05))
  p <- profile$input$parameters$p
  p[profile$positions] <- log(fixture$sigma)
  active <- !is.na(profile$input$map$p)
  expect_equal(as.numeric(profile$obj$fn(p[active])), free_value, tolerance = 1e-8)
})

test_that("small sensitivity experiment reoptimizes nuisance covariance parameters", {
  skip_if(Sys.getenv("RUN_SLOW_TESTS") != "true", "Set RUN_SLOW_TESTS=true for sensitivity integration.")
  on.exit(clear())
  result <- sensitivity_benchmark$run_sensitivity_benchmark(horizons = 4,
    replicates = 1, starts = 0.1, grid = c(.001, .05), profile_replicates = 1)
  attempts <- result$tables$attempts
  expect_equal(nrow(attempts), 6)
  expect_true(all(is.na(attempts$error)))
  expect_true(all(attempts$valid))
  expect_true(all(attempts$n_outer[!is.na(attempts$profile_sd)] == 2))
  expect_equal(attempts$M_sd[!is.na(attempts$profile_sd)],
    attempts$profile_sd[!is.na(attempts$profile_sd)])
  expect_true(all(result$tables$best$delta_nll >= 0))
})
