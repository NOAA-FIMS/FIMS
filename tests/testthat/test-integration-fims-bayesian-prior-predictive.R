test_that("posterior equals prior with no data through the native interface", {
  skip_if_not_installed("SparseNUTS")

  # With no data likelihood, the posterior for the four estimated selectivity
  # parameters should be exactly their independent normal priors.
  native_clear()
  on.exit(native_clear(), add = TRUE)

  n_years <- 3L
  ages <- 1:4
  n_ages <- length(ages)
  age_to_length <- diag(n_ages)

  inflection_mean <- 2.5
  inflection_sd <- 3
  log_slope_mean <- log(0.8)
  log_slope_sd <- 0.4

  fishing_selectivity_id <- selectivity_logistic_create(
    inflection_point = 2,
    slope = 0.6,
    inflection_point_estimation_type = "fixed_effects",
    slope_estimation_type = "fixed_effects"
  )
  survey_selectivity_id <- selectivity_logistic_create(
    inflection_point = 3,
    slope = 1,
    inflection_point_estimation_type = "fixed_effects",
    slope_estimation_type = "fixed_effects"
  )

  fishing_fleet_id <- fleet_create(
    log_fmort = rep(log(0.2), n_years),
    log_q = 0,
    selectivity_id = fishing_selectivity_id,
    age_to_length_conversion = as.numeric(t(age_to_length))
  )
  survey_fleet_id <- fleet_create(
    log_fmort = rep(-200, n_years),
    log_q = log(0.5),
    selectivity_id = survey_selectivity_id,
    age_to_length_conversion = as.numeric(t(age_to_length))
  )

  recruitment_id <- recruitment_beverton_holt_create(
    logit_steep = 0,
    log_rzero = log(1000),
    log_devs = rep(0, n_years - 1L)
  )
  growth_id <- growth_ewaa_create(
    ages = ages,
    weights = rep(ages, n_years + 1L),
    n_years = n_years
  )
  maturity_id <- maturity_logistic_create(
    inflection_point = 2.5,
    slope = 1
  )
  population_create(
    log_m = log(0.2),
    log_f_multiplier = rep(0, n_years),
    log_init_naa = rep(log(1000), n_ages),
    maturity_id = maturity_id,
    growth_id = growth_id,
    recruitment_id = recruitment_id,
    fleet_ids = c(fishing_fleet_id, survey_fleet_id)
  )

  for (selectivity_id in c(fishing_selectivity_id, survey_selectivity_id)) {
    native_add_prior(
      module = "Selectivity",
      object_id = selectivity_id,
      parameter = "inflection_point",
      mean = inflection_mean,
      log_sd = log(inflection_sd)
    )
    # Native logistic selectivity estimates slope on the log scale.
    native_add_prior(
      module = "Selectivity",
      object_id = selectivity_id,
      parameter = "slope",
      mean = log_slope_mean,
      log_sd = log(log_slope_sd)
    )
  }

  expect_type(native_create_model(), "logical")
  parameters <- list(p = native_get_fixed(), re = native_get_random())
  expect_length(parameters$p, 4L)
  expect_length(parameters$re, 0L)

  obj <- TMB::MakeADFun(
    data = list(), parameters = parameters, DLL = "FIMS", silent = TRUE
  )

  parameter_names <- native_get_parameter_names()
  names(obj$par) <- parameter_names
  inflection <- grepl("inflection_point", parameter_names, fixed = TRUE)
  log_slope <- grepl("log_slope", parameter_names, fixed = TRUE)
  expect_equal(sum(inflection), 2L)
  expect_equal(sum(log_slope), 2L)

  expected_nll <- -sum(stats::dnorm(
    obj$par[inflection], inflection_mean, inflection_sd, log = TRUE
  )) - sum(stats::dnorm(
    obj$par[log_slope], log_slope_mean, log_slope_sd, log = TRUE
  ))
  expect_equal(obj$fn(obj$par), expected_nll, tolerance = 1e-10)

  fit <- NULL
  invisible(utils::capture.output(
    invisible(utils::capture.output(
      fit <- SparseNUTS::sample_snuts(
        obj,
        chains = 1,
        print = FALSE,
        refresh = 0,
        quiet = TRUE
      ),
      type = "message"
    ))
  ))

  expect_equal(unname(fit$mle$est[inflection]), rep(inflection_mean, 2L))
  expect_equal(unname(fit$mle$se[inflection]), rep(inflection_sd, 2L))
  expect_equal(unname(fit$mle$est[log_slope]), rep(log_slope_mean, 2L))
  expect_equal(unname(fit$mle$se[log_slope]), rep(log_slope_sd, 2L))
})
