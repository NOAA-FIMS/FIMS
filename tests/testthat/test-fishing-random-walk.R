fishing_rw_fixture <- function(initial = "normal") {
  raw <- subset(data_big, is.na(timing) | timing <= 6 | type == "weight_at_age")
  raw$timing[raw$type == "weight_at_age"] <- NA_integer_
  data <- FIMSFrame(unique(raw))
  parameters <- setup_default_parameters(data)
  idx <- which(parameters$module_name %in% "Fleet" & parameters$fleet %in% "fleet1" &
                 parameters$label %in% "log_Fmort")
  parameters$value[idx] <- log(c(0.2, 0.22, 0.19, 0.24, 0.21, 0.2))
  parameters$estimation_type[idx] <- "random_effects"
  if (initial == "fixed") parameters$estimation_type[idx[1]] <- "constant"
  rule <- if (initial == "fixed") list(type = "fixed") else
    list(type = "normal", mean = log(0.2), sd = 0.5)
  list(parameters = parameters, data = data, rows = idx,
       specification = list(fleet1 = list(type = "rw1", sd = 0.2,
         estimation_type = "constant", initial = rule)))
}

rw_test_density <- function(values, initial = "normal", sd = 0.2,
                            mean = -1.5, initial_sd = 0.7) {
  density <- methods::new(GaussianProcessDistribution)
  density$set_random_walk(initial, mean, initial_sd)
  density$log_sd[] <- log(sd)
  density$observed_values[] <- values
  density$expected_values[] <- 0
  density
}

rw_normal_oracle <- function(values, mean, sd, initial_sd) {
  t <- seq_along(values) - 1
  covariance <- initial_sd^2 + sd^2 * outer(t, t, pmin)
  residual <- values - mean
  -length(values) / 2 * log(2 * pi) - sum(log(diag(chol(covariance)))) -
    drop(crossprod(residual, solve(covariance, residual))) / 2
}

test_that("random-walk likelihood distinguishes proper and conditioned initial states", {
  on.exit(clear())
  #' @description Compare the full likelihood with an independent dense covariance oracle and the fixed-anchor likelihood with independent increments.
  x <- c(-1.3, -1.6, -1.45, -1.7)
  density <- rw_test_density(x)
  expect_equal(density$evaluate(), rw_normal_oracle(x, -1.5, 0.2, 0.7), tolerance = 1e-10)
  density$set_random_walk("fixed", 0, 1)
  expect_equal(density$evaluate(), sum(dnorm(diff(x), 0, 0.2, log = TRUE)))
  density$observed_values[] <- 17
  expect_equal(density$evaluate(), 0)
  density$set_random_walk("normal", -1.5, 0.7)
  expect_equal(density$evaluate(), dnorm(17, -1.5, 0.7, log = TRUE))
  expect_error(density$set_random_walk("improper", 0, 1), "fixed or normal")
})

test_that("random-walk simulation has growing variance and correct continuation", {
  on.exit(clear())
  #' @description Validate marginal and increment variances, fixed anchors, and zero-innovation log-scale limits.
  density <- rw_test_density(rep(-1.5, 5))
  set.seed(311)
  draws <- replicate(3500, density$simulate())
  expect_lt(max(abs(diag(cov(t(draws))) - (0.7^2 + (0:4) * 0.2^2))), 0.04)
  expect_lt(abs(var(draws[2, ] - draws[1, ]) - 0.2^2), 0.004)
  expect_equal(density$simulate_from(0L, FALSE), rep(-1.5, 5))
  density$set_random_walk("fixed", 0, 1)
  set.seed(312)
  draws <- replicate(3500, density$simulate())
  expect_true(all(draws[1, ] == -1.5))
  expect_lt(abs(var(draws[5, ]) - 4 * 0.2^2), 0.015)
  density$observed_values[] <- c(-1.5, -1.2, 99, 99, 99)
  expect_equal(density$simulate_from(2L, FALSE), c(-1.5, -1.2, -1.2, -1.2, -1.2))
})

