devtools::test(filter = "without-wrappers")testthat::skip_if(
  Sys.getenv("RUN_SLOW_TESTS") != "true",
  "Skipping: RUN_SLOW_TESTS is not set to true."
)

load(testthat::test_path("fixtures", "integration_test_data.RData"))

# Set the iteration ID to 1 for accessing specific input/output list
iter_id <- 1

# Extract model input and output data for the specified iteration
om_input <- om_input_list[[iter_id]]
om_output <- om_output_list[[iter_id]]
em_input <- em_input_list[[iter_id]]

test_that("1. estimation test with recruitment re on log_r", {
  #' @description Test that the estimation run of FIMS with recruitment random effects on log_r matches expected behavior.
  fims_data <- FIMS::FIMSFrame(data_big)

  # Clear any previous FIMS settings
  clear()

  fleets <- list(
    fleet1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    ),
    survey1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    )
  )

  default_parameters <- fims_data |>
    setup_default_parameters(
      fleets = fleets,
      recruitment = list(
        form = "BevertonHoltRecruitment"
      )
    ) |>
    setup_default_process(
      data = fims_data,
      module = "recruitment",
      par = "log_r",
      process_distribution = gaussian(),
      estimated = TRUE,
      random = TRUE
    )

  modified_parameters <- list(
    fleet1 = list(
      Fleet.log_Fmort.value = log(om_output_list[[1]][["f"]])
    ),
    survey1 = list(
      LogisticSelectivity.inflection_point.value = 1.5,
      LogisticSelectivity.slope.value = 2,
      Fleet.log_q.value = log(om_output_list[[1]][["survey_q"]][["survey1"]])
    ),
    recruitment = list(
      BevertonHoltRecruitment.log_rzero.value = log(om_input_list[[1]][["R0"]]),
      DnormDistribution.log_sd.value = om_input_list[[1]][["logR_sd"]]
    ),
    maturity = list(
      LogisticMaturity.inflection_point.value = om_input_list[[1]][["A50.mat"]],
      LogisticMaturity.inflection_point.estimated = FALSE,
      LogisticMaturity.slope.value = om_input_list[[1]][["slope.mat"]],
      LogisticMaturity.slope.estimated = FALSE
    ),
    population = list(
      Population.log_init_naa.value = log(om_output_list[[1]][["N.age"]][1, ])
    )
  )

  parameters <- default_parameters |>
    update_parameters(
      modified_parameters = modified_parameters
    )

  parameter_list <- initialize_fims(
    parameters = parameters,
    data = fims_data
  )
  fit_log_r <- fit_fims(parameter_list, optimize = TRUE)

  # make sure random effects are turned on
  #' @description Test that the random effects parameters are correctly set up for log_r.
  expect_equal(fit_log_r@obj[["env"]][["parameters"]][["re"]], rep(0, get_n_years(fims_data) - 1))

  clear()
})

test_that("2. compare recruitment log_devs vs log_r implementations", {
  #' @description Test comparing log_r and log_devs recruitment random effects implementations.
  fims_data <- FIMS::FIMSFrame(data_big)

  # Fit log_r
  clear()
  fleets <- list(
    fleet1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    ),
    survey1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    )
  )

  default_parameters_log_r <- fims_data |>
    setup_default_parameters(
      fleets = fleets,
      recruitment = list(
        form = "BevertonHoltRecruitment"
      )
    ) |>
    setup_default_process(
      data = fims_data,
      module = "recruitment",
      par = "log_r",
      process_distribution = gaussian(),
      estimated = TRUE,
      random = TRUE
    )

  modified_parameters <- list(
    fleet1 = list(
      Fleet.log_Fmort.value = log(om_output_list[[1]][["f"]])
    ),
    survey1 = list(
      LogisticSelectivity.inflection_point.value = 1.5,
      LogisticSelectivity.slope.value = 2,
      Fleet.log_q.value = log(om_output_list[[1]][["survey_q"]][["survey1"]])
    ),
    recruitment = list(
      BevertonHoltRecruitment.log_rzero.value = log(om_input_list[[1]][["R0"]]),
      DnormDistribution.log_sd.value = om_input_list[[1]][["logR_sd"]]
    ),
    maturity = list(
      LogisticMaturity.inflection_point.value = om_input_list[[1]][["A50.mat"]],
      LogisticMaturity.inflection_point.estimated = FALSE,
      LogisticMaturity.slope.value = om_input_list[[1]][["slope.mat"]],
      LogisticMaturity.slope.estimated = FALSE
    ),
    population = list(
      Population.log_init_naa.value = log(om_output_list[[1]][["N.age"]][1, ])
    )
  )

  parameters_log_r <- default_parameters_log_r |>
    update_parameters(
      modified_parameters = modified_parameters
    )

  fit_log_r <- fit_fims(initialize_fims(parameters = parameters_log_r, data = fims_data), optimize = TRUE)

  # Fit log_devs
  clear()
  fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution",
      LengthComp = "DmultinomDistribution"
    )
  )

  default_parameters_log_devs <- fims_data |>
    setup_default_parameters(
      fleets = list(fleet1 = fleet1, survey1 = survey1),
      recruitment = list(
        form = "BevertonHoltRecruitment"
      ),
      growth = list(form = "EWAAGrowth"),
      maturity = list(form = "LogisticMaturity")
    ) |>
    setup_default_process(
      data = fims_data,
      module = "recruitment",
      par = "log_devs",
      process_distribution = gaussian(),
      estimated = TRUE,
      random = TRUE
    )

  parameters_log_devs <- default_parameters_log_devs |>
    update_parameters(
      modified_parameters = modified_parameters
    )

  fit_log_devs <- fit_fims(initialize_fims(parameters = parameters_log_devs, data = fims_data), optimize = TRUE)

  # Compare log_r and log_devs outputs
  #' @description Test that the `nll_components` from both `fit_log_r` and `fit_log_devs` runs are approximately equal within a tolerance of 0.001.
  expect_equal(fit_log_r@report[["nll_components"]], fit_log_devs@report[["nll_components"]], tolerance = .001)
  #' @description Test that the `expected_recruitment` from both `fit_log_r` and `fit_log_devs` runs are approximately equal within a tolerance of 0.001.
  expect_equal(fit_log_r@report[["expected_recruitment"]], fit_log_devs@report[["expected_recruitment"]], tolerance = .001)
  #' @description Test that the `time_optimization` from `fit_log_r` is less than or equal to that from `fit_log_devs`.
  expect_lte(fit_log_r@run_time[["time_optimization"]], fit_log_devs@run_time[["time_optimization"]])
  #' @description Test that the `time_sdreport` from `fit_log_r` is less than or equal to that from `fit_log_devs`.
  expect_lte(fit_log_r@run_time[["time_sdreport"]], fit_log_devs@run_time[["time_sdreport"]])

  clear()
})
