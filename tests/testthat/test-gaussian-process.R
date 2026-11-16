# Independent covariance calculation, rather than repeating the conditional code.
gaussian_process_oracle <- function(x, mean, sd, rho) {
  n <- length(x)
  covariance <- sd^2 / (1 - rho^2) * rho^abs(outer(seq_len(n), seq_len(n), "-"))
  residual <- x - mean
  -n / 2 * log(2 * pi) - sum(log(diag(chol(covariance)))) -
    drop(crossprod(residual, solve(covariance, residual))) / 2
}

new_test_gaussian_process <- function(x, mean, sd, rho, type = "ar1") {
  density <- methods::new(GaussianProcessDistribution)
  density$set_process(type)
  density$observed_values[] <- x
  density$expected_values[] <- mean
  density$log_sd[] <- log(sd)
  density$unconstrained_rho[] <- atanh(rho)
  density
}

test_that("Gaussian process density agrees with an independent MVN oracle", {
  on.exit(clear())
  #' @description Check initial normalizer, nonconstant means, negative correlation, and length-one series against dense covariance algebra.
  for (rho in c(-0.8, 0, 0.65, 0.98)) {
    for (n in c(1L, 6L)) {
      x <- seq(-0.4, 0.8, length.out = n)
      mean <- seq(0.1, -0.3, length.out = n)
      density <- new_test_gaussian_process(x, mean, 0.7, rho)
      expect_equal(density$evaluate(), gaussian_process_oracle(x, mean, 0.7, rho),
                   tolerance = 1e-10)
    }
  }
  density <- new_test_gaussian_process(c(1, 2, 4), 0.5, 0.8, 0, "iid")
  expect_equal(density$evaluate(), sum(dnorm(c(1, 2, 4), 0.5, 0.8, log = TRUE)))
})

test_that("Gaussian process validates dimensions and hyperparameters", {
  on.exit(clear())
  #' @description Reject invalid target and mean sizes, invalid parameter roles, and unidentified IID correlation.
  density <- new_test_gaussian_process(1:3, 0, 0.4, 0.3)
  density$expected_values[] <- c(0, 0)
  expect_error(density$evaluate(), "scalar or matching mean")
  density$expected_values[] <- 0
  density$observed_values$resize(0L)
  expect_error(density$evaluate(), "nonempty target")
  density$observed_values[] <- 1:3
  density$log_sd[] <- c(0, 0)
  expect_error(density$evaluate(), "finite scalar")
  density$log_sd[] <- 0
  density$unconstrained_rho$set_estimation_types("random_effects")
  expect_error(density$evaluate(), "constant or fixed_effects")
  density$unconstrained_rho$set_estimation_types("constant")
  density$set_process("iid")
  expect_error(density$evaluate(), "constant zero correlation")
  expect_error(density$set_process("rw"), "iid or ar1")
  expect_error(density$set_distribution_links("data", 1L), "process target ID")
})

test_that("process simulation has the stationary covariance and does not mutate targets", {
  on.exit(clear())
  #' @description Seeded process draws reproduce stationary marginal and lag-one covariance for either sign of correlation.
  for (rho in c(-0.6, 0.65)) {
    density <- new_test_gaussian_process(rep(0, 4), c(1, 2, 3, 4), 0.4, rho)
    set.seed(391)
    first <- density$simulate()
    set.seed(391)
    expect_identical(first, density$simulate())
    set.seed(392)
    draws <- replicate(4000, density$simulate())
    expected_variance <- 0.4^2 / (1 - rho^2)
    expect_equal(rowMeans(draws), c(1, 2, 3, 4), tolerance = 0.03)
    expect_lt(max(abs(diag(cov(t(draws))) - expected_variance)), 0.025)
    expect_lt(abs(cov(draws[1, ], draws[2, ]) - rho * expected_variance), 0.02)
    expect_equal(vapply(1:4, function(i) density$observed_values[i]$value,
                        numeric(1)), rep(0, 4))
  }
})

# Rebuild before each objective: FIMS uses shared model state across objectives.
make_gaussian_assessment <- function(parameters, data, process) {
  initialized <- initialize_fims(parameters, data, recruitment_process = process)
  TMB::MakeADFun(data = list(), parameters = initialized$parameters,
                random = NULL,
                DLL = "FIMS", silent = TRUE)
}

test_that("opt-in IID and zero-correlation AR1 preserve the assessment objective", {
  on.exit(clear())
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)
  #' @description Existing IID and new IID/AR1 agree on objective, AD gradient, and parameter dimensions without changing defaults.
  legacy <- make_gaussian_assessment(parameters, data, NULL)
  value <- legacy$fn(legacy$par)
  gradient <- legacy$gr(legacy$par)
  count <- length(legacy$par)
  for (type in c("iid", "ar1")) {
    obj <- make_gaussian_assessment(parameters, data,
                                    list(type = type, rho = 0, estimation_type = "constant"))
    expect_length(obj$par, count)
    expect_equal(obj$fn(obj$par), value, tolerance = 1e-9)
    expect_equal(obj$gr(obj$par), gradient, tolerance = 1e-8)
  }
})

