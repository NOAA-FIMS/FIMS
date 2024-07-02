# Run FIMS in serial
load(test_path("fixtures", "integration_test_data.RData"))

estimation_results_serial <-
  vector(mode = "list", length = length(om_input_list))

start_time_serial <- Sys.time()
for (i in 1:length(om_input_list)) {
  estimation_results_serial[[i]] <- setup_and_run_FIMS(
    iter_id = i,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = TRUE)
}
end_time_serial <- Sys.time()
estimation_time_serial <- end_time_serial - start_time_serial

# Ensure the latest precompiled version of FIMS is installed in R before
# running devtools. To do this, either run:
# - devtools::install() followed by devtools::test(), or
# - devtools::check()

test_that("Run FIMS in parallel using {snowfall} and assign predefined
          running IDs to each core works", {

  load(test_path("fixtures", "integration_test_data.RData"))

  core_num <- parallel::detectCores() - 1

  segment_num <- length(om_input_list) / core_num
  id_begin <- id_end <- vector(length = core_num)

  for (i in 1:core_num) {
    id_begin[i] <- as.integer((i - 1) * segment_num + 1)
    id_end[i] <- as.integer(i * segment_num)
  }

  snowfall::sfInit(parallel = TRUE, cpus = core_num)
  start_time_parallel <- Sys.time()
  results_by_core <- snowfall::sfLapply(
    1:core_num,
    setup_and_run_FIMS_by_core,
    om_input_list,
    om_output_list,
    em_input_list,
    id_begin, id_end
  )
  end_time_parallel <- Sys.time()
  time_parallel <- end_time_parallel - start_time_parallel
  snowfall::sfStop()

  results_parallel <- do.call(c, results_by_core)

  expect_lt(object = time_parallel, expected = estimation_time_serial)

  expect_setequal(unlist(lapply(results_parallel, `[[`, "sdr_report")),
                  unlist(lapply(estimation_results_serial, `[[`, "sdr_report")))
})

test_that("Run FIMS in parallel using {snowfall} without assigning predefined
          running IDs to each core works", {

  load(test_path("fixtures", "integration_test_data.RData"))

  core_num <- parallel::detectCores() - 1

  snowfall::sfInit(parallel = TRUE, cpus = core_num)
  start_time_parallel <- Sys.time()
  results_parallel <- snowfall::sfLapply(
    1:length(om_input_list),
    setup_and_run_FIMS,
    om_input_list,
    om_output_list,
    em_input_list,
    TRUE)
  end_time_parallel <- Sys.time()
  time_parallel <- end_time_parallel - start_time_parallel
  snowfall::sfStop()

  expect_lt(object = time_parallel, expected = estimation_time_serial)

  expect_setequal(unname(unlist(lapply(results_parallel, `[[`, "parameters"))),
                  unname(unlist(lapply(estimation_results_serial, `[[`, "parameters"))))

  expect_setequal(unlist(lapply(results_parallel, `[[`, "sdr_fixed")),
                  unlist(lapply(estimation_results_serial, `[[`, "sdr_fixed")))

  expect_setequal(unlist(lapply(results_parallel, `[[`, "sdr_report")),
                  unlist(lapply(estimation_results_serial, `[[`, "sdr_report")))

})
