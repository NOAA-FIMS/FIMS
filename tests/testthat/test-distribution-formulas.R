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

test_that("native likelihood honors normal and observation-specific uncertainty formulas", {
  data <- get_data(FIMSFrame(data_big))
  parameters <- setup_default_parameters(FIMSFrame(data))

  gaussian_data <- data
  gaussian_data$uncertainty[gaussian_data$type == "catch"] <-
    "~dnorm(mean = catch_expected, sd = 0.1)"
  gaussian_model <- initialize_fims(parameters, FIMSFrame(gaussian_data))
  gaussian_obj <- TMB::MakeADFun(
    data = list(), parameters = gaussian_model$parameters, random = "re",
    DLL = "FIMS", silent = TRUE
  )
  expect_true(is.finite(gaussian_obj$fn(gaussian_obj$par)))
  gaussian_report <- gaussian_obj$report()
  catch_observed <- gaussian_data$observed[gaussian_data$type == "catch"]
  expected_catch_nll <- -sum(stats::dnorm(
    catch_observed,
    gaussian_report$catch_expected[[1]],
    sd = 0.1,
    log = TRUE
  ))
  expect_true(any(abs(
    gaussian_report$nll_components - expected_catch_nll
  ) < 1e-6))

  varying_data <- data
  catch_rows <- which(varying_data$type == "catch")
  varying_data$uncertainty[catch_rows[[1L]]] <-
    "~dlnorm(meanlog = log_catch_expected, sdlog = 0.2)"
  varying_model <- initialize_fims(parameters, FIMSFrame(varying_data))
  varying_obj <- TMB::MakeADFun(
    data = list(), parameters = varying_model$parameters, random = "re",
    DLL = "FIMS", silent = TRUE
  )
  expect_true(is.finite(varying_obj$fn(varying_obj$par)))

  mixed_data <- data
  mixed_data$uncertainty[catch_rows[[1L]]] <-
    "~dnorm(mean = catch_expected, sd = 0.1)"
  expect_error(
    initialize_fims(parameters, FIMSFrame(mixed_data)),
    "supports one continuous family"
  )

  invalid_composition <- data
  invalid_composition$uncertainty[
    invalid_composition$type == "age_comp"
  ] <- "~dnorm(mean = agecomp_proportion, sd = 0.1)"
  expect_error(
    initialize_fims(parameters, FIMSFrame(invalid_composition)),
    "require `dmultinom"
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
