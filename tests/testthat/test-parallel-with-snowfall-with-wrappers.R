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
estimation_results_serial <- vector(mode = "list", length = sim_num)

for (i in 1:sim_num) {
  estimation_results_serial[[i]] <- setup_and_run_FIMS_with_wrappers(
    iter_id = i,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = TRUE
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
    TRUE
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
