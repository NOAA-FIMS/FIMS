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

test_that("shared-scale uncertainty cancels common SB and SB_MSY variation in annual ratios", {
  #' @description Multiplicative R0 and weight variation changes SB and SB_MSY together while leaving SB/SB_MSY constant.
  report <- uncertainty_test_report()
  shared_scale_fn <- function(par) {
    inputs <- uncertainty_test_inputs(par)
    scale <- exp((par[["log_rzero"]] - log(1000)) + par[["log_weight"]])
    inputs$population_id <- 1L
    inputs$year_index <- 2L
    inputs$historical <- data.frame(
      population_id = rep(1L, 3L),
      year_index = 1:3,
      calendar_year = 2020:2022,
      spawning_biomass = c(2400, 1800, 1500) * scale,
      f_multiplier = c(1, 1, NA_real_),
      fleet_fishing_mortality = c(0, 0.3, NA_real_),
      effective_fishing_mortality = c(0, 0.3, NA_real_)
    )
    inputs
  }
  set.seed(404)
  res <- calculate_stock_status_uncertainty(report, shared_scale_fn, n_draws = 500)
  expect_equal(nrow(res$errors), 0L)
  expect_equal(res$conditioning, "conditional_on_random_effects")
  expect_true(all(res$draw_diagnostics$sb_usable))
  expect_true(all(res$draw_diagnostics$f_usable))

  # Numerator and denominator both vary substantially across draws, but their ratio is invariant.
  yr1_draws <- subset(res$draws, year_index == 1L)
  expect_gt(stats::sd(yr1_draws$spawning_biomass), 100)
  expect_gt(stats::sd(yr1_draws$sb_msy), 100)
  for (yr in 1:3) {
    sub_d <- subset(res$draws, year_index == yr)
    est_ratio <- res$estimate$stock_status$sb_over_sb_msy[yr]
    expect_equal(sub_d$sb_over_sb_msy, rep(est_ratio, 500), tolerance = 1e-7)
    expect_equal(res$summary$sb_over_sb_msy_lower[yr], est_ratio, tolerance = 1e-7)
    expect_equal(res$summary$sb_over_sb_msy_median[yr], est_ratio, tolerance = 1e-7)
    expect_equal(res$summary$sb_over_sb_msy_upper[yr], est_ratio, tolerance = 1e-7)
    expect_lt(res$summary$sb_over_sb_msy_sd[yr], 1e-7)
    expect_equal(res$summary$sb_over_sb_msy_n_usable[yr], 500L)
    expect_equal(res$summary$sb_over_sb_msy_n_excluded[yr], 0L)
  }
  # Year 1 has zero fishing mortality; its zero ratio is preserved. Year 3 is terminal biomass only.
  expect_equal(res$summary$f_over_f_msy[1], 0)
  expect_equal(res$summary$f_over_f_msy_lower[1], 0)
  expect_equal(res$summary$f_over_f_msy_upper[1], 0)
  expect_equal(res$summary$f_over_f_msy_sd[1], 0)
  expect_equal(res$summary$f_over_f_msy_n_usable[1], 500L)
  expect_true(is.na(res$summary$f_over_f_msy[3]))
  expect_true(is.na(res$summary$f_over_f_msy_lower[3]))
  expect_true(is.na(res$summary$f_over_f_msy_sd[3]))
  expect_equal(res$summary$f_over_f_msy_n_usable[3], 0L)
  expect_equal(res$summary$f_over_f_msy_n_excluded[3], 500L)
})

