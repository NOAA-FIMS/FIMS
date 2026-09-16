test_that("native information state can be cleared and inspected", {
  expect_null(native_clear())
  expect_equal(native_get_fixed(), numeric())
  expect_equal(native_get_random(), numeric())
  expect_equal(native_get_parameter_names(), character())
  expect_equal(
    unname(native_information_model_counts()),
    rep(0L, 5L)
  )
})

test_that("native model assembly requires a population", {
  native_clear()
  expect_error(
    native_create_model(),
    "No populations are registered"
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

test_that("native prior targets resolve every supported module and parameter", {
  on.exit(native_clear(), add = TRUE)
  cases <- list(
    list(module = "Fleet", create = function() fleet_create(c(-2, -1), c(0, 1)),
         parameters = c("log_Fmort", "log_q")),
    list(module = "Population", create = function() population_create(
      c(-2, -1), c(0, 1), c(5, 6)),
      parameters = c("log_M", "log_f_multiplier", "log_init_naa")),
    list(module = "Recruitment", create = function() recruitment_beverton_holt_create(
      c(0, 1), c(5, 6), c(-0.1, 0.1)),
      parameters = c("logit_steep", "log_rzero", "log_devs")),
    list(module = "Maturity", create = function() maturity_logistic_create(
      c(2, 3), c(0.5, 1)), parameters = c("inflection_point", "slope")),
    list(module = "Selectivity", create = function() selectivity_logistic_create(
      c(2, 3), c(0.5, 1)), parameters = c("inflection_point", "slope")),
    list(module = "Selectivity", create = function() selectivity_double_logistic_create(
      c(2, 3), c(0.5, 1), c(8, 9), c(0.5, 1)),
      parameters = c("inflection_point_asc", "slope_asc", "inflection_point_desc", "slope_desc"))
  )
  for (case in cases) {
    for (parameter in case$parameters) {
      native_clear()
      id <- case$create()
      for (distribution in c("Dnorm", "Dlnorm")) {
        expect_true(native_add_prior(
          case$module, id, parameter, distribution = distribution,
          mean = 1, log_sd = log(0.5)
        ), info = paste(case$module, parameter, distribution))
      }
      expect_equal(native_information_model_counts()[["density_components"]], 2L)
      expect_error(native_add_prior(case$module, id, parameter, mean = 1:3),
                   "must have length 1 or match")
      expect_error(native_add_prior(case$module, id, "missing"),
                   "Could not find native prior target")
      expect_error(native_add_prior(case$module, 2147483647L, parameter),
                   "Could not find native prior target")
    }
  }
})