test_that("fleet random-walk density enters the assessment once with correct AD", {
  on.exit(clear())
  x <- fishing_rw_fixture()
  #' @description Add exactly the random-walk density without duplicating target registration, and verify innovation-SD gradient and curvature.
  old <- initialize_fims(x$parameters, x$data)
  obj <- TMB::MakeADFun(data = list(), parameters = old$parameters,
                        random = NULL, DLL = "FIMS", silent = TRUE)
  before <- obj$fn(obj$par)
  nre <- length(old$parameters$re)
  initialized <- initialize_fims(x$parameters, x$data, fishing_process = x$specification)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters,
                        random = NULL, DLL = "FIMS", silent = TRUE)
  expected <- rw_normal_oracle(x$parameters$value[x$rows], log(0.2), 0.2, 0.5)
  expect_equal(obj$fn(obj$par), before - expected, tolerance = 1e-9)
  expect_length(initialized$parameters$re, nre)
  x$specification$fleet1$estimation_type <- "fixed_effects"
  initialized <- initialize_fims(x$parameters, x$data, fishing_process = x$specification)
  names <- names(get_parameter_names(as.list(initialized$parameters$p)))
  index <- grep("gaussian_process.*log_sd", names)
  expect_length(index, 1L)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters,
                        random = NULL, DLL = "FIMS", silent = TRUE)
  plus <- minus <- obj$par
  step <- 1e-5
  plus[index] <- plus[index] + step
  minus[index] <- minus[index] - step
  expect_equal(as.numeric(obj$gr(obj$par))[index],
                (obj$fn(plus) - obj$fn(minus)) / (2 * step), tolerance = 1e-5)
  expect_equal(obj$he(obj$par)[index, index],
                (as.numeric(obj$gr(plus))[index] - as.numeric(obj$gr(minus))[index]) /
                  (2 * step), tolerance = 1e-5)
})

test_that("fishing random-walk paths drive mortality, catches, and survival", {
  on.exit(clear())
  x <- fishing_rw_fixture("fixed")
  #' @description Seeded F draws are applied before dynamics and observation predictions, and are invariant to observation-noise toggles.
  sim <- function(noise) simulate_fims(x$parameters, x$data, list(type = "iid"),
    nsim = 2, seed = 82, process_mode = "fixed", observation_error = noise,
    fishing_process = x$specification, fishing_mode = "simulate")
  result <- sim(FALSE)
  noisy <- sim(TRUE)
  expect_false(identical(result$replicates[[1]]$population,
                          result$replicates[[2]]$population))
  for (i in 1:2) {
    draw <- result$replicates[[i]]
    expect_identical(draw$fishing_mortality, noisy$replicates[[i]]$fishing_mortality)
    expect_equal(draw$fishing_mortality$log_Fmort[1], x$parameters$value[x$rows[1]])
    ages <- length(get_ages(x$data))
    n <- matrix(draw$population$numbers_at_age, ncol = ages, byrow = TRUE)
    z <- matrix(draw$population$mortality_Z, ncol = ages, byrow = TRUE)
    expect_equal(n[2, 2:(ages - 1)], n[1, 1:(ages - 2)] * exp(-z[1, 1:(ages - 2)]))
    obs <- subset(draw$observations, fleet == "fleet1" & type == "catch")
    expect_equal(obs$expected, draw$fleets$fleet1$catch_expected)
  }
})

test_that("native evaluation applies F scenarios and restores parameter state", {
  on.exit(clear())
  x <- fishing_rw_fixture()
  #' @description Increasing F increases same-year catches and lowers cohort survival; temporary paths do not change registered values.
  initialized <- initialize_fims(x$parameters, x$data, fishing_process = x$specification)
  before <- get_random()
  baseline <- initialized$model$evaluate_recruitment_path(numeric(), "log_devs")
  changed <- initialized$model$evaluate_process_paths(numeric(), "log_devs",
    list(fleet1 = x$parameters$value[x$rows] + log(2)))
  expect_gt(changed$fleets$fleet1$catch_expected[1], baseline$fleets$fleet1$catch_expected[1])
  ages <- length(get_ages(x$data))
  expect_lt(changed$population$numbers_at_age[ages + 2], baseline$population$numbers_at_age[ages + 2])
  expect_identical(get_random(), before)
  expect_identical(initialized$model$evaluate_recruitment_path(numeric(), "log_devs"), baseline)
  expect_error(initialized$model$evaluate_process_paths(numeric(), "log_devs",
    list(unknown = rep(-1, 6))), "not in the active model")
  expect_error(initialized$model$evaluate_process_paths(numeric(), "log_devs",
    list(fleet1 = c(-1, NA_real_))), "cover all")
  expect_identical(get_random(), before)
})