test_that("varying annual stock-status ratios match manual per-draw ratios and empirical quantiles", {
  #' @description Draw-paired ratios, quantiles, and SDs match manual calculations and reproduce under set.seed.
  report <- list(
    pdHess = TRUE,
    par.fixed = c(log_rzero = log(1000), logit_steep = log((0.75 - 0.2) / (1 - 0.75)), log_f = log(0.3)),
    cov.fixed = matrix(
      c(0.03, 0.008, -0.006,
        0.008, 0.02, -0.004,
        -0.006, -0.004, 0.015),
      nrow = 3, ncol = 3,
      dimnames = rep(list(c("log_rzero", "logit_steep", "log_f")), 2)
    )
  )
  varying_fn <- function(par) {
    rzero <- exp(par[["log_rzero"]])
    steepness <- 0.2 + 0.8 * stats::plogis(par[["logit_steep"]])
    f_base <- exp(par[["log_f"]])
    f_eff <- c(0.7, 1.2) * f_base
    sb_hist <- c(
      2200 * (rzero / 1000)^0.7 * exp(-0.4 * f_eff[1]),
      1700 * (rzero / 1000)^0.85 * exp(-0.6 * f_eff[2]),
      1450 * (rzero / 1000)^0.9 * exp(-0.8 * f_eff[2])
    )
    list(
      natural_mortality = c(0.2, 0.2), weight = c(1, 1),
      maturity = c(0, 1), proportion_female = c(1, 1),
      fleets = list(list(share = 1, selectivity = c(1, 1), weight = c(1, 1))),
      recruitment = list(
        type = "beverton_holt", rzero = rzero,
        steepness = steepness, phi0 = 1 / expm1(0.2)
      ),
      population_id = 1L,
      year_index = 2L,
      historical = data.frame(
        population_id = rep(1L, 3L),
        year_index = 1:3,
        calendar_year = 2021:2023,
        spawning_biomass = sb_hist,
        f_multiplier = c(1, 1, NA_real_),
        fleet_fishing_mortality = c(f_eff, NA_real_),
        effective_fishing_mortality = c(f_eff, NA_real_)
      )
    )
  }

  set.seed(902)
  res <- calculate_stock_status_uncertainty(report, varying_fn, n_draws = 400, level = 0.90)
  set.seed(902)
  repeated <- calculate_stock_status_uncertainty(report, varying_fn, n_draws = 400, level = 0.90)
  expect_identical(res, repeated)

  # Manually compute reference points and ratios for every parameter draw.
  manual_sb_ratio <- matrix(NA_real_, nrow = 400, ncol = 3)
  manual_f_ratio <- matrix(NA_real_, nrow = 400, ncol = 3)
  for (i in seq_len(400)) {
    par_i <- stats::setNames(res$parameter_draws[i, ], names(report$par.fixed))
    inp_i <- varying_fn(par_i)
    rp_i <- calculate_reference_points(inp_i)
    manual_sb_ratio[i, ] <- inp_i$historical$spawning_biomass / rp_i$msy$spawning_biomass
    manual_f_ratio[i, ] <- inp_i$historical$effective_fishing_mortality / rp_i$msy$fishing_mortality
  }

  probs <- c(0.05, 0.5, 0.95)
  for (yr in 1:3) {
    sub_d <- subset(res$draws, year_index == yr)
    expect_equal(sub_d$sb_over_sb_msy, manual_sb_ratio[, yr], tolerance = 1e-12)
    expect_gt(stats::sd(manual_sb_ratio[, yr]), 0.01)
    expect_equal(
      unname(unlist(res$summary[yr, c("sb_over_sb_msy_lower", "sb_over_sb_msy_median", "sb_over_sb_msy_upper")])),
      unname(stats::quantile(manual_sb_ratio[, yr], probs)),
      tolerance = 1e-12
    )
    expect_equal(res$summary$sb_over_sb_msy_sd[yr], stats::sd(manual_sb_ratio[, yr]), tolerance = 1e-12)
    if (yr <= 2L) {
      expect_equal(sub_d$f_over_f_msy, manual_f_ratio[, yr], tolerance = 1e-12)
      expect_gt(stats::sd(manual_f_ratio[, yr]), 0.01)
      expect_equal(
        unname(unlist(res$summary[yr, c("f_over_f_msy_lower", "f_over_f_msy_median", "f_over_f_msy_upper")])),
        unname(stats::quantile(manual_f_ratio[, yr], probs)),
        tolerance = 1e-12
      )
      expect_equal(res$summary$f_over_f_msy_sd[yr], stats::sd(manual_f_ratio[, yr]), tolerance = 1e-12)
    } else {
      expect_true(all(is.na(sub_d$f_over_f_msy)))
      expect_true(is.na(res$summary$f_over_f_msy_lower[yr]))
      expect_true(is.na(res$summary$f_over_f_msy_sd[yr]))
    }
  }
})

