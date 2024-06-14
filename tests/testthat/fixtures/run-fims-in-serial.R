# Run FIMS in serial
load(test_path("fixtures", "integration_test_data.RData"))

# Run FIMS in serial
deterministic_results_serial <- estimation_results_serial <-
  vector(mode = "list", length = length(om_input_list))

# Deterministic test
start_time_serial <- Sys.time()
for (i in 1:length(om_input_list)) {
  deterministic_results_serial[[i]] <- setup_and_run_FIMS(
    iter_id = i,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = FALSE)
}
end_time_serial <- Sys.time()
deterministic_time_serial <- end_time_serial - start_time_serial

# Estimation test
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

save(deterministic_results_serial,
     deterministic_time_serial,
     estimation_results_serial,
     estimation_time_serial,
     file = test_path("fixtures", "fims_serial_data.RData"))
