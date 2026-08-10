data <- FIMSFrame(data_big)
default_parameters <- create_default_configurations(data = data) |>
  create_default_parameters(data = data)

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
    tidyr::unnest(cols = "data") |>
    dplyr::filter(is.na(.data$fleet))
  expect_error(
    initialize_fims(no_fleets, data),
    "No fleets found"
  )

  invalid_type <- default_parameters |>
    tidyr::unnest(cols = "data") |>
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
