uncertainty_test_report <- function() {
  list(pdHess = TRUE, par.fixed = c(log_rzero = log(1000), log_weight = 0),
    cov.fixed = matrix(c(0.04, 0.015, 0.015, 0.01), 2, 2,
      dimnames = rep(list(c("log_rzero", "log_weight")), 2)))
}

uncertainty_test_inputs <- function(par) {
  list(natural_mortality = c(0.2, 0.2), weight = rep(exp(par[[2]]), 2),
    maturity = c(0, 1), proportion_female = c(1, 1),
    fleets = list(list(share = 1, selectivity = c(1, 1), weight = rep(exp(par[[2]]), 2))),
    recruitment = list(type = "beverton_holt", rzero = exp(par[[1]]),
      steepness = 0.75, phi0 = exp(par[[2]]) / expm1(0.2)))
}

test_that("uncertainty preserves correlations and known equilibrium scaling", {
  set.seed(281)
  report <- uncertainty_test_report()
  result <- calculate_reference_points_uncertainty(report, uncertainty_test_inputs,
    n_draws = 2000)
  expect_equal(unname(stats::cov(result$parameter_draws)), unname(report$cov.fixed),
    tolerance = 0.1)
  expect_equal(nrow(result$errors), 0)
  expect_true(all(result$summary$n_success == 2000))
  # Multiplicative R0 and weight changes leave SPR and optimal F invariant.
  msy <- subset(result$draws, reference_point == "MSY")
  expect_equal(msy$fishing_mortality, rep(result$estimate$msy$fishing_mortality, 2000),
    tolerance = 1e-7)
  scale <- exp(rowSums(sweep(result$parameter_draws, 2, report$par.fixed)))
  expected <- result$estimate$msy$total_yield * scale
  expect_equal(msy$total_yield, expected, tolerance = 1e-8)
  interval <- subset(result$summary, reference_point == "MSY" & quantity == "total_yield")
  expect_equal(unname(unlist(interval[c("lower", "median", "upper")])),
    unname(stats::quantile(expected, c(0.025, 0.5, 0.975))))
  set.seed(281)
  repeated <- calculate_reference_points_uncertainty(report, uncertainty_test_inputs,
    n_draws = 2000)
  expect_identical(result, repeated)
})

test_that("uncertainty retains failed draws and excludes unconverged candidates", {
  report <- uncertainty_test_report()
  reject <- function(par) {
    if (par[[1]] > report$par.fixed[[1]]) stop("unsupported biology")
    uncertainty_test_inputs(par)
  }
  set.seed(81)
  result <- calculate_reference_points_uncertainty(report, reject, n_draws = 30)
  expect_gt(nrow(result$errors), 0)
  expect_true(all(result$errors$message == "unsupported biology"))
  expect_equal(sort(c(unique(result$draws$draw), result$errors$draw)), 1:30)
  expect_true(all(result$summary$n_failed == nrow(result$errors)))
  boundary <- calculate_reference_points_uncertainty(report, uncertainty_test_inputs,
    n_draws = 3, max_f = 0.001)
  expect_true(all(boundary$summary$n_success == 0))
  expect_true(all(is.na(boundary$summary$median)))
  expect_true(all(boundary$draws$status != "converged"))
})

test_that("uncertainty rejects unusable covariance and invalid controls", {
  report <- uncertainty_test_report()
  bad <- report
  bad$cov.fixed[1, 1] <- -1
  expect_error(calculate_reference_points_uncertainty(bad, uncertainty_test_inputs),
    "positive definite")
  bad <- report
  bad$pdHess <- FALSE
  expect_error(calculate_reference_points_uncertainty(bad, uncertainty_test_inputs),
    "positive Hessian")
  bad <- report
  dimnames(bad$cov.fixed) <- rep(list(rev(names(report$par.fixed))), 2)
  expect_error(calculate_reference_points_uncertainty(bad, uncertainty_test_inputs),
    "names must match")
  expect_error(calculate_reference_points_uncertainty(report, uncertainty_test_inputs,
    n_draws = 1), "n_draws")
  expect_error(calculate_reference_points_uncertainty(report, uncertainty_test_inputs,
    level = 1), "level")
})

test_that("all rejected draws retain an empty table and missing intervals", {
  calls <- 0L
  reject_draws <- function(par) {
    calls <<- calls + 1L
    if (calls > 1L) stop("draw rejected")
    uncertainty_test_inputs(par)
  }
  result <- calculate_reference_points_uncertainty(uncertainty_test_report(),
    reject_draws, n_draws = 3)
  expect_equal(result$errors$draw, 1:3)
  expect_equal(nrow(result$draws), 0)
  expect_true(all(result$summary$n_failed == 3))
  expect_true(all(is.na(result$summary$lower)))
})
