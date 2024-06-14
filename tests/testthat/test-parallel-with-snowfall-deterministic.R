# Ensure the latest precompiled version of FIMS is installed in R before
# running devtools. To do this, either run:
# - devtools::install() followed by devtools::test(), or
# - devtools::check()

test_that("Run FIMS in parallel using {snowfall} for deterministic tests work", {

  load(test_path("fixtures", "integration_test_data.RData"))
  load(test_path("fixtures", "fims_serial_data.RData"))

  core_num <- parallel::detectCores() - 1

  snowfall::sfInit(parallel = TRUE, cpus = core_num)
  start_time_parallel <- Sys.time()
  results_parallel <- snowfall::sfLapply(
    1:length(om_input_list),
    setup_and_run_FIMS,
    om_input_list,
    om_output_list,
    em_input_list,
    FALSE)
  end_time_parallel <- Sys.time()
  time_parallel <- end_time_parallel - start_time_parallel
  snowfall::sfStop()

  expect_lt(object = time_parallel, expected = deterministic_time_serial)

  expect_setequal(unname(unlist(lapply(results_parallel, `[[`, "parameters"))),
                  unname(unlist(lapply(deterministic_results_serial, `[[`, "parameters"))))

  expect_setequal(unlist(lapply(results_parallel, `[[`, "sdr_fixed")),
                  unlist(lapply(deterministic_results_serial, `[[`, "sdr_fixed")))

  expect_setequal(unlist(lapply(results_parallel, `[[`, "sdr_report")),
                  unlist(lapply(deterministic_results_serial, `[[`, "sdr_report")))

})
