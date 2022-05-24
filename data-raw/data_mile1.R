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
# Simulate the data
###############################################################################
# Install {ASSAMC}, which is not a dependency of {FIMS}
if (!"ASSAMC" %in% installed.packages()[, 1]) {
  remotes::install_github(
    "Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison@feat_eg-data"
  )
}

# Load the necessary packages manually
library(ASSAMC)
library(dplyr)
returnedom <- ASSAMC::save_om_example()

###############################################################################
# Landings
###############################################################################
landings_data <- data.frame(
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
# TODO: discuss if CV the appropriate input here given that landings will be
#       modeled with a lognormal or similar likelihood. Just because previous
#       models have used CV doesn't mean we have to continue to use it.
#       E.g., `dlnorm(sdlog = )` uses a standard deviation on the log scale.
  uncertainty = returnedom[["em_input"]]$cv.L[[1]]
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
  value = returnedom[["em_input"]]$survey.obs[[1]],
  unit = "", # yearly sum of number-at-age / mean(sum of number-at-age)
  uncertainty = returnedom[["em_input"]]$cv.survey[[1]]
)

###############################################################################
# Age-composition data
###############################################################################
age_data <- rbind(
  data.frame(
    name = names(returnedom[["em_input"]]$n.L),
    returnedom[["em_input"]]$L.age.obs$fleet1,
    unit = "", # unit less values
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
    unit = "",
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
) %>%
  dplyr::mutate(
    type = "age"
  ) %>%
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

# check csv can be read into r well
test_read <- read.csv(file.path("FIMS_input_data.csv"))
testthat::expect_equal(test_read, data_mile1)
# unlink("FIMS_input_data.csv")

usethis::use_data(data_mile1, overwrite = TRUE)
rm(
  age_data, landings_data, index_data, weightatage_data,
  timingfishery, weightsfishery,
  data_mile1, returnedom,
  test_read
)

