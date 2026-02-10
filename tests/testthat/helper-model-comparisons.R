# Use the sequence of shell commands to install Wine to run .exe on Ubuntu
# sudo wget -O /etc/apt/keyrings/winehq-archive.key https://dl.winehq.org/wine-builds/winehq.key
# sudo wget -NP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/ubuntu/dists/jammy/winehq-jammy.sources
# sudo apt update
# sudo apt install --install-recommends winehq-stable
model_comparison <- function(){
  # Use the sequence of shell commands to install Wine to run .exe on Ubuntu
  install_wine_on_linux()
  
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
  # sim_input$r_dev_sum2zero = TRUE

  # Run OM and generate om_input, om_output, and em_input
  # using function from the model comparison project
  ASSAMC::run_om(input_list = sim_input)

  sim_input[["om_sim_num"]] <- 1
  sim_input[["keep_sim_num"]] <- 1

  # Run EMs
  ASSAMC::run_em(
    em_names = c("ASAP", "BAM", "SS", "FIMS", "WHAM"),
    input_list = sim_input,
    em_input_filenames = data.frame(
      ASAP = "C0",
      SS = "C1", 
      BAM = "C0"
    )
  )
  
  # on.exit(unlink(maindir, recursive = TRUE), add = TRUE)
  # on.exit(setwd(working_dir), add = TRUE)

  # Load output
  case_dir <- file.path(sim_input[["maindir"]], sim_input[["case_name"]])

  # OM
  load(file.path(case_dir, "output", "OM", "OM1.RData"))
  sum_spawning_biomass_om <- om_output[["SSB"]] |>
    sum()
  sum_numbers_at_age_om <- (om_output[["abundance"]]/1000) |>
    sum()
  r0_om <- om_input[["median_R0"]]/1000
  rec_dev_om <- om_input[["logR.resid"]]

  # ASAP
  asap_output <- dget(file.path(case_dir, "output", "ASAP", "s1", "asap3.rdat"))
  sum_spawning_biomass_asap <- asap_output[["SSB"]] |>
    sum()
  sum_numbers_at_age_asap <- asap_output[["N.age"]] |>
    sum()
  r0_asap <- asap_output[["SR.parms"]][["SR.R0"]]
  rec_dev_asap <- asap_output[["SR.resids"]][["logR.dev"]]

  # BAM
  bam_output <- dget(file.path(case_dir, "output", "BAM", "s1", "BAM-Sim.rdat"))
  sum_spawning_biomass_bam <- bam_output[["t.series"]][["SSB"]] |>
    sum(na.rm = TRUE)
  sum_numbers_at_age_bam <- (bam_output[["N.age"]]/1000) |>
    head(-1) |>
    sum()
  r0_bam <- bam_output[["parms"]][["R0"]]/1000
  rec_dev_bam <- bam_output[["t.series"]][["logR.dev"]] |>
    head(-1)

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
  r0_ss3 <- ss_output[["timeseries"]] |>
      dplyr::filter(Era == "INIT") |>
      dplyr::select(Recruit_0)
  
  # WHAM
  fit_wham_fixed_effects <- readRDS(file.path(case_dir, "output", "WHAM", "s1", "fit_wham_fixed_effects.RDS"))
  sum_spawning_biomass_wham_fixed_effects <- fit_wham_fixed_effects[["rep"]][["SSB"]] |>
    sum()
  sum_numbers_at_age_wham_fixed_effects <- fit_wham_fixed_effects[["rep"]][["NAA"]][1,1,,] |>
    sum()

  alpha_wham_fixed_effects <- exp(fit_wham_fixed_effects[["rep"]][["log_SR_a"]][1])
  beta_wham_fixed_effects  <- exp(fit_wham_fixed_effects[["rep"]][["log_SR_b"]][1])
  spr0_wham_fixed_effects  <- exp(fit_wham_fixed_effects[["rep"]][["log_SPR0"]][1,1]) 

  r0_wham_fixed_effects <- (alpha_wham_fixed_effects * spr0_wham_fixed_effects - 1) / (beta_wham_fixed_effects * spr0_wham_fixed_effects)
  # r0_wham <- fit_wham_fixed_effects[["rep"]][["N1"]][1,1,1]

  fit_wham_random_effects <- readRDS(file.path(case_dir, "output", "WHAM", "s1", "fit_wham_random_effects.RDS"))
  sum_spawning_biomass_wham_random_effects <- fit_wham_random_effects[["rep"]][["SSB"]] |>
    sum()
  sum_numbers_at_age_wham_random_effects <- fit_wham_random_effects[["rep"]][["NAA"]][1,1,,] |>
    sum()

  alpha_wham_random_effects <- exp(fit_wham_random_effects[["rep"]][["log_SR_a"]][1])
  beta_wham_random_effects  <- exp(fit_wham_random_effects[["rep"]][["log_SR_b"]][1])
  spr0_wham_random_effects  <- exp(fit_wham_random_effects[["rep"]][["log_SPR0"]][1,1]) 

  r0_wham_random_effects <- (alpha_wham_random_effects * spr0_wham_random_effects - 1) / (beta_wham_random_effects * spr0_wham_random_effects)

  h_om <- om_input[["h"]]
  spr0_om <- om_input[["Phi.0"]] * 1000
  alpha_om <- (4 * h_om) / (spr0_om * (1 - h_om))
  beta_om <- (5 * h_om - 1) / ((1 - h_om) * r0_om * spr0_om)

  (alpha_wham_fixed_effects - alpha_om) / alpha_om * 100
  (beta_wham_fixed_effects - beta_om) / beta_om * 100
  (spr0_wham_fixed_effects - spr0_om) / spr0_om * 100

  (alpha_wham_random_effects - alpha_om) / alpha_om * 100
  (beta_wham_random_effects - beta_om) / beta_om * 100
  (spr0_wham_random_effects - spr0_om) / spr0_om * 100

  # FIMS
  fims_output <- readRDS(file.path(case_dir, "output", "FIMS", "s1", "fit_fims.RDS"))
  fims_estimates <- FIMS::get_estimates(fims_output)
  fims_max_gradient <- FIMS::get_max_gradient(fims_output)

  sum_spawning_biomass_fims <- fims_estimates |>
    dplyr::filter(label == "spawning_biomass" & year_i %in% 1:30) |>
    dplyr::select(estimated) |>
    sum()
  sum_numbers_at_age_fims <- fims_estimates |>
    dplyr::filter(label == "numbers_at_age" & year_i %in% 1:30) |>
    dplyr::select(estimated) |>
    sum() |>
    (\(x) x / 1000)()
  r0_fims <- fims_estimates |>
    dplyr::filter(label == "log_rzero") |>
    dplyr::select(estimated) |>
    exp() |>
    (\(x) x / 1000)()
  rec_dev_fims <- fims_estimates |>
    dplyr::filter(label == "log_devs" & year_i %in% 1:30) |>
    dplyr::pull(estimated) |>
    sum()

  # Relative error
  # Spawning Biomass (SB) errors
  sb_abs_error <- c(
    sum_spawning_biomass_asap - sum_spawning_biomass_om,
    sum_spawning_biomass_bam - sum_spawning_biomass_om, 
    sum_spawning_biomass_ss3 - sum_spawning_biomass_om,
    sum_spawning_biomass_wham_fixed_effects - sum_spawning_biomass_om,
    sum_spawning_biomass_wham_random_effects - sum_spawning_biomass_om,
    sum_spawning_biomass_fims - sum_spawning_biomass_om
  )
  sb_rel_error <- sb_abs_error / sum_spawning_biomass_om * 100

  # Numbers at Age (NAA) errors
  naa_abs_error <- c(
    sum_numbers_at_age_asap - sum_numbers_at_age_om,
    sum_numbers_at_age_bam - sum_numbers_at_age_om,
    sum_numbers_at_age_ss3 - sum_numbers_at_age_om,
    sum_numbers_at_age_wham_fixed_effects - sum_numbers_at_age_om,
    sum_numbers_at_age_wham_random_effects - sum_numbers_at_age_om,
    sum_numbers_at_age_fims - sum_numbers_at_age_om
  )
  naa_rel_error <- naa_abs_error / sum_numbers_at_age_om * 100
  
  # R0 errors
  r0_abs_error <- c(
    r0_asap - r0_om,
    r0_bam - r0_om,
    r0_ss3[["Recruit_0"]] - r0_om,
    r0_wham_fixed_effects - r0_om,
    r0_wham_random_effects - r0_om,
    r0_fims[["estimated"]] - r0_om
  )

  r0_rel_error <- r0_abs_error / r0_om * 100

  error_df <- data.frame(
    Model = c("ASAP", "BAM", "SS3", "WHAM_fixed_effects", "WHAM_random_effects", "FIMS"),
    #SB_Absolute_Error = round(sb_abs_error, digit = 2),
    SB_Relative_Error_Pct = round(sb_rel_error, digit = 2),
    #NAA_Absolute_Error = round(naa_abs_error, digit = 2),
    NAA_Relative_Error_Pct = round(naa_rel_error, digit = 2),
    #R0_Absolute_Error = round(r0_abs_error, digit = 2),
    R0_Relative_Error_Pct = round(r0_rel_error, digit = 2)
  )
  knitr::kable(error_df, digits = 2)

  # REs in recruitment devs
  data.frame(
    OM = om_input$logR.resid,
    ASAP = asap_output$SR.resids$logR.dev,
    BAM = bam_output[["t.series"]][["logR.dev"]] |>
      head(-1),
    SS = ss_output$recruit |>
      dplyr::filter(Yr %in% 0:29) |>
      dplyr::pull(dev),
    WHAM_fixed_effects = c(NA, fit_wham_fixed_effects$rep$NAA_devs[1,1,-1,1]),
    WHAM_random_effects = c(NA, fit_wham_random_effects$rep$NAA_devs[1,1,-1,1]),
    FIMS = c(NA, fims_estimates |>
      dplyr::filter(label == "log_devs" & year_i %in% 1:30) |>
      dplyr::pull(estimated))
  ) |>
    knitr::kable(digits = 2)
}

install_wine_on_linux <- function() {
  
  # Check if the operating system is Linux
  if (Sys.info()["sysname"] != "Linux") {
    message("This script is for Linux only. Skipping.")
    return()
  }
  
  # These are the commands to run, in order.
  # They are specific to Ubuntu 22.04 (Jammy).
  commands <- c(
    "sudo wget -O /etc/apt/keyrings/winehq-archive.key https://dl.winehq.org/wine-builds/winehq.key",
    "sudo wget -NP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/ubuntu/dists/jammy/winehq-jammy.sources",
    "sudo dpkg --add-architecture i386",
    "sudo apt update",
    "sudo apt install --install-recommends winehq-stable"
  )
  
  # Loop through each command and run it
  for (cmd in commands) {
    cli::cli_text(paste("Running:", cmd))
    status <- system(cmd) # This runs the command
    
    # Check if the command failed (status != 0)
    if (status != 0) {
      cli::cli_abort("A command failed. Stopping the script.")
      return() # Stop the function
    }
  }
  
  cli::cli_text("All commands finished successfully.")
}
