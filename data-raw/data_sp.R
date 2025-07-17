library(JABBA)
library(tibble)
library(FLCore)

#download OM surplus production test data
path_sp_data <- testthat::test_path("fixtures", "integration_test_data_sp.RData")
download.file(
"https://github.com/iagomosqueira/simtest_SP/raw/main/data/sims.RData",
path_sp_data
)
# remotes::install_github("flr/FLCore")
#load om object
load(path_sp_data)

# Load om survey indices
om_survey_indices <- lapply(FLCore::FLIndices(A = idx), index)
om_survey_indices_se <- rep(0.2, length(FLCore::FLIndices(A = idx)))
# Load om landings data
om_landings <- FLCore::catch(om)
# HANDLE NAs in catch
om_landings[is.na(om_landings)] <- 0

#converting om data from FLR format to FIMS data format
#type, name, age, length,  datestart,  dateend, value, unit, uncertainty

#use only first iteration of OM data 
iter_id <- 1

#using FLR magic to get data out of FLQuant objects
fishing_fleet_landings <- as.data.frame(
    iter(om_landings, iter_id),
    drop = TRUE
  )

survey_fleet_index <- model.frame(
window(iter(om_survey_indices, iter_id), start = 1951),
drop = TRUE
) 
survey_fleet_index$A <- ifelse(is.na(survey_fleet_index$A), -999, survey_fleet_index$A)
se <- survey_fleet_index
# Assign indx.se
se[, -1] <- as.list(om_survey_indices_se)

#helper to get year to full date
date_start <- paste0(fishing_fleet_landings$year, "-01-01")
date_end <- paste0(fishing_fleet_landings$year, "-12-31")

#create FIMS surplus production data object 
data_sp <- data.frame(
    type = c(rep("landings", nrow(fishing_fleet_landings)), 
             rep("index", nrow(survey_fleet_index))),
    name = c(rep("fleet1", nrow(fishing_fleet_landings)), 
             rep("survey1", nrow(survey_fleet_index))),
    age = NA, 
    length = NA, 
    datestart = rep(as.Date(date_start, format = "%Y-%m-%d"),2), 
    dateend = rep(as.Date(date_end, format = "%Y-%m-%d"),2),
    value = c(fishing_fleet_landings$data, survey_fleet_index$A), 
    unit = rep("t", nrow(fishing_fleet_landings), NA), 
    uncertainty = c(rep(0.00999975, nrow(fishing_fleet_landings)), se$A)
)

#save data_sp
usethis::use_data(data_sp, overwrite = TRUE)
