# Ensure the latest precompiled version of FIMS is installed in R before
# running devtools. To do this, either run:
# - devtools::install() followed by devtools::test(), or
# - devtools::check()

# Run FIMS in serial and parallel
# This test demonstrates how to run the FIMS model in both serial and parallel
# modes. The parallel execution uses {snowfall} to parallelize the tasks across
# multiple CPU cores.

testthat::skip_on_covr()
# Load the model comparison operating model data from the fixtures folder
load(test_path("fixtures", "integration_test_data.RData"))

sim_num <- 10

# Run the FIMS model in serial and record the execution time
modified_parameters <- estimation_results_serial <- vector(mode = "list", length = sim_num)

for (i in 1:sim_num) {
  # Define modified parameters for different modules
  modified_parameters[[i]] <- list(
    fleet1 = list(
      Fleet.log_Fmort.value = log(om_output_list[[i]][["f"]])
    ),
    survey1 = list(
      LogisticSelectivity.inflection_point.value = 1.5,
      LogisticSelectivity.slope.value = 2,
      Fleet.log_q.value = log(om_output_list[[i]][["survey_q"]][["survey1"]])
    ),
    recruitment = list(
      BevertonHoltRecruitment.log_rzero.value = log(om_input_list[[i]][["R0"]]),
      BevertonHoltRecruitment.log_devs.value = om_input_list[[i]][["logR.resid"]][-1],
      BevertonHoltRecruitment.log_devs.estimated = FALSE,
      DnormDistribution.log_sd.value = om_input_list[[i]][["logR_sd"]]
    ),
    maturity = list(
      LogisticMaturity.inflection_point.value = om_input_list[[i]][["A50.mat"]],
      LogisticMaturity.inflection_point.estimated = FALSE,
      LogisticMaturity.slope.value = om_input_list[[i]][["slope.mat"]],
      LogisticMaturity.slope.estimated = FALSE
    ),
    population = list(
      Population.log_init_naa.value = log(om_output_list[[i]][["N.age"]][1, ])
    )
  )

  estimation_results_serial[[i]] <- setup_and_run_FIMS_with_wrappers(
    iter_id = i,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = TRUE, 
    modified_parameters = modified_parameters
  )
}

test_that("Run FIMS in parallel using {snowfall}", {
  core_num <- 2
  snowfall::sfInit(parallel = TRUE, cpus = core_num)

  snowfall::sfLibrary(FIMS)
  results_parallel <- snowfall::sfLapply(
    1:sim_num,
    setup_and_run_FIMS_with_wrappers,
    om_input_list,
    om_output_list,
    em_input_list,
    TRUE,
    modified_parameters
  )

  snowfall::sfStop()

  # Comparison of results:
  # Verify that SSB values from both runs are equivalent.
  expect_setequal(
    purrr::map(
      results_parallel,
      \(x) x@estimates[x@estimates$name == "SSB", "value"]
    ),
    purrr::map(
      estimation_results_serial,
      \(x) x@estimates[x@estimates$name == "SSB", "value"]
    )
  )

  # Verify that parameter values from both runs are equivalent.
  expect_setequal(
    purrr::map(
      results_parallel,
      \(x) x@estimates[x@estimates$name == "p", "value"]
    ),
    purrr::map(
      estimation_results_serial,
      \(x) x@estimates[x@estimates$name == "p", "value"]
    )
  )

  # Verify that total NLL values from both runs are equivalent.
  expect_equal(
    purrr::map(results_parallel, \(x) x@report[["jnll"]]),
    purrr::map(estimation_results_serial, \(x) x@report[["jnll"]])
  )
})