test_that("fixed-anchor continuation preserves historical F and zero innovations", {
  on.exit(clear())
  x <- fishing_rw_fixture("fixed")
  #' @description Conditional mean forecasts hold the last historical log-F level instead of reverting to zero or the initial mean.
  result <- forecast_fims(x$parameters, x$data, list(type = "iid"), from_year = 5,
    process_mode = "fixed", observation_error = FALSE, fishing_process = x$specification,
    fishing_mode = "mean")
  values <- result$replicates[[1]]$fishing_mortality$log_Fmort
  expect_equal(values, c(x$parameters$value[x$rows[1:4]], rep(x$parameters$value[x$rows[4]], 2)))
  expect_equal(result$replicates[[1]]$fishing_mortality$conditioned, c(TRUE, TRUE, TRUE, TRUE, FALSE, FALSE))
})

test_that("fishing configuration rejects ambiguous states before model mutation", {
  on.exit(clear())
  x <- fishing_rw_fixture()
  #' @description Enforce explicit proper initial rules, contiguous estimated states, positive SDs, and catch-fleet selection.
  initialize_fims(x$parameters, x$data)
  before <- get_random()
  bad <- x$specification
  bad$fleet1$initial <- NULL
  expect_error(initialize_fims(x$parameters, x$data, fishing_process = bad), "initial rule")
  bad <- x$specification
  bad$fleet1$sd <- 0
  expect_error(initialize_fims(x$parameters, x$data, fishing_process = bad), "positive innovation")
  bad <- x$parameters
  bad$estimation_type[x$rows[3]] <- "constant"
  expect_error(initialize_fims(bad, x$data, fishing_process = x$specification), "all log_Fmort states")
  bad <- x$specification
  names(bad) <- "survey1"
  expect_error(initialize_fims(x$parameters, x$data, fishing_process = bad), "catch fleet")
  expect_identical(get_random(), before)
})

test_that("innovation SD is recovered from simulated random-walk increments", {
  on.exit(clear())
  #' @description Repeated process fits recover a known innovation SD and agree with the independent Gaussian increment MLE.
  density <- rw_test_density(rep(-1.5, 250), "fixed", sd = 0.2)
  set.seed(613)
  paths <- replicate(25, density$simulate())
  estimates <- vapply(seq_len(ncol(paths)), function(i) {
    density$observed_values[] <- paths[, i]
    fit <- nlminb(log(0.3), function(log_sd) {
      density$log_sd[] <- log_sd
      -density$evaluate()
    })
    expect_equal(fit$convergence, 0L)
    expect_equal(exp(fit$par), sqrt(mean(diff(paths[, i])^2)), tolerance = 1e-5)
    exp(fit$par)
  }, numeric(1))
  expect_lt(abs(mean(estimates) - 0.2), 0.015)
})

test_that("latent fleet F is recovered from coherent simulated assessment data", {
  on.exit(clear())
  x <- fishing_rw_fixture()
  #' @description With known biology and precise catch data, Laplace fitting recovers the simulated latent annual F states.
  simulation <- simulate_fims(x$parameters, x$data, list(type = "iid"), seed = 201,
    process_mode = "fixed", fishing_process = x$specification, fishing_mode = "simulate")
  draw <- simulation$replicates[[1]]
  raw <- simulation$configuration$data
  idx <- raw$type %in% c("catch", "index", "age_comp", "length_comp")
  raw$observed[idx] <- draw$observations$simulated
  data <- FIMSFrame(raw)
  parameters <- x$parameters
  parameters$estimation_type[] <- "constant"
  parameters$estimation_type[x$rows] <- "random_effects"
  rec <- parameters$module_name %in% "Recruitment"
  parameters$distribution_type[rec] <- NA_character_
  parameters$distribution[rec] <- NA_character_
  initialized <- initialize_fims(parameters, data, fishing_process = x$specification)
  expect_length(initialized$parameters$re, 6L)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters,
                        random = "re", DLL = "FIMS", silent = TRUE)
  expect_true(is.finite(obj$fn(obj$par)))
  estimate <- obj$env$last.par.best
  expect_lt(max(abs(estimate - draw$fishing_mortality$log_Fmort)), 0.05)
})

