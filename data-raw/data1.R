#' Create a simulated data set using OM from model comparison project
#'
#' Use a simulated data set from {ASSAMC} to create an input data set
#' for an age-structured stock assessment model fit using {FIMS}.
#'
#' @details
#' * This script would need to be augmented to accommodate more than
#'   one one fleet or one survey.
#' * Timing:
#'   * Fishery is assumed to operate over the entire year
#'   * Survey occurs instantaneously at the start of the year
#' * Note: If you encounter warning messages such as `cannot open the
#'   connection: warning messages from top-level task callback
#'   'vsc.workspace'` while running the code in VS Code, try calling
#'   `removeTaskCallback("vsc.workspace")` before executing the code.
#'
#' @author Kathryn L. Doering and Kelli F. Johnson
#'
###############################################################################
# Helper functions and load packages
###############################################################################
cv_2_sd <- function(x) {
  sqrt(log(x^2 + 1))
}

check_ASSAMC <- function() {
  packages_all <- .packages(all.available = TRUE)
  if (!"ASSAMC" %in% packages_all) {
    remotes::install_github(
      "NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison"
    )
  }
  library("ASSAMC")
  return(TRUE)
}

check_ASSAMC()
library(dplyr)

###############################################################################
# Simulate the data
###############################################################################
working_dir <- getwd()

main_dir <- tempdir()

# Save the initial OM input using ASSAMC package (sigmaR = 0.4)
model_input <- ASSAMC::save_initial_input()

# Configure the input parameters for the simulation
sim_num <- 100
sim_input <- ASSAMC::save_initial_input(
  base_case = TRUE,
  input_list = model_input,
  maindir = main_dir,
  om_sim_num = sim_num,
  keep_sim_num = sim_num,
  figure_number = 1,
  seed_num = 9924,
  case_name = "sim_data"
)

# Run OM and generate om_input, om_output, and em_input
# using function from the model comparison project
ASSAMC::run_om(input_list = sim_input)

setwd(working_dir)

# Helper function to calculate length at age using the von Bertalanffy growth
# model
# a: current age
# Linf: asymptotic average length
# K: Growth coefficient
# a_0: Theoretical age at size zero
AtoL <- function(a, Linf, K, a_0) {
  L <- Linf * (1 - exp(-K * (a - a_0)))
}

# Initialize lists for operating model (OM) and estimation model (EM) inputs
# and outputs
om_input_list <- om_output_list <- em_input_list <-
  vector(mode = "list", length = sim_num)

