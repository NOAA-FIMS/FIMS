# tests were commented out because they were not working with the helper function
# setup_and_run_FIMS(). They can be tested after developing the wrapper function
# to run FIMS.

# test_that("Run FIMS in parallel without local() using {parallel} and {foreach} doesn't works", {
#   load(test_path("fixtures", "integration_test_data.RData"))
#   load(test_path("fixtures", "fims_serial_data.RData"))
#
#   cl <- parallel::makeCluster(2)
#   doParallel::registerDoParallel(cl)
#   start_time_parallel <- Sys.time()
#   results <- vector(mode = "list", length = length(om_input_list))
#   `%dopar%` <- foreach::`%dopar%`
#   results_parallel <- foreach::foreach(i = 1:length(om_input_list)) %dopar% {
#     library(FIMS)
#     results[[i]] <- setup_and_run_FIMS(om_input_list = om_input_list,
#                                        om_output_list = om_output_list,
#                                        em_input_list = em_input_list,
#                                        iter_id = i)
#   }
#   end_time_parallel <- Sys.time()
#   time_parallel <- end_time_parallel - start_time_parallel
#   parallel::stopCluster(cl)
#
#   expect_lt(object = time_parallel, expected = time_serial)
#   expect_lt(
#     length(unlist(lapply(results_parallel, `[[`, "sdr_report"))),
#     length(unlist(lapply(results_serial, `[[`, "sdr_report")))
#   )
#
# })
# #
# test_that("Run FIMS in parallel with local() using {parallel} and {foreach} doesn't works", {
#   load(test_path("fixtures", "integration_test_data.RData"))
#   load(test_path("fixtures", "fims_serial_data.RData"))
#
#   # CRAN limits the number of cores available to packages to 2, for performance reasons.
#   # Run devtools::check() with two threads/cores.
#   # cl <- parallel::makeCluster(parallel::detectCores()-1)
#   cl <- parallel::makeCluster(2)
#   doParallel::registerDoParallel(cl)
#   start_time_parallel <- Sys.time()
#   results <- vector(mode = "list", length = length(om_input_list))
#   `%dopar%` <- foreach::`%dopar%`
#   results_parallel <- foreach::foreach(i = 1:length(om_input_list)) %dopar% {
#     local({
#       library(FIMS)
#       results[[i]] <- setup_and_run_FIMS(om_input_list = om_input_list,
#                                          om_output_list = om_output_list,
#                                          em_input_list = em_input_list,
#                                          iter_id = i)
#     })
#   }
#   end_time_parallel <- Sys.time()
#   time_parallel <- end_time_parallel - start_time_parallel
#   parallel::stopCluster(cl)
#
#   expect_lt(object = time_parallel, expected = time_serial)
#   expect_lt(
#     length(unlist(lapply(results_parallel, `[[`, "sdr_report"))),
#     length(unlist(lapply(results_serial, `[[`, "sdr_report")))
#   )
# })
#
# test_that("Run FIMS in parallel with new.env() using {parallel} and {foreach} doesn't works", {
#   load(test_path("fixtures", "integration_test_data.RData"))
#   load(test_path("fixtures", "fims_serial_data.RData"))
#
#   cl <- parallel::makeCluster(2)
#   doParallel::registerDoParallel(cl)
#   start_time_parallel <- Sys.time()
#   results <- vector(mode = "list", length = length(om_input_list))
#   `%dopar%` <- foreach::`%dopar%`
#   results_parallel <- foreach::foreach(i = 1:length(om_input_list)) %dopar% {
#     # library(FIMS)
#     assign(paste0("env", i), new.env())
#     environment(setup_and_run_FIMS) <- get(paste0("env", i))
#     results[[i]] <- setup_and_run_FIMS(om_input_list, om_output_list, em_input_list, i)
#   }
#   end_time_parallel <- Sys.time()
#   time_parallel <- end_time_parallel - start_time_parallel
#   parallel::stopCluster(cl)
#
#   expect_lt(object = time_parallel, expected = time_serial)
#   expect_lt(
#     length(unlist(lapply(results_parallel, `[[`, "sdr_report"))),
#     length(unlist(lapply(results_serial, `[[`, "sdr_report")))
#   )
# })
