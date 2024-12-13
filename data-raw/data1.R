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
      "Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison"
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

load(file.path(main_dir, "sim_data", "output", "OM", paste0("OM", 1, ".RData")))
returnedom <- list(
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
  name = names(returnedom[["om_output"]]$L.mt)[1],
  age = NA, # Not by age in this case, but there is a by age option.
  datestart = as.Date(
    paste(returnedom[["om_input"]]$year, 1, 1, sep = "-"),
    format = "%Y-%m-%d"
  ),
  dateend = as.Date(
    paste(returnedom[["om_input"]]$year, 12, 31, sep = "-"),
    format = "%Y-%m-%d"
  ),
  value = returnedom[["em_input"]]$L.obs[[1]],
  unit = "mt", # metric tons
  uncertainty = cv_2_sd(returnedom[["em_input"]]$cv.L[[1]])
)

###############################################################################
# Survey index
###############################################################################
index_data <- data.frame(
  type = "index",
  name = names(returnedom[["om_output"]]$survey_index)[1],
  age = NA, # Not by age in this case, but there is a by age option.
  datestart = as.Date(
    paste(returnedom[["om_input"]]$year, 1, 1, sep = "-"),
    format = "%Y-%m-%d"
  ),
  dateend = as.Date(
    paste(returnedom[["om_input"]]$year, 1, 1, sep = "-"),
    format = "%Y-%m-%d"
  ),
  value = returnedom[["em_input"]]$surveyB.obs[[1]],
  unit = "mt",
  uncertainty = cv_2_sd(returnedom[["em_input"]]$cv.survey[[1]])
)

###############################################################################
# Age-composition data
###############################################################################
age_data <- rbind(
  data.frame(
    name = names(returnedom[["em_input"]]$n.L),
    returnedom[["em_input"]]$L.age.obs$fleet1,
    unit = "proportion",
    uncertainty = returnedom[["em_input"]]$n.L$fleet1,
    datestart = as.Date(
      paste(returnedom[["om_input"]][["year"]], 1, 1, sep = "-"),
      "%Y-%m-%d"
    ),
    dateend = as.Date(
      paste(returnedom[["om_input"]][["year"]], 12, 31, sep = "-"),
      "%Y-%m-%d"
    )
  ),
  data.frame(
    name = names(returnedom[["om_output"]]$survey_age_comp)[1],
    returnedom[["em_input"]]$survey.age.obs[[1]],
    unit = "number of fish in proportion",
    uncertainty = returnedom[["om_input"]][["n.survey"]][["survey1"]],
    datestart = as.Date(
      paste(returnedom[["om_input"]][["year"]], 1, 1, sep = "-"),
      "%Y-%m-%d"
    ),
    dateend = as.Date(
      paste(returnedom[["om_input"]][["year"]], 1, 1, sep = "-"),
      "%Y-%m-%d"
    )
  )
) |>
  dplyr::mutate(
    type = "age"
  ) |>
  tidyr::pivot_longer(
    cols = dplyr::starts_with("X"),
    names_prefix = "X",
    names_to = "age",
    values_to = "value"
  )

###############################################################################
# Weight-at-age data
###############################################################################
timingfishery <- data.frame(
  datestart = as.Date(
    paste(returnedom[["om_input"]][["year"]], 1, 1, sep = "-"),
    "%Y-%m-%d"
  ),
  dateend = as.Date(
    paste(returnedom[["om_input"]][["year"]], 12, 31, sep = "-"),
    "%Y-%m-%d"
  )
)
weightsfishery <- data.frame(
  type = "weight-at-age",
  name = names(returnedom[["em_input"]]$n.L),
  age = seq_along(returnedom[["om_input"]][["W.kg"]]),
  value = returnedom[["om_input"]][["W.mt"]],
  uncertainty = NA,
  unit = "mt"
)
weightatage_data <- merge(timingfishery, weightsfishery)

###############################################################################
# {FIMS} data
###############################################################################
data1 <- type.convert(
  rbind(landings_data, index_data, age_data, weightatage_data),
  as.is = TRUE
)

# Add new column for length values and set to NA for all milestone 1 data
data1 <- data1[, c(1:3, 3:8)]
colnames(data1)[4] <- "length"
data1[, 4] <- NA

# Growth function values to create length age conversion matrix from model
# comparison project
Linf <- 800
K <- 0.18
a0 <- -1.36
amax <- 12
cv <- 0.1