# Loop through each simulation to generate length data
for (iter in 1:sim_num) {
  # Load the OM data for the current simulation
  load(file.path(main_dir, "sim_data", "output", "OM", paste0("OM", iter, ".RData")))

  # Extract von Bertalanffy growth model parameters from the OM input
  Linf <- om_input[["Linf"]]
  K <- om_input[["K"]]
  a0 <- om_input[["a0"]]
  amax <- max(om_input[["ages"]])
  # Define coefficient of variation for length-at-age
  cv <- 0.1
  # Extract length-weight coefficient from OM
  L2Wa <- om_input[["a.lw"]]
  # Extract length-weight exponent from OM
  L2Wb <- om_input[["b.lw"]]

  # Extract age bins from the OM input
  ages <- om_input[["ages"]]
  # Define length bins in intervals of 50
  len_bins <- seq(0, 1100, 50)

  # Create length at age conversion matrix and fill proportions using above
  # growth parameters
  age_to_length_conversion <- matrix(
    NA,
    nrow = length(ages),
    ncol = length(len_bins)
  )
  for (age in seq_along(ages)) {
    # Calculate mean length at age to spread lengths around
    mean_length <- AtoL(ages[age], Linf, K, a0)
    # mean_length <- AtoLSchnute(ages[age],L1,L2,a1,a2,Ks)
    # Calculate the cumulative proportion shorter than each composition length
    temp_len_probs <- pnorm(
      q = len_bins,
      mean = mean_length,
      sd = mean_length * cv
    )
    # Reset the first length proportion to zero so the first bin includes all
    # density smaller than that bin
    temp_len_probs[1] <- 0
    # subtract the offset length probabilities to calculate the proportion in
    # each bin. For each length bin the proportion is how many fish are larger
    # than this length but shorter than the next bin length.
    temp_len_probs <- c(temp_len_probs[-1], 1) - temp_len_probs
    age_to_length_conversion[age, ] <- temp_len_probs
  }
  colnames(age_to_length_conversion) <- len_bins
  rownames(age_to_length_conversion) <- ages

  # Loop through each simulation to load the results from the corresponding
  # .RData files
  # Assign the conversion matrix and other information to the OM input
  om_input[["lengths"]] <- len_bins
  om_input[["nlengths"]] <- length(len_bins)
  om_input[["cv.length_at_age"]] <- cv
  om_input[["age_to_length_conversion"]] <- age_to_length_conversion

  om_output[["L.length"]] <- list()
  om_output[["survey_length_comp"]] <- list()
  om_output[["N.length"]] <- matrix(
    0,
    nrow = om_input[["nyr"]],
    ncol = length(len_bins)
  )
  om_output[["L.length"]][["fleet1"]] <- matrix(
    0,
    nrow = om_input[["nyr"]],
    ncol = length(len_bins)
  )
  om_output[["survey_length_comp"]][["survey1"]] <- matrix(
    0,
    nrow = om_input[["nyr"]],
    ncol = length(len_bins)
  )

  em_input[["L.length.obs"]] <- list()
  em_input[["survey.length.obs"]] <- list()
  em_input[["L.length.obs"]][["fleet1"]] <- matrix(
    0,
    nrow = om_input[["nyr"]],
    ncol = length(len_bins)
  )
  em_input[["survey.length.obs"]][["survey1"]] <- matrix(
    0,
    nrow = om_input[["nyr"]],
    ncol = length(len_bins)
  )

  em_input[["lengths"]] <- len_bins
  em_input[["nlengths"]] <- length(len_bins)
  em_input[["cv.length_at_age"]] <- cv
  em_input[["age_to_length_conversion"]] <- age_to_length_conversion
  n_lengthcomps <- 200
  em_input[["n.L.lengthcomp"]][["fleet1"]] <- n_lengthcomps
  em_input[["n.survey.lengthcomp"]][["survey1"]] <- n_lengthcomps

  # Populate length-based outputs for each year, length bin, and age
  for (i in seq_along(om_input[["year"]])) {
    for (j in seq_along(len_bins)) {
      for (k in seq_along(om_input[["ages"]])) {
        # Calculate numbers and landings at length for each fleet and survey
        om_output[["N.length"]][i, j] <- om_output[["N.length"]][i, j] +
          age_to_length_conversion[k, j] *
            om_output[["N.age"]][i, k]

        om_output[["L.length"]][[1]][i, j] <- om_output[["L.length"]][[1]][i, j] +
          age_to_length_conversion[k, j] *
            om_output[["L.age"]][[1]][i, k]

        om_output[["survey_length_comp"]][[1]][i, j] <- om_output[["survey_length_comp"]][[1]][i, j] +
          age_to_length_conversion[k, j] *
            om_output[["survey_age_comp"]][[1]][i, k]

        em_input[["L.length.obs"]][[1]][i, j] <- em_input[["L.length.obs"]][[1]][i, j] +
          age_to_length_conversion[k, j] *
            em_input[["L.age.obs"]][[1]][i, k]

        em_input[["survey.length.obs"]][[1]][i, j] <- em_input[["survey.length.obs"]][[1]][i, j] +
          age_to_length_conversion[k, j] *
            em_input[["survey.age.obs"]][[1]][i, k]
      }
    }
  }

  # Save updated inputs and outputs to file
  save(
    om_input, om_output, em_input,
    file = file.path(
      main_dir,
      "sim_data",
      "output",
      "OM",
      paste0("OM", iter, ".RData")
    )
  )
  # Store inputs and outputs in respective lists
  om_input_list[[iter]] <- om_input
  om_output_list[[iter]] <- om_output
  em_input_list[[iter]] <- em_input
}

# Save all simulations to a single file for {testthat} integration tests
save(
  om_input_list, om_output_list, em_input_list,
  file = testthat::test_path("fixtures", "integration_test_data.RData")
)

# Load a specific simulation for further processing
sim_id <- 1
load(
  file.path(
    main_dir,
    "sim_data",
    "output",
    "OM",
    paste0("OM", sim_id, ".RData")
  )
)

# Return the loaded data
returned_om <- list(
  om_input = om_input,
  om_output = om_output,
  em_input = em_input
)

###############################################################################
# Landings
###############################################################################
landings_data <- data.frame(
  # TODO: Should there be a type that are not removed but just noted,
  #       where obviously in this instance they are removed.
  type = "landings",
  name = names(returned_om[["om_output"]][["L.mt"]])[1],
  age = NA, # Not by age in this case, but there is a by age option.
  timing = returned_om[["om_input"]][["year"]],
  value = returned_om[["em_input"]][["L.obs"]][[1]],
  unit = "mt", # metric tons
  uncertainty = cv_2_sd(returned_om[["em_input"]][["cv.L"]][[1]])
)

###############################################################################
# Survey index
###############################################################################
index_data <- data.frame(
  type = "index",
  name = names(returned_om[["om_output"]][["survey_index"]])[1],
  age = NA, # Not by age in this case, but there is a by age option.
  timing = returned_om[["om_input"]][["year"]],
  value = returned_om[["em_input"]][["surveyB.obs"]][[1]],
  unit = "mt",
  uncertainty = cv_2_sd(returned_om[["em_input"]][["cv.survey"]][[1]])
)

