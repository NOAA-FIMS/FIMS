test_that("native information state can be cleared and inspected", {
  expect_null(native_clear())
  expect_equal(native_get_fixed(), numeric())
  expect_equal(native_get_random(), numeric())
  expect_equal(native_get_parameter_names(), character())
  expect_equal(native_get_random_effect_names(), character())
  expect_equal(
    unname(native_information_model_counts()),
    rep(0L, 5L)
  )
})

test_that("native random effects retain their domain identity", {
  native_clear()
  recruitment_beverton_holt_create(
    logit_steep = 0,
    log_rzero = 10,
    log_devs = c(0.1, -0.1),
    log_devs_estimation_type = "random_effects"
  )
  on.exit(native_clear(), add = TRUE)

  random_effects <- native_get_random()
  expected_names <- c(
    "Recruitment.1.log_devs.0",
    "Recruitment.1.log_devs.1"
  )

  expect_equal(native_get_random_effect_names(), expected_names)
  expect_equal(names(get_random_names(random_effects)), expected_names)
})

test_that("native model assembly requires a population", {
  native_clear()
  expect_error(
    native_create_model(),
    "No populations are registered"
  )
})

test_that("native scalar identifiers and dimensions are not silently truncated", {
  expect_error(
    native_add_prior("Selectivity", 1.5, "inflection_point"),
    "object_id.*whole number"
  )
  expect_error(
    native_build_default_likelihood(
      fishing_fleet_id = 1,
      survey_fleet_id = 2,
      landings = numeric(),
      landings_sd = 1,
      landings_age_comp = numeric(),
      landings_length_comp = numeric(),
      survey_index = numeric(),
      survey_sd = 1,
      survey_age_comp = numeric(),
      survey_length_comp = numeric(),
      recruitment_log_sd = 0,
      n_years = 1.5,
      n_ages = 1,
      n_lengths = 0
    ),
    "n_years.*whole number"
  )
})

test_that("native priors link parameter vectors and register hyperparameters", {
  native_clear()
  selectivity_id <- selectivity_logistic_create(
    inflection_point = c(2, 3),
    slope = 0.5,
    inflection_point_estimation_type = "fixed_effects",
    slope_estimation_type = "constant"
  )

  expect_true(native_add_prior(
    module = "Selectivity",
    object_id = selectivity_id,
    parameter = "inflection_point",
    distribution = "Dnorm",
    mean = 2.5,
    log_sd = log(0.5),
    mean_estimation_type = "fixed_effects",
    log_sd_estimation_type = "fixed_effects"
  ))
  expect_equal(native_information_model_counts()[["density_components"]], 1L)
  expect_match(native_get_parameter_names(), "Dnorm.*mean", all = FALSE)
  expect_match(native_get_parameter_names(), "Dnorm.*log_sd", all = FALSE)

  expect_error(
    native_add_prior("Selectivity", selectivity_id, "missing", mean = 0),
    "Could not find native prior target"
  )
  expect_error(
    native_add_prior("Selectivity", selectivity_id, "inflection_point", mean = 1:3),
    "must have length 1 or match"
  )
  expect_error(
    native_add_prior(
      "Selectivity", selectivity_id, "inflection_point",
      mean_estimation_type = "random_effects"
    ),
    "cannot be random effects"
  )
})

test_that("native prior validation rejects malformed values before mutation", {
  native_clear()
  selectivity_id <- selectivity_logistic_create(
    inflection_point = 2,
    slope = 0.5,
    inflection_point_estimation_type = "fixed_effects"
  )
  on.exit(native_clear(), add = TRUE)
  counts_before <- native_information_model_counts()
  names_before <- native_get_parameter_names()

  expect_error(
    .Call(
      "fims_call_add_prior",
      "Selectivity",
      -1,
      "inflection_point",
      "normal",
      2,
      0,
      0L,
      0L,
      PACKAGE = "FIMS"
    ),
    "object_id.*greater than or equal to 0"
  )
  expect_error(
    .Call(
      "fims_call_add_prior",
      "Selectivity",
      selectivity_id,
      "inflection_point",
      "normal",
      NA_real_,
      0,
      0L,
      0L,
      PACKAGE = "FIMS"
    ),
    "mean.*finite"
  )

  expect_equal(native_information_model_counts(), counts_before)
  expect_equal(native_get_parameter_names(), names_before)
})
