test_that("the recruitment helper sets n_years before model creation", {
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)

  clear()
  on.exit(clear())

  recruitment <- initialize_recruitment(parameters, data)

  expect_s3_class(recruitment, "fims_module")
  expect_no_error(CreateTMBModel())
})

test_that("model creation diagnoses a missing recruitment year count", {
  clear()
  on.exit(clear())

  create_recruitment("BevertonHolt")

  expect_error(
    CreateTMBModel(),
    "BevertonHoltRecruitment requires n_years to be greater than zero"
  )
})

test_that("the XPtr helper model can be taped by TMB", {
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)

  clear()
  on.exit(clear())

  input <- initialize_fims(parameters, data)
  fit <- fit_fims(input, optimize = FALSE)

  expect_s4_class(fit, "FIMSFit")
})