test_that("fleet innovation uncertainty parameters reach fitted output", {
  on.exit(clear())
  x <- fishing_rw_fixture()
  x$specification$fleet1$estimation_type <- "fixed_effects"
  #' @description Report a single estimated innovation log-SD under the owning fleet with its initial-rule metadata.
  initialized <- initialize_fims(x$parameters, x$data, fishing_process = x$specification)
  fit <- suppressMessages(fit_fims(initialized, optimize = FALSE, get_sd = FALSE))
  estimates <- get_estimates(fit)
  rows <- estimates[estimates$label %in% "log_sd" & estimates$fleet %in% "fleet1", ]
  expect_equal(nrow(rows), 1L)
  # Existing parameter JSON output uses six significant digits.
  expect_equal(rows$input, log(0.2), tolerance = 1e-5)
})

test_that("selected fleets have separate walks and restored batch state", {
  on.exit(clear())
  x <- fishing_rw_fixture()
  raw <- get_data(x$data)
  extra <- raw[raw$fleet %in% "fleet1", ]
  extra$fleet <- "fleet2"
  data <- FIMSFrame(dplyr::bind_rows(raw, extra))
  parameters <- setup_default_parameters(data)
  selected <- parameters$module_name %in% "Fleet" & parameters$fleet %in% c("fleet1", "fleet2") &
    parameters$label %in% "log_Fmort"
  parameters$estimation_type[selected] <- "random_effects"
  parameters$value[selected] <- log(0.2)
  specs <- list(fleet1 = x$specification$fleet1, fleet2 = x$specification$fleet1)
  specs$fleet2$sd <- 0.1
  #' @description Independent selected-fleet paths retain distinct variance configuration and failed batch validation mutates neither fleet.
  result <- simulate_fims(parameters, data, list(type = "iid"), seed = 293,
    process_mode = "fixed", observation_error = FALSE,
    fishing_process = specs, fishing_mode = "simulate")
  f <- result$replicates[[1]]$fishing_mortality
  expect_setequal(unique(f$fleet), c("fleet1", "fleet2"))
  expect_false(identical(f$log_Fmort[f$fleet == "fleet1"], f$log_Fmort[f$fleet == "fleet2"]))
  expect_equal(result$configuration$fishing_process$fleet2$sd, 0.1)
  initialized <- initialize_fims(parameters, data, fishing_process = specs)
  before <- get_random()
  expect_error(initialized$model$evaluate_process_paths(numeric(), "log_devs",
    list(fleet1 = rep(-1, 6), fleet2 = rep(NA_real_, 6))), "finite")
  expect_identical(get_random(), before)
})

test_that("fixed direct recruitment remains fixed when stochastic F changes its mean", {
  on.exit(clear())
  x <- fishing_rw_fixture()
  rec <- x$parameters$label %in% "log_devs"
  x$parameters$label[rec] <- "log_r"
  x$parameters$value[rec] <- x$parameters$value[x$parameters$label %in% "log_rzero"][1]
  #' @description Holding direct log-R states fixes their values, not their old residuals, under newly drawn fishing trajectories.
  result <- forecast_fims(x$parameters, x$data, list(type = "iid"), from_year = 1,
    nsim = 2, seed = 392, process_mode = "fixed", observation_error = FALSE,
    fishing_process = x$specification, fishing_mode = "simulate")
  for (draw in result$replicates) {
    expect_identical(draw$recruitment$value, x$parameters$value[rec])
    expect_identical(draw$recruitment$log_recruitment, x$parameters$value[rec])
    expect_equal(draw$recruitment$residual,
                  draw$recruitment$value - draw$recruitment$log_mean)
  }
  expect_false(identical(result$replicates[[1]]$recruitment$log_mean,
                          result$replicates[[2]]$recruitment$log_mean))
  expect_true(result$configuration$fishing_initial_uncertainty)
})