L2Wa <- 2.5e-08
L2Wb <- 3

AtoL <- function(a, Linf, K, a_0) {
  L <- Linf * (1 - exp(-K * (a - a_0)))
}

ages <- 1:amax
len_bins <- seq(0, 1100, 50)

# Create length at age conversion matrix and fill proportions using above
# growth parameters
length_age_conversion <- matrix(NA, nrow = length(ages), ncol = length(len_bins))
for (i in seq_along(ages)) {
  # Calculate mean length at age to spread lengths around
  mean_length <- AtoL(ages[i], Linf, K, a0)
  # mean_length <- AtoLSchnute(ages[i],L1,L2,a1,a2,Ks)
  # Calculate the cumulative proportion shorter than each composition length
  temp_len_probs <- pnorm(q = len_bins, mean = mean_length, sd = mean_length * cv)
  # Reset the first length proportion to zero so the first bin includes all
  # density smaller than that bin
  temp_len_probs[1] <- 0
  # subtract the offset length probabilities to calculate the proportion in each
  # bin. For each length bin the proportion is how many fish are larger than this
  # length but shorter than the next bin length.
  temp_len_probs <- c(temp_len_probs[-1], 1) - temp_len_probs
  length_age_conversion[i, ] <- temp_len_probs
}
colnames(length_age_conversion) <- len_bins
rownames(length_age_conversion) <- ages

# Extract years and fleets from milestone 1 data
start_date <- unique(data1$datestart[data1$type == "weight-at-age"])
end_date <- unique(data1$dateend[data1$type == "weight-at-age"])
observers <- unique(data1$name[data1$type == "age"])

# Create data frame for new fleet and year specific length at age conversion proportions
# These are identical across years and fleets in this default example
# length_age_data <- data1[rep(1, length(ages) *
#   length(len_bins) *
#   length(start_date) *
#   length(observers)), ]
length_age_data <- data.frame(
  matrix(NA, ncol = ncol(data1), nrow = length(ages) *
           length(len_bins) *
           length(start_date) *
           length(observers))
)
colnames(length_age_data) <- colnames(data1)
row_index <- 1
for (k in seq_along(start_date)) {
  for (l in seq_along(observers)) {
    for (i in seq_along(ages)) {
      for (j in seq_along(len_bins)) {
        length_age_data[row_index, ] <- c(
          "age-to-length-conversion",
          observers[l],
          ages[i],
          len_bins[j],
          start_date[k],
          end_date[k],
          length_age_conversion[i, j],
          "proportion",
          200
        )
        row_index <- row_index + 1
      }
    }
  }
}

# Create a length compostion data frame that will be filled by transforming
# the age composition data
# length_comp_data <- data1[rep(1, length(len_bins) *
#   length(start_date) *
#   length(observers)), ]

length_comp_data <- data.frame(
  matrix(NA, ncol = ncol(data1), nrow = length(len_bins) *
           length(start_date) *
           length(observers))
)
colnames(length_comp_data) <- colnames(data1)

age_comp_data <- data1[data1$type == "age", ]

row_index <- 1
for (k in seq_along(start_date)) {
  year_sub <- age_comp_data[age_comp_data$datestart == start_date[k], ]
  for (l in seq_along(observers)) {
    obs_sub <- year_sub[year_sub$name == observers[l], ]
    for (j in seq_along(len_bins)) {
      temp_len_props <- length_age_conversion[, j]
      temp_len_prob <- sum(temp_len_props * obs_sub$value)
      length_comp_data[row_index, ] <- c(
        "length",
        observers[l],
        NA,
        len_bins[j],
        start_date[k],
        end_date[k],
        temp_len_prob,
        "proportion",
        200
      )
      row_index <- row_index + 1
    }
  }
}

# Add the conversion matrix and length composition data to dataframe
# data1 <- rbind(data1,length_comp_data,length_age_data)

data1 <- type.convert(
  rbind(data1, length_comp_data, length_age_data),
  as.is = TRUE
)

write.csv(data1,
          file.path("FIMS_input_data.csv"),
          row.names = FALSE
)

# check csv can be read into R well
test_read <- read.csv(file.path("FIMS_input_data.csv"))
testthat::expect_equal(test_read, data1)
unlink("FIMS_input_data.csv")

usethis::use_data(data1, overwrite = TRUE)
on.exit(unlink(main_dir, recursive = TRUE), add = TRUE)
on.exit(setwd(working_dir), add = TRUE)
rm(list = ls())
