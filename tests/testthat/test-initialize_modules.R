data <- FIMSFrame(data_big)
default_parameters <- setup_default_parameters(data = data)

test_that("initialize_fims() builds a native model", {
  result <- initialize_fims(default_parameters, data)

  expect_named(result, c("parameters", "model"))
  expect_named(result$parameters, c("p", "re"))
  expect_gt(length(result$parameters$p), 0L)
  expect_gt(length(result$parameters$re), 0L)
  expect_gt(native_information_model_counts()[["models_map"]], 0L)

  obj <- TMB::MakeADFun(
    data = list(),
    parameters = result$parameters,
    random = "re",
    DLL = "FIMS",
    silent = TRUE
  )
  expect_true(is.finite(obj$fn(obj$par)))
})

test_that("initialize_fims() validates native model constraints", {
  expect_error(
    initialize_fims(data = data),
    "parameters.*must be a tibble"
  )
  expect_error(
    initialize_fims(parameters = "not a tibble", data = data),
    "parameters.*must be a tibble"
  )

  no_fleets <- default_parameters |>
    dplyr::filter(is.na(.data$fleet))
  expect_error(
    initialize_fims(no_fleets, data),
    "No fleets found"
  )

  invalid_type <- default_parameters |>
    dplyr::mutate(
      estimation_type = dplyr::if_else(
        .data$estimation_type == "fixed_effects",
        "fixed.effects",
        .data$estimation_type
      )
    )
  expect_error(
    initialize_fims(invalid_type, data),
    "estimation_type"
  )
})

test_that("native priors contribute their density to the objective", {
  baseline <- initialize_fims(default_parameters, data)
  baseline_obj <- TMB::MakeADFun(
    data = list(), parameters = baseline$parameters, random = "re",
    DLL = "FIMS", silent = TRUE
  )
  baseline_value <- baseline_obj$fn(baseline_obj$par)

  with_prior <- initialize_fims(default_parameters, data)
  maturity_value <- default_parameters |>
    dplyr::filter(
      .data$module_name == "Maturity",
      .data$label == "inflection_point"
    ) |>
    dplyr::pull(.data$value)
  native_add_prior(
    module = "Maturity",
    object_id = with_prior$model$maturity_id,
    parameter = "inflection_point",
    distribution = "normal",
    mean = 0,
    log_sd = 0
  )
  prior_obj <- TMB::MakeADFun(
    data = list(), parameters = with_prior$parameters, random = "re",
    DLL = "FIMS", silent = TRUE
  )

  expected_prior_nll <- sum(-stats::dnorm(
    maturity_value,
    mean = 0, sd = 1, log = TRUE
  ))
  expect_equal(
    as.numeric(prior_obj$fn(prior_obj$par) - baseline_value),
    as.numeric(expected_prior_nll),
    tolerance = 1e-8
  )
})
test_that("`initialize_fims()` accepts legacy parameter timing columns", {
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data) |>
    dplyr::rename(time = "timing")

  initialized <- initialize_fims(parameters, data)
  on.exit(native_clear(), add = TRUE)

  expect_true(length(initialized[["parameters"]][["p"]]) > 0L)
  expect_equal(
    length(native_get_parameter_names()),
    length(initialized[["parameters"]][["p"]])
  )
})

test_that("`initialize_fims()` supports models with one composition type", {
  for (excluded_type in c("age_comp", "length_comp")) {
    composition_data <- data_big |>
      dplyr::filter(.data[["type"]] != .env[["excluded_type"]]) |>
      FIMSFrame()
    parameters <- setup_default_parameters(composition_data)

    expect_no_error(initialize_fims(parameters, composition_data))
    native_clear()
  }
})