test_that("native registration rejects an estimated fixed random-walk anchor", {
  on.exit(clear())
  clear()
  #' @description An omitted initial density cannot hide an unanchored estimated first state at the native interface.
  fleet <- methods::new(Fleet)
  fleet$n_years$set(2L)
  fleet$n_ages$set(1L)
  fleet$log_Fmort[] <- c(-1, -1.1)
  fleet$log_Fmort$set_estimation_types("random_effects")
  density <- rw_test_density(c(-1, -1.1), initial = "fixed")
  density$set_distribution_links("random_effects", fleet$log_Fmort$get_id())
  expect_error(CreateTMBModel(), "cannot be registered for estimation")
})

test_that("fleet SD groups pool likelihoods without sharing trajectories", {
  on.exit(clear())
  x <- fishing_rw_fixture()
  raw <- get_data(x$data)
  extra <- raw[raw$fleet %in% "fleet1", ]
  extra$fleet <- "fleet2"
  data <- FIMSFrame(dplyr::bind_rows(raw, extra))
  parameters <- setup_default_parameters(data)
  selected <- parameters$module_name %in% "Fleet" & parameters$fleet %in% c("fleet1", "fleet2") &
    parameters$label %in% "log_Fmort"
  parameters$estimation_type[selected] <- "random_effects"
  parameters$value[selected] <- log(0.2)
  spec <- x$specification$fleet1
  spec$estimation_type <- "fixed_effects"
  spec$sd_group <- "commercial"
  specs <- list(fleet1 = spec, fleet2 = spec)
  initialized <- initialize_fims(parameters, data, fishing_process = specs)
  mapping <- FIMS:::tmb_parameter_mapping(initialized)
  expect_length(mapping$p$reduced_names, length(initialized$parameters$p) - 1L)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters,
    map = initialized$map, random = NULL, DLL = "FIMS", silent = TRUE)
  position <- grep("gaussian_process.*log_sd", mapping$p$reduced_names)
  expect_length(position, 1L)
  plus <- minus <- obj$par
  plus[position] <- plus[position] + 1e-5
  minus[position] <- minus[position] - 1e-5
  expect_equal(as.numeric(obj$gr(obj$par))[position],
    (obj$fn(plus) - obj$fn(minus)) / 2e-5, tolerance = 1e-5)
  changed <- obj$par
  changed[position] <- log(0.3)
  obj$fn(changed)
  full <- obj$env$parList(par = obj$env$last.par)
  ids <- grep("gaussian_process.*log_sd", mapping$p$full_names)
  expect_equal(unname(full$p[ids]), rep(log(0.3), 2))
  fit <- suppressMessages(fit_fims(initialized, optimize = FALSE, get_sd = FALSE))
  estimates <- get_estimates(fit)
  rows <- estimates[estimates$label %in% "log_sd" & estimates$fleet %in% c("fleet1", "fleet2"), ]
  expect_equal(nrow(rows), 2L)
  expect_equal(rows$input, rep(log(0.2), 2), tolerance = 1e-5)
  before <- get_random()
  bad <- specs
  bad$fleet2$sd <- 0.3
  expect_error(initialize_fims(parameters, data, fishing_process = bad), "identical SDs")
  expect_identical(get_random(), before)
  bad <- specs
  bad$fleet2$estimation_type <- "constant"
  expect_error(initialize_fims(parameters, data, fishing_process = bad), "identical SDs")
  bad <- specs
  bad$fleet2$sd_group <- ""
  expect_error(initialize_fims(parameters, data, fishing_process = bad), "nonempty string")
  result <- simulate_fims(parameters, data, list(type = "iid"), seed = 121,
    process_mode = "fixed", observation_error = FALSE, fishing_process = specs, fishing_mode = "simulate")
  expect_identical(result$configuration$fishing_process$fleet2$sd_group, "commercial")
  f <- result$replicates[[1]]$fishing_mortality
  expect_false(identical(f$log_Fmort[f$fleet == "fleet1"], f$log_Fmort[f$fleet == "fleet2"]))
  specs <- lapply(specs, function(s) {s$estimation_type <- "constant"; s})
  initialized <- initialize_fims(parameters, data, fishing_process = specs)
  expect_null(initialized$map)
})