test_that("AR1 recruitment AD gradients and reporting include covariance parameters", {
  on.exit(clear())
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)
  #' @description Estimate the transformed correlation once, verify its AD derivative, and retain it in fitted output.
  obj <- make_gaussian_assessment(parameters, data, list(type = "ar1", rho = 0.4))
  index <- grep("unconstrained_rho", names(get_parameter_names(as.list(get_fixed()))))
  expect_length(index, 1L)
  step <- 1e-5
  plus <- minus <- obj$par
  plus[index] <- plus[index] + step
  minus[index] <- minus[index] - step
  numeric_gradient <- (obj$fn(plus) - obj$fn(minus)) / (2 * step)
  expect_equal(as.numeric(obj$gr(obj$par))[index], numeric_gradient, tolerance = 1e-5)
  numeric_curvature <- (as.numeric(obj$gr(plus))[index] -
                         as.numeric(obj$gr(minus))[index]) / (2 * step)
  expect_equal(obj$he(obj$par)[index, index], numeric_curvature, tolerance = 1e-5)
  # Exercise output finalization without optimizing.
  initialized <- initialize_fims(parameters, data,
                                 recruitment_process = list(type = "ar1", rho = 0.4))
  fit <- suppressMessages(fit_fims(initialized, optimize = FALSE, get_sd = FALSE))
  estimates <- get_estimates(fit)
  expect_true("unconstrained_rho" %in% estimates$label)
})

test_that("Gaussian recruitment rejects unsupported configurations before model mutation", {
  on.exit(clear())
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)
  #' @description Reject fixed years, unsorted or missing annual states, ambiguous targets, and invalid covariance configuration.
  initialize_fims(parameters, data)
  before <- get_fixed()
  bad <- parameters
  first <- which(bad$label %in% "log_devs")[1]
  bad$estimation_type[first] <- "constant"
  expect_error(initialize_fims(bad, data, list(type = "ar1")), "fixed elements")
  bad <- parameters
  bad$timing[first] <- bad$timing[first] + 0.5
  expect_error(initialize_fims(bad, data, list(type = "ar1")), "contiguous annual")
  for (spec in list(list(type = "rw"), list(type = "ar1", rho = 1),
                   list(type = "iid", rho = 0.3),
                   list(type = "ar1", estimation_type = "random_effects"))) {
    expect_error(initialize_fims(parameters, data, spec))
  }
  expect_identical(get_fixed(), before)
})

test_that("direct log recruitment uses the linked stock-recruit mean", {
  on.exit(clear())
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)
  target <- parameters$label %in% "log_devs"
  parameters$label[target] <- "log_r"
  parameters$value[target] <- parameters$value[parameters$label %in% "log_rzero"][1] +
    seq(-0.1, 0.1, length.out = sum(target))
  #' @description Zero-correlation direct-state density agrees with the legacy linked-mean process, including derivatives.
  old <- make_gaussian_assessment(parameters, data, NULL)
  value <- old$fn(old$par)
  gradient <- old$gr(old$par)
  obj <- make_gaussian_assessment(parameters, data,
                                  list(type = "ar1", rho = 0, estimation_type = "constant"))
  expect_equal(obj$fn(obj$par), value, tolerance = 1e-9)
  expect_equal(obj$gr(obj$par), gradient, tolerance = 1e-8)
})

test_that("stationary covariance parameters can be recovered from simulated paths", {
  on.exit(clear())
  #' @description Seeded process-only fits recover innovation SD and autocorrelation over repeated annual paths.
  density <- new_test_gaussian_process(rep(0, 120), 0, 0.35, 0.6)
  set.seed(8461)
  paths <- replicate(30, density$simulate())
  estimates <- vapply(seq_len(ncol(paths)), function(i) {
    density$observed_values[] <- paths[, i]
    fit <- nlminb(c(log(0.4), atanh(0.3)), function(theta) {
      density$log_sd[] <- theta[1]
      density$unconstrained_rho[] <- theta[2]
      -density$evaluate()
    })
    expect_equal(fit$convergence, 0L)
    c(sd = exp(fit$par[1]), rho = tanh(fit$par[2]))
  }, numeric(2))
  expect_lt(abs(mean(estimates[1, ]) - 0.35), 0.03)
  expect_lt(abs(mean(estimates[2, ]) - 0.6), 0.06)
})

test_that("AR1 recruitment integrates through TMB on a small assessment", {
  on.exit(clear())
  # Retain all age bins and use one shared weight-at-age table including the
  # terminal report year, while keeping four observation years.
  input <- subset(data_big, is.na(timing) | timing <= 4 | type == "weight_at_age")
  input$timing[input$type == "weight_at_age"] <- NA_integer_
  data <- FIMSFrame(unique(input))
  parameters <- setup_default_parameters(data)
  parameters$estimation_type[!parameters$label %in% "log_devs"] <- "constant"
  #' @description Three latent recruitment states yield a finite Laplace likelihood under a nonzero AR1 process.
  initialized <- initialize_fims(parameters, data,
                                 recruitment_process = list(type = "ar1", rho = 0.3,
                                                            estimation_type = "constant"))
  expect_length(initialized$parameters$re, 3L)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters,
                        random = "re", DLL = "FIMS", silent = TRUE)
  expect_true(is.finite(obj$fn(obj$par)))
})
