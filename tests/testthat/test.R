# Load necessary data for the integration test
devtools::load_all()

load(test_path("fixtures", "integration_test_data.RData"))

# Set the iteration ID to 1 for accessing specific input/output list
iter_id <- 1

# Extract model input and output data for the specified iteration
om_input <- om_input_list[[iter_id]]
om_output <- om_output_list[[iter_id]]
em_input <- em_input_list[[iter_id]]

modified_parameters <- readRDS(test_path("fixtures", "parameters_model_comparison_project.RDS"))

fit_age_length_comp <- setup_and_run_FIMS_with_wrappers(
  iter_id = iter_id,
  om_input_list = om_input_list,
  om_output_list = om_output_list,
  em_input_list = em_input_list,
  estimation_mode = TRUE,
  modified_parameters = modified_parameters
)

clear()

# Save FIMS results as a test fixture for additional fimsfit tests
saveRDS(
  fit_age_length_comp,
  file = testthat::test_path("fixtures", "fit_age_length_comp.RDS"),
  compress = FALSE
)

model_fit_path <- test_path("fixtures", "fit_age_length_comp.RDS")
fit <- readRDS(model_fit_path)
# Extract the core TMB components (object, sdreport, optimization result)
# from the fit object.
obj  <- get_obj(fit)
sdreport <- get_sdreport(fit)
opt <- get_opt(fit)
parameter_names <- get_obj(fit)[["par"]] |>
  names()

# Reshape the TMB estimates into a standardized data frame.
# This serves as the "expected" result to compare against.
tmb_estimates <- FIMS:::reshape_tmb_estimates(
  obj = obj,
  sdreport = sdreport,
  opt = opt,
  parameter_names = parameter_names
) |>
  # Select only the columns needed for the comparison to ensure a clean test.
  dplyr::select(
    label
  ) |>
  # Remove any duplicate rows.
  dplyr::distinct() |>
  # Sort the data frame to ensure a consistent order for reliable comparison.
  dplyr::arrange(label) |>
  print(n = 100)

# Extract the finalized_fims, which contains the JSON-like structure.
finalized_fims <- get_finalized_fims(fit)
# Reshape the estimates from the JSON structure into a data frame.
json_estimates <- reshape_json_estimates(finalized_fims) |>
  dplyr::select(label) |>
  dplyr::distinct() |>
  dplyr::arrange(label) |>
  print(n = 100)

dplyr::anti_join(tmb_estimates, json_estimates)
dplyr::anti_join(json_estimates, tmb_estimates)
