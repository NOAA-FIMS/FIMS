############################################################################
## Setup data from JABBA for a surplus production model
###########################################################################

# FLCore is from the flr organization on GitHub

# download OM surplus production test data
path_data <- testthat::test_path("fixtures", "integration_test_data_limited.RData")
download.file(
  "https://github.com/iagomosqueira/simtest_SP/raw/main/data/sims.RData",
  path_data
)
# load om object
load(path_data)

# Load om survey indices
# TODO: remove lapply call
om_survey_indices <- lapply(FLCore::FLIndices(A = idx), index)
om_survey_indices_se <- rep(0.2, length(FLCore::FLIndices(A = idx)))
# Load om landings data
om_landings <- FLCore::catch(om)
# HANDLE NAs in catch
om_landings[is.na(om_landings)] <- 0

# converting om data from FLR format to FIMS data format
# type, name, age, length,  datestart,  dateend, value, unit, uncertainty

# use only first iteration of OM data
iter_id <- 1

# using FLR magic to get data out of FLQuant objects
fishing_fleet_landings <- 
  iter(om_landings, iter_id) |> 
   FLCore::window(start = 1951) |>
   as.data.frame() |>
   dplyr::select(age, year, season, area, data) |>
   stats::model.frame(drop = TRUE)

survey_fleet_index <- 
 iter(om_survey_indices, iter_id) |> 
  FLCore::window(start = 1951) |>
  as.data.frame() 
survey_fleet_index$A <- survey_fleet_index$data
survey_fleet_index <- dplyr::select(survey_fleet_index, age, year, season, area, A) 

survey_fleet_index$A <- ifelse(is.na(survey_fleet_index$A), -999, survey_fleet_index$A)
survey_fleet_index <- stats::model.frame(survey_fleet_index, drop = TRUE)

se <- survey_fleet_index
# Assign indx.se
se[, -1] <- as.list(om_survey_indices_se)

# helper to get year to full date
date_start <- paste0(fishing_fleet_landings$year, "-01-01")
date_end <- paste0(fishing_fleet_landings$year, "-12-31")

# create FIMS surplus production data object
data_limited_jabba <- data.frame(
  type = c(
    rep("landings", nrow(fishing_fleet_landings)),
    rep("index", nrow(survey_fleet_index))
  ),
  name = c(
    rep("fleet1", nrow(fishing_fleet_landings)),
    rep("survey1", nrow(survey_fleet_index))
  ),
  age = NA,
  length = NA,
  datestart = rep(as.Date(date_start, format = "%Y-%m-%d"), 2),
  dateend = rep(as.Date(date_end, format = "%Y-%m-%d"), 2),
  value = c(fishing_fleet_landings$data, survey_fleet_index$A),
  unit = rep("t", nrow(fishing_fleet_landings), NA),
  uncertainty = c(rep(0.00999975, nrow(fishing_fleet_landings)), se$A)
)

# save data_sp
usethis::use_data(data_limited_jabba, overwrite = TRUE)

########################################################################
# Setup tuna dataset from Meyer and Millar (1999)
########################################################################


tuna_dat <- rbind(
  c(15.9, 61.89),
  c(25.7, 78.98),
  c(28.5, 55.59),
  c(23.7, 44.61),
  c(25.0, 56.89),
  c(33.3, 38.27),
  c(28.2, 33.84),
  c(19.7, 36.13),
  c(17.5, 41.95),
  c(19.3, 36.63),
  c(21.6, 36.33),
  c(23.1, 38.82),
  c(22.5, 34.32),
  c(22.5, 37.64),
  c(23.6, 34.01),
  c(29.1, 32.16),
  c(14.4, 26.88),
  c(13.2, 36.61),
  c(28.4, 30.07),
  c(34.6, 30.75),
  c(37.5, 23.36),
  c(25.9, 22.36),
  c(25.3, 21.91))
colnames(tuna_dat) <- c('C', 'I')
tuna_dat <- as.data.frame(tuna_dat)
tuna_dat$year <- tuna_dat$timing <- 0
tuna_dat$year <- 1967:1989
tuna_dat$timing <- 1: nrow(tuna_dat)

# helper to get year to full date
date_start <- paste0(tuna_dat$year, "-01-01")
date_end <- paste0(tuna_dat$year, "-12-31")



# create FIMS surplus production data object
data_limited_tuna <- data.frame(
  type = c(
    rep("landings", nrow(tuna_dat)),
    rep("index", nrow(tuna_dat))
  ),
  name = c(
    rep("fleet1", nrow(tuna_dat)),
    rep("survey1", nrow(tuna_dat))
  ),
  age = NA,
  length = NA,
  timing = rep(tuna_dat$timing, 2),
  datestart = rep(as.Date(date_start, format = "%Y-%m-%d"), 2),
  dateend = rep(as.Date(date_end, format = "%Y-%m-%d"), 2),
  value = c(tuna_dat$C, tuna_dat$I),
  unit = rep("t", nrow(tuna_dat), NA),
  uncertainty = rep(0.1, nrow(tuna_dat)*2)
)

# save data_sp
usethis::use_data(data_limited_tuna, overwrite = TRUE)


# Setup results table from Meyer and Millar (1999) Appendix
data_limited_tuna_results <- data.frame(
  label = c("growth_rate", "carrying_capacity", "q", "sigma2_depletion", 
  "sigma2_obs", "depletion", "depletion", "depletion", "depletion", 
  "depletion", "fmsy", "msy", "bmsy", "biomass_pred"),
  mean = c(0.3287, 261.9, 0.2622, 0.003204, 0.0124, 1.019, 0.9945, 0.08735,
  0.3253, NA, NA, NA, NA, NA),
  median = c(0.3121, 255.6, 0.02471, 0.002689, 0.0116, 1.016, 0.9847, 0.8693,
  0.3234, NA, NA, NA, NA, NA)
)

usethis::use_data(data_limited_tuna_results, overwrite = TRUE)