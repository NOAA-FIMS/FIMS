dm_benchmark <- new.env(parent = asNamespace("FIMS"))
sys.source(testthat::test_path("..", "integration", "process-recovery", "dm-validation.R"), envir = dm_benchmark)

test_that("Monte Carlo uncertainty uses datasets and handles absent reports", {
  expect_equal(dm_benchmark$dm_mean_mcse(c(0, 1, NA)), c(n = 2, mean = .5, mcse = .5))
  expect_true(is.na(dm_benchmark$dm_mean_mcse(1)[["mcse"]]))
  expect_equal(dm_benchmark$dm_mean_mcse(numeric())[["n"]], 0)
})

test_that("DM summaries retain failures and pair only eligible datasets", {
  metrics <- expand.grid(replicate = 1:3, case = c("multinomial", "estimated_dm"), stringsAsFactors = FALSE)
  metrics$scenario <- "strong"; metrics$process <- "M"
  metrics$eligible <- c(TRUE, TRUE, FALSE, TRUE, FALSE, TRUE)
  metrics$regular <- metrics$eligible & metrics$replicate == 1
  metrics$coverage <- c(.6, .8, NA, .9, NA, .7)
  metrics$rmse <- 1 - metrics$coverage; metrics$bias <- 0; metrics$mean_width <- 1
  reports <- metrics
  reports$warm <- TRUE; reports$optimizer_valid <- reports$eligible
  reports$status <- ifelse(reports$eligible, "consistent_local_report", "rebuild_mismatch")
  reports$boundary <- !reports$regular
  points <- metrics
  points$parameter <- "concentration/fleet1/age_comp"
  points$truth <- 200; points$estimate <- 200; points$value <- log(200); points$covered <- TRUE
  points$boundary <- !points$regular
  output <- dm_benchmark$dm_validation_summaries(list(reports = reports, metrics = metrics, points = points))
  expect_true(all(output$fit_summary$planned == 3))
  expect_true(all(output$fit_summary$eligible == 2))
  row <- subset(output$state_summary, case == "estimated_dm" & stratum == "eligible" & metric == "coverage")
  expect_equal(row$n, 2); expect_equal(row$mean, .8); expect_equal(row$mcse, .1)
  paired <- subset(output$paired_summary, comparison == "estimated_dm minus multinomial" & metric == "coverage")
  expect_equal(paired$n, 1); expect_equal(paired$mean_difference, .3)
  expect_true(is.na(paired$mcse))
  expect_true(all(output$hyperparameter_summary$log_rmse == 0))
  metrics$eligible[] <- FALSE; metrics$regular[] <- FALSE
  reports$eligible[] <- FALSE; reports$regular[] <- FALSE
  points$eligible[] <- FALSE; points$regular[] <- FALSE
  empty <- dm_benchmark$dm_validation_summaries(list(reports = reports, metrics = metrics, points = points))
  expect_true(all(empty$state_summary$n == 0))
  expect_true(all(is.na(empty$state_summary$mean)))
})

test_that("failed optimizations retain every planned attempt and report", {
  e <- new.env(parent = asNamespace("FIMS"))
  sys.source(testthat::test_path("..", "integration", "process-recovery", "dm-validation.R"), e)
  e$recovery_fixture <- function(years) list(sigma = c(R = .2, F = .15, M = .05))
  e$robustness_design <- function() data.frame(scenario = "strong", concentration = 200)
  e$robustness_operating_models <- function(...) list(truth = list(), models = list(strong = list(data = data.frame())))
  e$get_data <- identity
  e$dm_composition_spec <- function(...) NULL
  e$dm_fit_snapshot <- function(...) stop("deliberate failure")
  # An all-failure run has no hyperparameter rows to summarize.
  result <- suppressWarnings(e$run_dm_validation(1, 4))
  expect_equal(nrow(result$tables$attempts), 12)
  expect_equal(nrow(result$tables$reports), 6)
  expect_true(all(result$tables$reports$status == "report_error"))
  expect_true(all(result$summaries$state_summary$n == 0))
  expect_true(all(result$summaries$fit_summary$planned == 1))
})

test_that("alternate-state estimated-DM snapshots preserve ownership and reports", {
  skip_if(Sys.getenv("RUN_SLOW_TESTS") != "true", "Set RUN_SLOW_TESTS=true for estimated-DM integration.")
  on.exit(clear())
  e <- new.env(parent = asNamespace("FIMS"))
  for (file in c("benchmark.R", "sensitivity.R", "information.R", "calibration.R", "robustness.R", "mode-handoff.R", "dm-validation.R"))
    sys.source(testthat::test_path("..", "integration", "process-recovery", file), e)
  fixture <- e$recovery_fixture(4)
  model <- e$robustness_operating_models(fixture, 91)$models$moderate
  composition <- e$dm_composition_spec(model, 2000, TRUE)
  fitted <- e$dm_fit_snapshot(fixture, model, .2, TRUE, composition)
  expect_setequal(fitted$point$parameter, c("R", "F", "M",
    "concentration/fleet1/age_comp", "concentration/fleet1/length_comp",
    "concentration/survey1/age_comp", "concentration/survey1/length_comp"))
  expect_true(all(is.finite(fitted$snapshot$p)))
  expect_equal(fitted$snapshot$composition_likelihood, composition)
  clear()
  report <- e$mode_report_snapshot(fixture, model, fitted$snapshot, TRUE)
  expect_identical(report$diagnostics$status, "consistent_local_report")
  expect_equal(dim(report$fixed_covariance), c(7L, 7L))
  expect_equal(nrow(report$states), length(fitted$snapshot$re))
})
