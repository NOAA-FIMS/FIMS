test_that("direct phase simulation recovers known parameters", {
  #' @description Exact predictions recover fishing mortality and catchability from multiple starts; noisy fits retain diagnostics and uncertainty.
  helpers <- new.env(parent = globalenv())
  sys.source(system.file("examples", "recruitment-phase-recovery.R",
                         package = "FIMS"), envir = helpers)
  set.seed(17)
  before <- .Random.seed
  result <- helpers$run_recruitment_recovery(n_replicates = 2L)
  expect_identical(.Random.seed, before)
  expect_equal(nrow(result$failures), 0L)
  expect_true(all(result$draws$usable))
  expect_true(all(is.finite(result$draws$estimate)))
  expect_true(all(is.finite(result$draws$lower)))
  expect_true(all(is.finite(result$draws$upper)))
  expect_true(all(result$draws$lower > 0))
  expect_true(all(result$draws$lower < result$draws$upper))
  expect_equal(as.integer(table(result$draws$replicate, result$draws$model)),
               rep(4L, 4))
  expect_equal(anyDuplicated(result$draws[c("replicate", "model", "parameter")]), 0L)
  expect_true(all(result$timing_comparison$usable))
  expect_lt(max(abs(result$likelihood_check$delta_error)), 1e-6)
  expect_s3_class(result$session_info, "sessionInfo")
  prediction <- helpers$recovery_model(helpers$recovery_observations(),
                                       result$truth, fit = FALSE)
  for (probabilities in list(prediction$annual_age, prediction$survey_age)) {
    expect_true(all(is.finite(probabilities) & probabilities >= 0))
    expect_equal(rowSums(probabilities), rep(1, 3), tolerance = 1e-8)
  }
  expect_true(all(is.finite(prediction$catch) & prediction$catch > 0))
  expect_true(all(is.finite(prediction$index) & prediction$index > 0))
  repeat_result <- helpers$run_recruitment_recovery(n_replicates = 2L)
  expect_equal(result$draws, repeat_result$draws)
  expect_lt(max(abs(result$exact_recovery$relative_error)), 1e-3)
  expect_true(all(result$exact_recovery$convergence == 0L))
  expect_equal(nrow(result$summary), 8L)
  expect_true(all(result$summary$attempted == 2L))
  expect_equal(nrow(result$draws) / 4 + nrow(result$failures), 4L)
  expect_true(all(c("gradient", "convergence", "hessian_ok", "usable",
                    "lower", "upper") %in% names(result$draws)))
  expect_true(all(result$summary$usable <= result$summary$attempted))
})


test_that("uncertainty failures retain a reason and cannot produce usable fits", {
  #' @description Singular and failed numerical Hessians are reported without losing optimizer results.
  helpers <- new.env(parent = globalenv())
  sys.source(system.file("examples", "recruitment-phase-recovery.R",
                         package = "FIMS"), envir = helpers)
  for (fn in list(function(p) 0, function(p) stop("injected Hessian failure"))) {
    uncertainty <- helpers$recovery_uncertainty(c(0, 0), fn, NULL)
    expect_false(uncertainty$hessian_ok)
    expect_true(all(is.na(uncertainty$log_se)))
    expect_true(nzchar(uncertainty$uncertainty_message))
  }
  fit <- list(convergence = 0L, nll = 1, gradient = 0,
              hessian_ok = TRUE, estimate = rep(1, 4), log_se = rep(0.1, 4))
  expect_true(helpers$recovery_usable(fit))
  for (field in c("nll", "gradient", "estimate", "log_se")) {
    bad <- fit
    bad[[field]] <- Inf
    expect_false(helpers$recovery_usable(bad))
  }
  fit$log_se <- rep(1000, 4)
  expect_false(helpers$recovery_usable(fit))
})

