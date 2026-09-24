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
