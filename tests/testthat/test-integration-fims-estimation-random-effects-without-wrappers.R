test_that("native recruitment deviations are registered as random effects", {
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)
  recruitment_rows <- parameters$module_name == "Recruitment" &
    parameters$label == "log_devs"

  expect_true(all(
    parameters$estimation_type[recruitment_rows] == "random_effects"
  ))

  initialized <- initialize_fims(parameters, data)
  on.exit(native_clear(), add = TRUE)
  expect_length(initialized$parameters$re, sum(recruitment_rows))
  obj <- TMB::MakeADFun(
    data = list(),
    parameters = initialized$parameters,
    random = "re",
    DLL = "FIMS",
    silent = TRUE
  )
  expect_true(is.finite(obj$fn(obj$par)))
  expect_length(obj$env$random, sum(recruitment_rows))
  expect_length(obj$env$parList()$re, sum(recruitment_rows))
  expect_true(all(is.finite(obj$env$parList()$re)))
  expect_length(
    obj$report()$expected_recruitment[[1]],
    get_n_years(data) + 1L
  )
})

test_that("native recruitment fixed and random effects give the same conditional objective", {
  data <- FIMSFrame(data_big)
  random_parameters <- setup_default_parameters(data)
  recruitment_rows <- random_parameters$module_name == "Recruitment" &
    random_parameters$label == "log_devs"

  random_model <- initialize_fims(random_parameters, data)
  random_obj <- TMB::MakeADFun(
    data = list(),
    parameters = random_model$parameters,
    DLL = "FIMS",
    silent = TRUE
  )
  random_value <- random_obj$fn(c(random_model$parameters$p, random_model$parameters$re))

  fixed_parameters <- random_parameters
  fixed_parameters$estimation_type[recruitment_rows] <- "fixed_effects"
  fixed_model <- initialize_fims(fixed_parameters, data)
  on.exit(native_clear(), add = TRUE)
  fixed_obj <- TMB::MakeADFun(
    data = list(),
    parameters = fixed_model$parameters,
    DLL = "FIMS",
    silent = TRUE
  )

  expect_length(fixed_model$parameters$re, 0L)
  expect_equal(
    fixed_obj$fn(fixed_obj$par),
    random_value,
    tolerance = 1e-8
  )
})
