# Ensure the latest precompiled version of FIMS is installed in R before
# running devtools. To do this, either run:
# - devtools::install() followed by devtools::test(), or
# - devtools::check()

# Run FIMS in serial and parallel
# This test demonstrates how to run the FIMS model in both serial and parallel
# modes. The test compares the execution time and results of running the model
# in serial versus parallel. The parallel execution uses the {snowfall} package
# to parallelize the tasks across multiple CPU cores

# Load the model comparison operating model data from the fixtures folder
load(test_path("fixtures", "integration_test_data.RData"))

# Run the FIMS model in serial and record the execution time
estimation_results_serial <- vector(mode = "list", length = length(om_input_list))

start_time_serial <- Sys.time()
for (i in 1:length(om_input_list)) {
  estimation_results_serial[[i]] <- setup_and_run_FIMS(
    iter_id = i,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = TRUE
  )
}
end_time_serial <- Sys.time()
estimation_time_serial <- end_time_serial - start_time_serial

test_that("Run FIMS in parallel using {snowfall}", {
  core_num <- parallel::detectCores() - 1
  snowfall::sfInit(parallel = TRUE, cpus = core_num)
  start_time_parallel <- Sys.time()

  results_parallel <- snowfall::sfLapply(
    1:length(om_input_list),
    setup_and_run_FIMS,
    om_input_list,
    om_output_list,
    em_input_list,
    TRUE
  )

  end_time_parallel <- Sys.time()

  time_parallel <- end_time_parallel - start_time_parallel

  snowfall::sfStop()

  # Compare execution times: verify that the execution time of the parallel run
  # is less than the serial run.
  expect_lt(object = time_parallel, expected = estimation_time_serial)

  # Compare parameters in results:
  # Verify that the results from both runs are equivalent.
  expect_setequal(
    unname(unlist(lapply(results_parallel, `[[`, "parameters"))),
    unname(unlist(lapply(estimation_results_serial, `[[`, "parameters")))
  )

  # Compare sdr_fixed values in results:
  # Verify that the results from both runs are equivalent.
  expect_setequal(
    unlist(lapply(results_parallel, `[[`, "sdr_fixed")),
    unlist(lapply(estimation_results_serial, `[[`, "sdr_fixed"))
  )

  # Compare sdr_report values in results:
  # Verify that the results from both runs are equivalent.
  expect_setequal(
    unlist(lapply(results_parallel, `[[`, "sdr_report")),
    unlist(lapply(estimation_results_serial, `[[`, "sdr_report"))
  )
})