test_that("stock status uncertainty handles failed draws, upper-bound candidates, and all-invalid draws", {
  #' @description Failed draws and upper-bound MSY candidates are excluded with explicit diagnostics.
  report <- uncertainty_test_report()
  call_idx <- 0L
  mixed_fn <- function(par) {
    call_idx <<- call_idx + 1L
    if (call_idx > 1L && call_idx %% 3L == 0L) stop("invalid draw biology")
    inputs <- uncertainty_test_inputs(par)
    inputs$historical <- data.frame(
      population_id = c(1L, 1L),
      year_index = 1:2,
      spawning_biomass = c(2000, 1500),
      effective_fishing_mortality = c(0.2, 0.3)
    )
    inputs
  }
  set.seed(55)
  res <- calculate_stock_status_uncertainty(report, mixed_fn, n_draws = 12)
  expect_gt(nrow(res$errors), 0L)
  expect_equal(nrow(res$draw_diagnostics), 12L)
  n_err <- nrow(res$errors)
  expect_true(all(res$summary$sb_over_sb_msy_n_usable == 12L - n_err))
  expect_true(all(res$summary$sb_over_sb_msy_n_excluded == n_err))
  expect_equal(
    res$draw_diagnostics$status[!res$draw_diagnostics$evaluated],
    rep("error", n_err)
  )

  # Upper-bound MSY candidates yield missing ratios and missing interval bounds.
  call_idx <- 0L
  upper_res <- calculate_stock_status_uncertainty(
    report, mixed_fn, n_draws = 6, max_f = 0.001
  )
  expect_true(all(upper_res$summary$sb_over_sb_msy_n_usable == 0L))
  expect_true(all(upper_res$summary$sb_over_sb_msy_n_excluded == 6L))
  expect_true(all(is.na(upper_res$summary$sb_over_sb_msy_lower)))
  expect_true(all(is.na(upper_res$summary$sb_over_sb_msy_sd)))
  expect_true(all(is.na(upper_res$draws$sb_over_sb_msy)))
  expect_true(all(
    upper_res$draw_diagnostics$sb_reason[upper_res$draw_diagnostics$evaluated] == "upper_bound"
  ))
})

