test_that("native likelihood reads lognormal uncertainty formulas", {
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)

  initialized <- initialize_fims(parameters, data)
  on.exit(native_clear(), add = TRUE)
  obj <- TMB::MakeADFun(
    data = list(),
    parameters = initialized$parameters,
    random = "re",
    DLL = "FIMS",
    silent = TRUE
  )

  expect_true(is.finite(obj$fn(obj$par)))
  expect_gte(native_information_model_counts()[["density_components"]], 5L)
})

test_that("native likelihood rejects unsupported or inconsistent uncertainty formulas", {
  data <- get_data(FIMSFrame(data_big))
  parameters <- setup_default_parameters(FIMSFrame(data))

  gaussian_data <- data
  gaussian_data$uncertainty[gaussian_data$type == "catch"] <-
    "~dnorm(mean = catch_expected, sd = 0.1)"
  expect_error(
    initialize_fims(parameters, FIMSFrame(gaussian_data)),
    "requires a lognormal distribution"
  )

  inconsistent_data <- data
  catch_rows <- which(inconsistent_data$type == "catch")
  inconsistent_data$uncertainty[catch_rows[[1L]]] <-
    "~dlnorm(meanlog = log_catch_expected, sdlog = 0.2)"
  expect_error(
    initialize_fims(parameters, FIMSFrame(inconsistent_data)),
    "requires one uncertainty specification"
  )
})

test_that("native priors validate distributions, scales, and target lengths", {
  native_clear()
  on.exit(native_clear(), add = TRUE)
  selectivity_id <- selectivity_logistic_create(
    inflection_point = c(2, 3),
    slope = c(0.5, 0.7),
    inflection_point_estimation_type = "fixed_effects",
    slope_estimation_type = "fixed_effects"
  )

  expect_error(
    native_add_prior(
      "Selectivity", selectivity_id, "inflection_point",
      distribution = "gamma"
    ),
    "arg"
  )
  expect_error(
    native_add_prior(
      "Selectivity", selectivity_id, "inflection_point",
      mean = 1:3
    ),
    "length 1 or match"
  )
  expect_error(
    native_add_prior(
      "Selectivity", selectivity_id, "inflection_point",
      log_sd = c(0, Inf)
    ),
    "finite"
  )
})
