robustness_benchmark <- new.env(parent = asNamespace("FIMS"))
for (file in c("benchmark.R", "sensitivity.R", "information.R", "calibration.R", "robustness.R"))
  sys.source(testthat::test_path("..", "integration", "process-recovery", file),
    envir = robustness_benchmark)

test_that("Dirichlet-multinomial generator matches independent analytical moments", {
  probability <- c(.2, .3, .5, 0)
  size <- 60L
  concentration <- 15
  set.seed(512)
  draws <- replicate(5000, {
    mixed <- robustness_benchmark$robustness_probabilities(probability, concentration, runif(4))
    robustness_benchmark$robustness_counts(mixed, runif(size)) / size
  })
  inflation <- (size + concentration) / (1 + concentration)
  covariance <- (diag(probability) - tcrossprod(probability)) * inflation / size
  expect_lt(max(abs(rowMeans(draws) - probability)), .008)
  expect_lt(max(abs(cov(t(draws)) - covariance)), .0018)
  expect_true(all(draws[4, ] == 0))
  expect_equal(colSums(draws), rep(1, ncol(draws)))
  expect_equal(robustness_benchmark$robustness_probabilities(probability, Inf, rep(.5, 4)), probability)
  design <- robustness_benchmark$robustness_design()
  expect_equal(design$effective_size[1], 2000)
  expect_equal(design$effective_size[2], 1000.5)
  expect_equal(design$variance_inflation[3], 2200 / 201)
})

test_that("composition sampling handles zero tails and rejects malformed inputs", {
  expect_error(robustness_benchmark$robustness_probabilities(c(.2, .3), 2, c(.5, .5)))
  expect_error(robustness_benchmark$robustness_probabilities(c(.2, .8), 0, c(.5, .5)))
  expect_error(robustness_benchmark$robustness_counts(c(.2, .8), c(0, 1)))
  set.seed(91)
  for (i in seq_len(20)) {
    weights <- rexp(30)
    probability <- c(weights / sum(weights), 0, 0)
    counts <- robustness_benchmark$robustness_counts(probability, runif(2000))
    expect_equal(sum(counts), 2000)
    expect_equal(tail(counts, 2), c(0L, 0L))
  }
})

test_that("misspecified compositions preserve truth, scalar observations and fitted weighting", {
  on.exit(clear())
  fixture <- robustness_benchmark$recovery_fixture(4)
  set.seed(712)
  rng <- .Random.seed
  paired <- robustness_benchmark$robustness_operating_models(fixture, 91)
  expect_identical(.Random.seed, rng)
  data <- lapply(paired$models, function(x) get_data(x$data))
  scalar <- data$multinomial$type %in% c("catch", "index")
  for (name in names(paired$models)) {
    expect_identical(paired$models[[name]]$truth, paired$truth)
    expect_identical(data[[name]]$observed[scalar], data$multinomial$observed[scalar])
    expect_identical(data[[name]]$uncertainty, data$multinomial$uncertainty)
    comp <- data[[name]]$type %in% c("age_comp", "length_comp")
    groups <- interaction(data[[name]]$fleet[comp], data[[name]]$type[comp], data[[name]]$timing[comp], drop = TRUE)
    expect_true(all(abs(tapply(data[[name]]$observed[comp], groups, sum) - 1) < 1e-12))
    expect_true(all(grepl("size = 2000", data[[name]]$uncertainty[comp], fixed = TRUE)))
  }
  expect_false(identical(data$multinomial$observed, data$strong$observed))
  again <- robustness_benchmark$robustness_operating_models(fixture, 91)
  expect_identical(lapply(again$models, function(x) get_data(x$data)), data)
})

test_that("robustness summaries retain scenario-specific denominators and columns", {
  diagnostics <- data.frame(scenario = c("a", "b"), true_M_sd = .05, replicate = 1,
    sd_mode = "known", error = NA_character_, optimizer_valid = TRUE, pd_hessian = TRUE,
    boundary = FALSE, M_boundary = FALSE, regular = TRUE, sdreport_error = NA_character_)
  metrics <- data.frame(scenario = c("a", "b"), true_M_sd = .05, replicate = 1,
    sd_mode = "known", process = "M", coverage = c(.8, .4), rmse = .1, mean_width = .2)
  result <- robustness_benchmark$robustness_summaries(list(diagnostics = diagnostics,
    state_metrics = metrics, hyperparameters = data.frame()))
  expect_false(anyDuplicated(names(result$diagnostics)) > 0)
  expect_setequal(result$diagnostics$scenario, c("a", "b"))
  expect_true(all(result$fit_summary$planned_fits == 1))
  all <- result$state_summary[result$state_summary$stratum == "all", ]
  expect_equal(all$mean_coverage[all$scenario == "a"], .8)
  expect_equal(all$mean_coverage[all$scenario == "b"], .4)
})

test_that("robustness experiment fits the nominal likelihood to all scenarios", {
  skip_if(Sys.getenv("RUN_SLOW_TESTS") != "true", "Set RUN_SLOW_TESTS=true for robustness integration.")
  on.exit(clear())
  result <- robustness_benchmark$run_robustness_benchmark(1, 4, starts = .1)
  expect_equal(nrow(result$tables$diagnostics), 6)
  expect_true(all(is.na(result$tables$diagnostics$error)))
  expect_equal(nrow(result$tables$attempts), 3)
  expect_equal(nrow(result$summaries$fit_summary), 6)
  expect_setequal(result$summaries$diagnostics$scenario, c("multinomial", "moderate", "strong"))
  expect_true(all(result$summaries$fit_summary$planned_fits == 1))
})
