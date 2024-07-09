# Install the operating model repo from GitHub
remotes::install_github(
  repo = "Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison"
)

working_dir <- getwd()

maindir <- tempdir()

# Save the initial OM input using ASSAMC package (sigmaR = 0.4)
model_input <- ASSAMC::save_initial_input()

# Configure the input parameters for the simulation
sim_num <- 100
FIMS_100iter <- ASSAMC::save_initial_input(
  base_case = TRUE,
  input_list = model_input,
  maindir = maindir,
  om_sim_num = sim_num,
  keep_sim_num = sim_num,
  figure_number = 1,
  seed_num = 9924,
  case_name = "FIMS_100iter"
)

# Run OM and generate om_input, om_output, and em_input
# using function from the model comparison project
ASSAMC::run_om(input_list = FIMS_100iter)

on.exit(unlink(maindir, recursive = TRUE), add = TRUE)

setwd(working_dir)
on.exit(setwd(working_dir), add = TRUE)

# Loop through each simulation to load the results from the corresponding
# .RData files and save them into one file
om_input_list <- om_output_list <- em_input_list <-
  vector(mode = "list", length = sim_num)
for (i in 1:sim_num) {
  load(file.path(maindir, "FIMS_100iter", "output", "OM", paste0("OM", i, ".RData")))
  om_input_list[[i]] <- om_input
  om_output_list[[i]] <- om_output
  em_input_list[[i]] <- em_input
}

save(om_input_list, om_output_list, em_input_list,
  file = test_path("fixtures", "integration_test_data.RData")
)
