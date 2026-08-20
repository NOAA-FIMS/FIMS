test_that("catch-at-age model assembles directly through native calls", {
  native_clear()
  on.exit(native_clear(), add = TRUE)

  n_years <- 3L
  ages <- 1:3
  n_ages <- length(ages)
  n_lengths <- n_ages
  age_to_length <- as.numeric(t(diag(n_ages)))

  fishing_selectivity_id <- selectivity_logistic_create(
    inflection_point = 2,
    slope = 1,
    inflection_point_estimation_type = "fixed_effects",
    slope_estimation_type = "fixed_effects"
  )
  survey_selectivity_id <- selectivity_logistic_create(
    inflection_point = 1.5,
    slope = 1.2,
    inflection_point_estimation_type = "fixed_effects",
    slope_estimation_type = "fixed_effects"
  )
  fishing_fleet_id <- fleet_create(
    log_fmort = rep(log(0.2), n_years),
    log_q = 0,
    selectivity_id = fishing_selectivity_id,
    age_to_length_conversion = age_to_length,
    log_fmort_estimation_type = "fixed_effects"
  )
  survey_fleet_id <- fleet_create(
    log_fmort = rep(-200, n_years),
    log_q = log(0.5),
    selectivity_id = survey_selectivity_id,
    age_to_length_conversion = age_to_length,
    log_q_estimation_type = "fixed_effects"
  )
  recruitment_id <- recruitment_beverton_holt_create(
    logit_steep = 0.5,
    log_rzero = log(1000),
    log_devs = rep(0, n_years - 1L),
    logit_steep_estimation_type = "fixed_effects",
    log_rzero_estimation_type = "fixed_effects",
    log_devs_estimation_type = "random_effects"
  )
  growth_id <- growth_ewaa_create(
    ages = ages,
    weights = rep(c(1, 2, 3), n_years + 1L),
    n_years = n_years
  )
  maturity_id <- maturity_logistic_create(
    inflection_point = 2,
    slope = 1
  )
  population_create(
    log_m = log(0.2),
    log_f_multiplier = rep(0, n_years),
    log_init_naa = log(c(1000, 500, 250)),
    maturity_id = maturity_id,
    growth_id = growth_id,
    recruitment_id = recruitment_id,
    fleet_ids = c(fishing_fleet_id, survey_fleet_id)
  )

  expect_type(native_create_model(), "logical")
  expect_true(native_build_default_likelihood(
    fishing_fleet_id = fishing_fleet_id,
    survey_fleet_id = survey_fleet_id,
    landings = c(100, 110, 120),
    landings_cv = 0.1,
    landings_age_comp = numeric(),
    landings_length_comp = numeric(),
    survey_index = c(500, 475, 450),
    survey_cv = 0.2,
    survey_age_comp = numeric(),
    survey_length_comp = numeric(),
    recruitment_log_sd = 0.3,
    n_years = n_years,
    n_ages = n_ages,
    n_lengths = n_lengths
  ))

  parameters <- list(
    p = native_get_fixed(),
    re = native_get_random()
  )
  expect_gt(length(parameters$p), 0L)
  expect_length(parameters$re, n_years - 1L)

  obj <- TMB::MakeADFun(
    data = list(),
    parameters = parameters,
    random = "re",
    DLL = "FIMS",
    silent = TRUE
  )
  expect_true(is.finite(obj$fn(obj$par)))

  report <- obj$report()
  expect_length(report$catch_expected[[1]], n_years)
  expect_length(report$index_expected[[2]], n_years)
  expect_true(all(is.finite(report$nll_components)))
})
