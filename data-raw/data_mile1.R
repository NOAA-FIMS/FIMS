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

maindir <- tempdir()

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

on.exit(unlink(maindir, recursive = TRUE), add = TRUE)

setwd(working_dir)
on.exit(setwd(working_dir), add = TRUE)

load(file.path(maindir, "sim_data", "output", "OM", paste0("OM", 1, ".RData")))
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
data_mile1 <- type.convert(
  rbind(landings_data, index_data, age_data, weightatage_data),
  as.is = TRUE
)
write.csv(data_mile1,
  file.path("FIMS_input_data.csv"),
  row.names = FALSE
)

# check csv can be read into R well
test_read <- read.csv(file.path("FIMS_input_data.csv"))
testthat::expect_equal(test_read, data_mile1)
unlink("FIMS_input_data.csv")

usethis::use_data(data_mile1, overwrite = TRUE)
rm(list = ls())
