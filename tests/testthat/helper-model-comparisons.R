# Use the sequence of shell commands to install Wine to run .exe on Ubuntu
# sudo wget -O /etc/apt/keyrings/winehq-archive.key https://dl.winehq.org/wine-builds/winehq.key
# sudo wget -NP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/ubuntu/dists/jammy/winehq-jammy.sources
# sudo apt update
# sudo apt install --install-recommends winehq-stable
model_comparison <- function(){
  library(parallel)
  library(doParallel)
  # Download the EM input files required for the model comparison project, including
  # source code .tpl files for model compilation, input data files, and configuration
  # files.
  # Create the API URL
  api_url <- "https://api.github.com/repos/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison/contents/example/em_input"
  # Make the API request
  response <- httr::GET(api_url)

  # Parse the JSON response into a data frame
  file_list <- jsonlite::fromJSON(httr::content(response, "text"), flatten = TRUE)

  # Create a local directory to save the files
  working_dir <- getwd()
  maindir <- tempdir()

  if (!dir.exists(maindir)) {
    dir.create(maindir)
  }

  local_dir <- file.path(tempdir(), "em_input")
  if (!dir.exists(local_dir)) {
    dir.create(local_dir)
  }

  # Filter for files only (type == "file") and get their download URLs
  file_urls <- subset(file_list, type == "file")[["download_url"]]

  # Loop through the URLs and download each file
  for (url in file_urls) {
    # Define the local path to save the file
    file_name <- basename(url)
    destination_path <- file.path(local_dir, file_name)
    
    # Download the file
    download.file(url, destfile = destination_path, mode = "wb")
  }

  # Download Stock Synthesis 3 executable file
  exe_urls <- c(
    "https://github.com/Bai-Li-NOAA/Model_Comparison_Paper/raw/refs/heads/master/em/em_input_raw/amak.exe",
    "https://github.com/Bai-Li-NOAA/Model_Comparison_Paper/raw/refs/heads/master/em/em_input_raw/ASAP3.exe",
    "https://github.com/Bai-Li-NOAA/Model_Comparison_Paper/raw/refs/heads/master/em/em_input_raw/BAM-Sim_C0.exe",
    "https://github.com/Bai-Li-NOAA/Model_Comparison_Paper/raw/refs/heads/master/em/em_input_raw/BAM-Sim_C8.exe",
    "https://github.com/Bai-Li-NOAA/Model_Comparison_Paper/raw/refs/heads/master/em/em_input_raw/BAM-Sim_C9.exe",
    "https://github.com/Bai-Li-NOAA/Model_Comparison_Paper/raw/refs/heads/master/em/em_input_raw/BAM-Sim_C13.exe",
    "https://github.com/Bai-Li-NOAA/Model_Comparison_Paper/raw/refs/heads/master/em/em_input_raw/ss.exe"
  )

  for (url in exe_urls) {
    # Define the local path to save the file
    file_name <- basename(url)
    destination_path <- file.path(local_dir, file_name)
    
    # Download the file
    download.file(url, destfile = destination_path, mode = "wb")
  }

  # Save the initial OM input using ASSAMC package (sigmaR = 0.4)
  model_input <- ASSAMC::save_initial_input()

  # Configure the input parameters for the simulation
  sim_num <- 100
  sim_input <- ASSAMC::save_initial_input(
    base_case = TRUE,
    input_list = model_input,
    maindir = maindir,
    om_sim_num = sim_num,
    keep_sim_num = sim_num,
    figure_number = 1,
    seed_num = 9924,
    case_name = "sim_data"
  )

  # Run OM and generate om_input, om_output, and em_input
  # using function from the model comparison project
  ASSAMC::run_om(input_list = sim_input)

  sim_input[["om_sim_num"]] <- 1
  sim_input[["keep_sim_num"]] <- 1

  # Run EMs
  ASSAMC::run_em(
    em_names = c("AMAK", "ASAP", "BAM", "SS"),
    # em_names = c("ASAP"),
    input_list = sim_input,
    em_input_filenames = data.frame(
      # ASAP = "C0"
      # AMAK = "C0",
      ASAP = "C0",
      # BAM = "C0",
      SS = "C1"
    )
  )
  # on.exit(unlink(maindir, recursive = TRUE), add = TRUE)
  on.exit(setwd(working_dir), add = TRUE)

  # Load output
  case_dir = file.path(sim_input[["maindir"]], sim_input[["case_name"]])

  # OM
  load(file.path(case_dir, "output", "OM", "OM1.RData"))
  sum_spawning_biomass_om <- sum(om_output[["SSB"]])
  sum_numbers_at_age_om <- sum(om_output[["abundance"]]/1000)
  geom_r0_om <- om_input[["median_R0"]]/1000

  # ASAP
  asap_output <- dget(file.path(case_dir, "output", "ASAP", "s1", "asap3.rdat"))
  sum_spawning_biomass_asap <- sum(asap_output[["SSB"]])
  sum_numbers_at_age_asap <- sum(asap_output[["N.age"]])
  geom_r0_asap <- asap_output[["SR.parms"]][["SR.R0"]]

  # SS3
  ss_output <- r4ss::SS_output(
    dir=file.path(case_dir, "output", "SS", "s1"),
    ncols = 300,
    verbose = F, printstats = F
  )
  sum_spawning_biomass_ss3 <- ss_output$timeseries |>
    dplyr::filter(Era == "TIME") |>
    dplyr::select(SpawnBio) |>
    sum()
  sum_numbers_at_age_ss3 <- ss_output$timeseries |>
    dplyr::filter(Era == "TIME") |>
    dplyr::select(`SmryNum_SX:1_GP:1`) |>
    sum()
  arim_R0_ss3 <- ss_output[["timeseries"]] |>
      dplyr::filter(Era == "INIT") |>
      dplyr::select(Recruit_0)
  arim_SB0_ss3 <- ss_output[["timeseries"]] |>
      dplyr::filter(Era == "INIT") |>
      dplyr::select(SpawnBio)
  arim_h_ss3 <- ss_output[["parameters"]] |>
      dplyr::filter(Label == "SR_BH_steep") |>
      dplyr::select(Value)
  geom_SRparms <- ASSAMC::convertSRparms(
    R0 = arim_R0_ss3,
    h = arim_h_ss3,
    phi = arim_SB0_ss3 / arim_R0_ss3,
    sigmaR = ss_output[["sigma_R_in"]],
    mean2med = TRUE,
    model = sim_input[["SRmodel"]]
  )

  geom_r0_ss3 <- geom_SRparms[["R0BC"]]

  # FIMS
  fims_output <- readRDS(testthat::test_path("fixtures", "fit_age_length_comp.RDS")) 
  fims_estimates <- get_estimates(fims_output)
  sum_spawning_biomass_fims <- fims_estimates |>
    dplyr::filter(label == "spawning_biomass" & year_i %in% 1:30) |>
    dplyr::select(estimated) |>
    sum()
  sum_numbers_at_age_fims <- fims_estimates |>
    dplyr::filter(label == "numbers_at_age" & year_i %in% 1:30) |>
    dplyr::select(estimated) |>
    sum() |>
    (\(x) x / 1000)()
  geom_r0_fims <- fims_estimates |>
    dplyr::filter(label == "log_rzero") |>
    dplyr::select(estimated) |>
    exp() |>
    (\(x) x / 1000)()

  # Relative error
  # Spawning Biomass (SB) errors
  sb_abs_error <- c(
    sum_spawning_biomass_asap - sum_spawning_biomass_om,
    sum_spawning_biomass_ss3 - sum_spawning_biomass_om,
    sum_spawning_biomass_fims - sum_spawning_biomass_om
  )
  sb_rel_error <- sb_abs_error / sum_spawning_biomass_om * 100

  # Numbers at Age (NAA) errors
  naa_abs_error <- c(
    sum_numbers_at_age_asap - sum_numbers_at_age_om,
    sum_numbers_at_age_ss3 - sum_numbers_at_age_om,
    sum_numbers_at_age_fims - sum_numbers_at_age_om
  )
  naa_rel_error <- naa_abs_error / sum_numbers_at_age_om * 100

  error_df <- data.frame(
    Model = c("ASAP", "SS3", "FIMS"),
    # SB_Absolute_Error = sb_abs_error,
    SB_Relative_Error_Pct = round(sb_rel_error, digit = 2),
    # NAA_Absolute_Error = naa_abs_error,
    NAA_Relative_Error_Pct = round(naa_rel_error, digit = 2)
  )

  cat("SB:")
  (sum_spawning_biomass_asap - sum_spawning_biomass_om) / sum_spawning_biomass_om * 100
  (sum_spawning_biomass_ss3 - sum_spawning_biomass_om) / sum_spawning_biomass_om * 100
  (sum_spawning_biomass_fims - sum_spawning_biomass_om) / sum_spawning_biomass_om * 100
  cat("NAA:")
  (sum_numbers_at_age_asap - sum_numbers_at_age_om) / sum_numbers_at_age_om * 100
  (sum_numbers_at_age_ss3 - sum_numbers_at_age_om) / sum_numbers_at_age_om * 100
  (sum_numbers_at_age_fims - sum_numbers_at_age_om) / sum_numbers_at_age_om * 100
  cat("R0:")
  (geom_r0_asap - geom_r0_om) / geom_r0_om * 100
  (geom_r0_ss3 - geom_r0_om) / geom_r0_om * 100
  (geom_r0_fims - geom_r0_om) / geom_r0_om * 100



}