###############################################################################
# Age-composition data
###############################################################################
age_data <- rbind(
  data.frame(
    name = names(returned_om[["em_input"]][["n.L"]]),
    returned_om[["em_input"]][["L.age.obs"]][["fleet1"]],
    unit = "proportion",
    uncertainty = returned_om[["em_input"]][["n.L"]][["fleet1"]],
    timing = returned_om[["om_input"]][["year"]]
  ),
  data.frame(
    name = names(returned_om[["om_output"]][["survey_age_comp"]])[1],
    returned_om[["em_input"]][["survey.age.obs"]][[1]],
    unit = "proportion",
    uncertainty = returned_om[["om_input"]][["n.survey"]][["survey1"]],
    timing = returned_om[["om_input"]][["year"]]
  )
) |>
  dplyr::mutate(
    type = "age_comp"
  ) |>
  tidyr::pivot_longer(
    cols = dplyr::starts_with("X"),
    names_prefix = "X",
    names_to = "age",
    values_to = "value",
    # Convert the "age" column from strings to integers
    names_transform = list(age = as.integer)
  )

###############################################################################
# Weight-at-age data
###############################################################################
timingfishery <- data.frame(
  timing = returned_om[["om_input"]][["year"]]
)
weights_fishery <- data.frame(
  type = "weight-at-age",
  name = names(returned_om[["em_input"]][["n.L"]]),
  age = seq_along(returned_om[["om_input"]][["W.kg"]]),
  value = returned_om[["om_input"]][["W.mt"]],
  uncertainty = NA,
  unit = "mt"
)
weight_at_age_data <- merge(timingfishery, weights_fishery)

###############################################################################
# {FIMS} data
###############################################################################
# Add new column for length values and set to NA for all milestone 1 data
data1 <- rbind(landings_data, index_data, age_data, weight_at_age_data) |>
  dplyr::mutate(
    length = NA,
    .after = "age"
  )

# Extract timing and fleets from milestone 1 data
observers <- c("fleet1", "survey1")

# Create data frame for new fleet and year-specific length at age conversion
# proportions. These are identical across timing and fleets in this default
# example.
length_age_data <- data.frame(
  type = "age-to-length-conversion",
  name = rep(
    sort(rep(observers, length(len_bins) * length(ages))),
    length(timingfishery[["timing"]])
  ),
  age = rep(
    sort(rep(ages, length(len_bins))),
    length(observers) * length(timingfishery[["timing"]])
  ),
  length = rep(
    len_bins,
    length(ages) * length(observers) * length(timingfishery[["timing"]])
  ),
  timing = timingfishery[["timing"]],
  value = rep(
    c(t(returned_om[["em_input"]][["age_to_length_conversion"]])),
    length(observers) * length(timingfishery[["timing"]])
  ),
  unit = "proportion",
  uncertainty = rep(
    c(
      em_input[["n.L.lengthcomp"]][["fleet1"]],
      em_input[["n.survey.lengthcomp"]][["survey1"]]
    ),
    length(len_bins) * length(ages) * length(timingfishery[["timing"]])
  )
)

# Create a length-composition data frame that will be filled by transforming
# the age composition data
length_comp_data <- data.frame(
  type = "length_comp",
  name = sort(rep(observers, length(len_bins) * length(timingfishery[["timing"]]))),
  age = NA,
  length = rep(len_bins, length(timingfishery[["timing"]]) * length(observers)),
  timing = timingfishery[["timing"]],
  value = c(
    c(t(returned_om[["em_input"]][["L.length.obs"]][["fleet1"]])),
    c(t(returned_om[["em_input"]][["survey.length.obs"]][["survey1"]]))
  ),
  unit = "proportion",
  uncertainty = rep(
    c(
      em_input[["n.L.lengthcomp"]][["fleet1"]],
      em_input[["n.survey.lengthcomp"]][["survey1"]]
    ),
    length(len_bins) * length(timingfishery[["timing"]])
  )
)

# Save individual dataframes to a single file for {testthat} integration tests
save(
  landings_data,
  index_data,
  age_data,
  weight_at_age_data,
  length_comp_data,
  length_age_data,
  file = testthat::test_path(
    "fixtures",
    "integration_test_data_components.RData"
  )
)

# Add the conversion matrix and length composition data to dataframe
data1 <- rbind(data1, length_comp_data, length_age_data)

usethis::use_data(data1, overwrite = TRUE)
on.exit(unlink(main_dir, recursive = TRUE), add = TRUE)
on.exit(setwd(working_dir), add = TRUE)
rm(list = ls())
