#' Create a simulated data set using OM from model comparison project
#'
#' Use a simulated data set from {ASSAMC} to create an input data set
#' for an age-structured stock assessment model fit using {FIMS}.
#'
#' @details This script only works for a simulation with one fleet and
#' one survey. Additionally, there are many items that need to be looked at,
#' which are referenced with the "TODO" tag.
#' @author Kathryn L. Doering and Kelli F. Johnson
#'
###############################################################################
# Simulate the data
###############################################################################
# Install the {ASSAMC} package, which is not a dependency of {FIMS}
# remotes::install_github(
#   "Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison@feat_eg-data"
# )
library(ASSAMC)
library(dplyr)
returnedom <- ASSAMC::save_om_example()

###############################################################################
# Landings
###############################################################################
landings_data <- data.frame(
# TODO: decide on the term used here for catch/landings.
#       Should there be a type that are not removed but just noted,
#       where obviously in this instance they are removed.
  type = "landings",
  name = names(returnedom[["om_output"]]$L.mt)[1],
  age = NA, # Not by age in this case, but there is a by age option.
# TODO: check that the fishery occurs continuously over the whole year.
  datestart = as.Date(
    paste(returnedom[["om_input"]]$year, 1, 1, sep = "-"),
    format = "%Y-%m-%d"
  ),
  dateend = as.Date(
    paste(returnedom[["om_input"]]$year, 12, 31, sep = "-"),
    format = "%Y-%m-%d"
  ),
  value = returnedom[["om_output"]]$L.mt[[1]], # note only 1 fleet in this case
  unit = "mt", # metric tons
  # TODO: is CV the appropriate input here?
  uncertainty = returnedom[["om_input"]]$cv.L[[1]]
)

###############################################################################
# Survey trend
###############################################################################
trend_data <- data.frame(
# TODO: ensure that "trend" is understood as a type
  type = "trend",
  name = names(returnedom[["om_output"]]$survey_index)[1],
  age = NA, # Not by age in this case, but there is a by age option.
# TODO: check that survey trend occurs instantaneously on Jan. 01
  datestart = as.Date(
    paste(returnedom[["om_input"]]$year, 1, 1, sep = "-"),
    format = "%Y-%m-%d"
  ),
  dateend = as.Date(
    paste(returnedom[["om_input"]]$year, 1, 1, sep = "-"),
    format = "%Y-%m-%d"
  ), # Maybe use NA if instantaneous?
  value = returnedom[["om_output"]]$survey_index[[1]],
# TODO: check the unit of the survey
  unit = "numbers",
  uncertainty = returnedom[["om_input"]]$cv.survey[[1]]
)

###############################################################################
# Age-composition data
###############################################################################
age_comp_data <- rbind(
  data.frame(
    name = names(returnedom[["em_input"]]$n.L),
    returnedom[["em_input"]]$L.age.obs$fleet1,
# TODO: is proportions okay as a unit?
#       What would be other units, e.g., frequency or numbers?
    unit = "proportions",
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
    returnedom[["om_output"]]$survey_age_comp[[1]],
# TODO: check the unit b/c I am not sure why there are decimals
    unit = "numbers",
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
# TODO: decide on type for empirical weight-at-age data.
  type = "weight-at-age",
  name = names(returnedom[["em_input"]]$n.L),
  age = seq_along(returnedom[["om_input"]][["W.kg"]]),
  value = returnedom[["om_input"]][["W.mt"]],
  uncertainty = NA,
  unit = "mt"
)
empiricalweight_data <- merge(timingfishery, weightsfishery)

###############################################################################
# {FIMS} data
###############################################################################
data_mile1 <- type.convert(
  rbind(landings_data, trend_data, age_comp_data, empiricalweight_data),
  as.is = TRUE
)
write.csv(data_mile1,
  file.path("FIMS_input_data.csv"),
  row.names = FALSE
)

# check csv can be read into r well ----
test_read <- read.csv(file.path("FIMS_input_data.csv"))
testthat::expect_equal(test_read, data_mile1)
unlink("FIMS_input_data.csv")

usethis::use_data(data_mile1)