test_that("live fitted-model route evaluates draws, restores state, and supports joint random-effect draws", {
  #' @description Live CatchAtAge evaluator restores fixed/random state on success or error and supports conditional and joint RE draws.
  clear()
  on.exit(clear(), add = TRUE)

  growth <- methods::new(EWAAGrowth)
  growth$n_years$set(2L)
  growth$ages[] <- 1:3
  growth$weights[] <- c(1, 2, 3, 2, 3, 4, 2, 3, 4)
  maturity <- methods::new(LogisticMaturity)
  maturity$inflection_point[] <- 2
  maturity$slope[] <- 1
  recruitment <- methods::new(BevertonHoltRecruitment)
  process <- methods::new(LogDevsRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(process$get_id())
  recruitment$n_years$set(2L)
  recruitment$log_rzero[] <- log(1000)
  recruitment$log_rzero$set_estimation_types("fixed_effects")
  recruitment$logit_steep[] <- log((0.75 - 0.2) / (1 - 0.75))
  recruitment$log_devs[] <- c(0.1)
  recruitment$log_devs$set_estimation_types("random_effects")

  selectivity <- methods::new(LogisticSelectivity)
  selectivity$inflection_point[] <- 2
  selectivity$slope[] <- 1

  population <- methods::new(Population)
  population$n_years$set(2L)
  population$n_ages$set(3L)
  population$n_fleets$set(2L)
  population$ages[] <- 1:3
  population$log_M[] <- log(c(0.2, 0.3, 0.4, 0.2, 0.3, 0.4))
  population$log_f_multiplier[] <- log(c(1.2, 1.5))
  population$log_init_naa[] <- log(c(1000, 500, 400))
  population$proportion_female[] <- 0.5
  population$SetGrowthID(growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$SetRecruitmentID(recruitment$get_id())

  fleet1 <- methods::new(Fleet)
  fleet1$n_years$set(2L)
  fleet1$n_ages$set(3L)
  fleet1$n_lengths$set(0L)
  fleet1$log_Fmort[] <- log(c(0.1, 0.15))
  fleet1$log_Fmort$set_estimation_types("fixed_effects")
  fleet1$log_q[] <- 0
  fleet1$SetSelectivityID(selectivity$get_id())
  population$AddFleet(fleet1$get_id())

  fleet2 <- methods::new(Fleet)
  fleet2$n_years$set(2L)
  fleet2$n_ages$set(3L)
  fleet2$n_lengths$set(0L)
  fleet2$log_Fmort[] <- log(c(0.2, 0.25))
  fleet2$log_q[] <- 0
  fleet2$SetSelectivityID(selectivity$get_id())
  population$AddFleet(fleet2$get_id())

  model <- methods::new(CatchAtAge)
  model$AddPopulation(population$get_id())
  CreateTMBModel()

  pop_id <- population$get_id()
  fixed_names <- names(get_parameter_names(as.list(get_fixed())))
  random_names <- names(get_random_names(as.list(get_random())))
  fitted_fixed <- stats::setNames(c(log(1000), log(0.1), log(0.15)), fixed_names)
  fitted_random <- stats::setNames(c(0.15), random_names)

  cov_fixed <- diag(c(0.002, 0.001, 0.001))
  dimnames(cov_fixed) <- list(fixed_names, fixed_names)
  joint_cov <- diag(c(0.002, 0.001, 0.001, 0.25))
  joint_prec <- solve(joint_cov)
  dimnames(joint_prec) <- list(c(fixed_names, random_names), c(fixed_names, random_names))

  sdrep <- list(
    pdHess = TRUE,
    par.fixed = fitted_fixed,
    cov.fixed = cov_fixed,
    par.random = fitted_random,
    jointPrecision = joint_prec
  )

  # Set custom live parameters before calling uncertainty to verify full state restoration.
  custom_fixed <- c(log(1500), log(0.25), log(0.35))
  custom_random <- c(-0.25)
  set_fixed(custom_fixed)
  set_random(custom_random)
  custom_snapshot <- model$reference_point_inputs(pop_id, 2L, numeric())

  evaluator <- make_reference_point_evaluator(
    model = model, population = pop_id, reference_year = 2L,
    include_in_msy = c(TRUE, FALSE), calendar_years = c(2021, 2022)
  )
  # Update held random effect on evaluator to match fitted_random.
  attr(evaluator, "held_random") <- fitted_random

  set.seed(77)
  cond_res <- calculate_stock_status_uncertainty(
    sdrep, evaluator, n_draws = 80
  )
  expect_equal(cond_res$conditioning, "conditional_on_random_effects")
  expect_null(cond_res$random_draws)
  expect_equal(nrow(cond_res$errors), 0L)
  expect_equal(cond_res$summary$calendar_year, c(2021, 2022, 2023))

  # Verify live parameters and evaluated model state were restored to custom_fixed / custom_random.
  expect_equal(get_fixed(), custom_fixed)
  expect_equal(get_random(), custom_random)
  expect_equal(model$reference_point_inputs(pop_id, 2L, numeric()), custom_snapshot)

  # Verify restoration also happens when an error occurs during evaluation.
  expect_error(
    evaluator(c(log(1000), NA_real_, log(0.15))),
    "finite"
  )
  expect_equal(get_fixed(), custom_fixed)
  expect_equal(get_random(), custom_random)
  expect_equal(model$reference_point_inputs(pop_id, 2L, numeric()), custom_snapshot)

  # Exercise paired joint fixed/random-effect draws from jointPrecision.
  set.seed(77)
  joint_res <- calculate_stock_status_uncertainty(
    sdrep, evaluator, n_draws = 80, include_random = TRUE
  )
  expect_equal(joint_res$conditioning, "joint_fixed_and_random_effects")
  expect_equal(dim(joint_res$random_draws), c(80L, 1L))
  # Year 2 recruitment depends on the random effect log_devs[1], so year 2 SB/SB_MSY SD increases.
  expect_gt(joint_res$summary$sb_over_sb_msy_sd[2], cond_res$summary$sb_over_sb_msy_sd[2])
  expect_equal(get_fixed(), custom_fixed)
  expect_equal(get_random(), custom_random)

  # Misordered jointPrecision names must be rejected.
  bad_sdrep <- sdrep
  dimnames(bad_sdrep$jointPrecision) <- rep(list(rev(c(fixed_names, random_names))), 2)
  expect_error(
    calculate_stock_status_uncertainty(bad_sdrep, evaluator, n_draws = 10, include_random = TRUE),
    "jointPrecision"
  )

  # Exercise the direct FIMSFit route for point estimates and uncertainty, verifying state restoration.
  mock_env <- new.env(parent = emptyenv())
  mock_env$last.par.best <- c(unname(fitted_fixed), unname(fitted_random))
  mock_env$random <- 4L
  mock_env$map <- list()
  mock_env$parList <- function(x = unname(fitted_fixed), par = mock_env$last.par.best) {
    list(p = as.numeric(x), re = as.numeric(par[mock_env$random]))
  }
  mock_obj <- list(
    par = fitted_fixed,
    fn = function(x) 0,
    gr = function(x) rep(0, length(x)),
    he = function(x) diag(length(x)),
    hessian = FALSE,
    method = "nlminb",
    retape = function() NULL,
    env = mock_env,
    report = function(x = NULL) list(jnll = 0, spawning_biomass = list(c(1, 1, 1))),
    simulate = function() list()
  )
  fit_obj <- methods::new(
    "FIMSFit",
    input = list(
      parameters = list(p = unname(fitted_fixed), re = unname(fitted_random)),
      model = model
    ),
    obj = mock_obj,
    opt = list(par = fitted_fixed, objective = 0, convergence = 0L),
    max_gradient = 0,
    gradient = rep(0, length(fitted_fixed)),
    report = list(jnll = 0, spawning_biomass = list(c(1, 1, 1))),
    sdreport = sdrep,
    number_of_parameters = c(fixed_effects = 3L, random_effects = 1L),
    run_time = c(time_total = as.difftime(0, units = "secs")),
    version = utils::packageVersion("FIMS"),
    model_output = "{}"
  )

  set_fixed(custom_fixed)
  set_random(custom_random)
  fit_status <- get_stock_status(
    fit_obj, population = pop_id, reference_year = 2L,
    include_in_msy = c(TRUE, FALSE), calendar_years = c(2021, 2022)
  )
  expect_equal(fit_status$stock_status, cond_res$estimate$stock_status)
  expect_equal(get_fixed(), custom_fixed)
  expect_equal(get_random(), custom_random)

  set.seed(77)
  fit_unc <- get_stock_status_uncertainty(
    fit_obj, population = pop_id, reference_year = 2L,
    n_draws = 80, include_in_msy = c(TRUE, FALSE),
    calendar_years = c(2021, 2022), include_random = TRUE
  )
  expect_equal(fit_unc$summary, joint_res$summary)
  expect_equal(get_fixed(), custom_fixed)
  expect_equal(get_random(), custom_random)
})