test_that("malformed recovery inputs fail before touching the FIMS registry", {
  #' @description Invalid parameter order, data shapes, values, and survey dates fail in R before module construction.
  helpers <- new.env(parent = globalenv())
  sys.source(system.file("examples", "recruitment-phase-recovery.R",
                         package = "FIMS"), envir = helpers)
  testthat::local_mocked_bindings(clear = function() stop("registry touched"),
                                  .package = "FIMS")
  observations <- helpers$recovery_observations()
  start <- c(F_2027 = 0.12, F_2028 = 0.2, F_2029 = 0.28, q = 0.03)
  for (bad in list(start[-1], c(0, 1, 1, 1), c(NA, 1, 1, 1),
                   c(Inf, 1, 1, 1), rev(start))) {
    expect_error(helpers$recovery_model(observations, bad), "start must")
  }
  expect_error(helpers$recovery_model(observations, start, fit = NA),
               "TRUE or FALSE")
  bad <- observations
  bad$annual <- bad$annual[3:1, ]
  expect_error(helpers$recovery_model(bad, start), "ordered 2027")
  bad <- observations
  bad$survey$log_sd[1] <- 0
  expect_error(helpers$recovery_model(bad, start), "finite and positive")
  bad <- observations
  bad$survey$index[1] <- NA_real_
  expect_error(helpers$recovery_model(bad, start), "finite and positive")
  for (counts in list(matrix(1, 5, 3), matrix(-1, 3, 5),
                      matrix(0, 3, 5), matrix(NA_real_, 3, 5))) {
    bad <- observations
    bad$annual_age <- counts
    expect_error(helpers$recovery_model(bad, start), "3-by-5 count matrix")
  }
  for (dates in list(as.character(observations$survey$date),
                     observations$survey$date + 365,
                     observations$survey$date + 0.5,
                     as.Date(c(NA, "2028-04-01", "2029-04-01")))) {
    bad <- observations
    bad$survey$date <- dates
    expect_error(helpers$recovery_model(bad, start), "whole calendar Dates")
  }
})

test_that("invalid run settings do not change the caller's random state", {
  #' @description Replicate counts and seeds reject missing, nonintegral, vector, and out-of-range values before simulation.
  helpers <- new.env(parent = globalenv())
  sys.source(system.file("examples", "recruitment-phase-recovery.R",
                         package = "FIMS"), envir = helpers)
  set.seed(123)
  before <- .Random.seed
  for (bad in list(NULL, NA, Inf, "2", c(2, 3), 2.5, 1e20)) {
    expect_error(helpers$run_recruitment_recovery(bad), "n_replicates must")
    expect_error(helpers$run_recruitment_recovery(2, seed = bad), "seed must")
  }
  expect_error(helpers$run_recruitment_recovery(1), "n_replicates must")
  expect_error(helpers$run_recruitment_recovery(2, seed = -1), "seed must")
  expect_identical(.Random.seed, before)
})

test_that("failed noisy fits remain accounted for even with no usable fits", {
  #' @description Injected model and Hessian failures preserve diagnostics and return missing summaries instead of silently dropping replicates.
  helpers <- new.env(parent = globalenv())
  sys.source(system.file("examples", "recruitment-phase-recovery.R",
                         package = "FIMS"), envir = helpers)
  original_observations <- helpers$recovery_observations
  original_model <- helpers$recovery_model
  helpers$recovery_observations <- function(prediction = NULL, noisy = FALSE) {
    data <- original_observations(prediction, noisy)
    attr(data, "noisy_test_data") <- noisy
    data
  }
  for (all_errors in c(TRUE, FALSE)) {
    helpers$recovery_model <- function(observations, start,
                                       january_only = FALSE, fit = TRUE) {
      if (isTRUE(attr(observations, "noisy_test_data"))) {
        if (all_errors || !january_only) stop("injected model failure")
        original_uncertainty <- helpers$recovery_uncertainty
        on.exit(helpers$recovery_uncertainty <- original_uncertainty)
        helpers$recovery_uncertainty <- function(par, fn, gr) {
          list(log_se = rep(NA_real_, length(par)), hessian_ok = FALSE,
               uncertainty_message = "injected uncertainty failure")
        }
      }
      original_model(observations, start, january_only, fit)
    }
    result <- helpers$run_recruitment_recovery(2L)
    expect_equal(nrow(result$failures), if (all_errors) 4L else 2L)
    expect_true(all(result$failures$message == "injected model failure"))
    expect_true(all(result$summary$usable == 0L))
    expect_equal(result$summary$failed + result$summary$unusable +
                   result$summary$usable, result$summary$attempted)
    expect_true(all(is.na(result$summary[c("relative_bias", "relative_rmse",
                                          "coverage", "coverage_mcse")])))
    if (all_errors) {
      expect_equal(nrow(result$draws), 0L)
    } else {
      expect_equal(nrow(result$draws), 8L)
      expect_true(all(is.finite(result$draws$estimate)))
      expect_true(all(result$draws$convergence == 0L))
      expect_true(all(is.na(result$draws$lower) & is.na(result$draws$upper)))
      expect_true(all(result$draws$uncertainty_message == "injected uncertainty failure"))
      expect_false(any(result$draws$usable))
    }
    outcomes <- rbind(unique(result$draws[c("replicate", "model")]),
                      result$failures[c("replicate", "model")])
    expect_equal(nrow(outcomes), 4L)
    expect_equal(anyDuplicated(outcomes), 0L)
  }
})
