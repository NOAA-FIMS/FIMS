test_that("native parameter identities match fixed and random vectors", {
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)
  initialized <- initialize_fims(parameters, data)
  on.exit(native_clear(), add = TRUE)

  fixed_names <- native_get_parameter_names()
  random_names <- native_get_random_effect_names()

  expect_length(fixed_names, length(initialized[["parameters"]][["p"]]))
  expect_length(random_names, length(initialized[["parameters"]][["re"]]))
  expect_true(all(grepl("^[A-Za-z]+\\.[0-9]+\\.[A-Za-z_]+\\.[0-9]+$", fixed_names)))
  expect_true(all(grepl("^[A-Za-z]+\\.[0-9]+\\.[A-Za-z_]+\\.[0-9]+$", random_names)))
  expect_true(all(grepl("^Recruitment\\.1\\.log_devs\\.", random_names)))
})

test_that("native TMB objective derivatives agree with finite differences", {
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)
  initialized <- initialize_fims(parameters, data)
  on.exit(native_clear(), add = TRUE)

  obj <- TMB::MakeADFun(
    data = list(),
    parameters = initialized[["parameters"]],
    DLL = "FIMS",
    silent = TRUE
  )
  parameter_values <- obj[["par"]]
  direction <- seq_along(parameter_values)
  direction <- direction / sqrt(sum(direction^2))
  step <- 1e-5

  objective_directional_derivative <- sum(obj[["gr"]](parameter_values) * direction)
  finite_difference_derivative <- (
    obj[["fn"]](parameter_values + step * direction) -
      obj[["fn"]](parameter_values - step * direction)
  ) / (2 * step)
  derivative_scale <- max(1, abs(finite_difference_derivative))

  expect_equal(
    objective_directional_derivative / derivative_scale,
    finite_difference_derivative / derivative_scale,
    tolerance = 1e-5
  )

  hessian <- obj[["he"]](parameter_values)
  finite_difference_hessian_direction <- (
    obj[["gr"]](parameter_values + step * direction) -
      obj[["gr"]](parameter_values - step * direction)
  ) / (2 * step)
  hessian_scale <- max(1, max(abs(finite_difference_hessian_direction)))

  expect_equal(hessian, t(hessian), tolerance = 1e-10)
  expect_equal(
    as.numeric(hessian %*% direction) / hessian_scale,
    as.numeric(finite_difference_hessian_direction) / hessian_scale,
    tolerance = 1e-4
  )
})

test_that("native recruitment likelihood preserves its scale and report order", {
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)
  initialized <- initialize_fims(parameters, data)
  on.exit(native_clear(), add = TRUE)
  obj <- TMB::MakeADFun(
    data = list(),
    parameters = initialized[["parameters"]],
    DLL = "FIMS",
    silent = TRUE
  )

  report <- obj[["report"]](c(
    initialized[["parameters"]][["p"]],
    initialized[["parameters"]][["re"]]
  ))
  recruitment_rows <- parameters[["module_name"]] == "Recruitment" &
    parameters[["label"]] == "log_devs"
  recruitment_log_sd <- parameters |>
    dplyr::filter(
      .data[["module_name"]] == "Recruitment",
      .data[["label"]] == "log_sd"
    ) |>
    dplyr::pull(.data[["value"]])
  expected_recruitment_nll <- sum(recruitment_rows) *
    (recruitment_log_sd + 0.5 * log(2 * pi))

  expect_length(report[["nll_components"]], 7L)
  expect_equal(
    report[["nll_components"]][[1L]],
    expected_recruitment_nll,
    tolerance = 1e-10
  )
  expect_true(any(grepl(
    "Recruitment.1.log_sd.0",
    native_get_parameter_names(),
    fixed = TRUE
  )))
})
