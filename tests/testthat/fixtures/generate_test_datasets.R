# This R script generates multiple test datasets for use in tests located in tests/testthat.
# For example, it creates datasets containing only age composition, only length composition data,
# or data with missing values.
# Note: Run this script only after ensuring that the file tests/testthat/fixtures/integration_test_data_components.RData exists.
# The integration_test_data_components.RData is generated after running the script R/data1.R.

# Load required integration test data components
load(test_path("fixtures", "integration_test_data_components.RData"))

# Generate dataset with only age composition data
# TODO: Check if we need to create a new "length" column with NAs
data_age_comp_raw <- rbind(landings_data, index_data, age_data, weightatage_data)
data_age_comp <- FIMS::FIMSFrame(data_age_comp_raw)
saveRDS(data_age_comp, file = testthat::test_path("fixtures", "data_age_comp.RDS"))

# Generate dataset with only length composition data
data_length_comp_raw <- rbind(landings_data, index_data, weightatage_data) |>
  dplyr::mutate(
    length = NA,
    .after = "age"
  ) |>
  rbind(length_comp_data, length_age_data)
data_length_comp <- FIMS::FIMSFrame(data_length_comp_raw)
saveRDS(data_length_comp, file = testthat::test_path("fixtures", "data_length_comp.RDS"))

# Generate dataset with missing values in age composition for fleet1
na_index <- as.Date("2-01-01")
data_age_comp_na <- data_age_comp_raw |>
  dplyr::filter(!(name == "fleet1" & type == "age" & datestart == na_index)) |>
  FIMS::FIMSFrame()
saveRDS(data_age_comp_na, file = testthat::test_path("fixtures", "data_age_comp_na.RDS"))

# Generate dataset with missing values in length composition, age-to-length-conversion, and index for survey1
data_length_comp_na <- data_length_comp_raw |>
  dplyr::filter(!(name == "survey1" & type %in% c("index", "length", "age-to-length-conversion") & datestart == na_index)) |>
  FIMS::FIMSFrame()
saveRDS(data_length_comp_na, file = testthat::test_path("fixtures", "data_length_comp_na.RDS"))

# Generate dataset with missing values in age composition, length composition for fleets
age_na_index <- as.Date("2-01-01")
length_na_index <- as.Date("12-01-01")
data_age_length_comp_raw <- rbind(landings_data, index_data, age_data, weightatage_data)
data_age_length_comp_na <- data_age_length_comp_raw |>
  dplyr::filter(!(name == "survey1" & type %in% c("age") & datestart == age_na_index)) |>
  dplyr::filter(!(name == "fleet1" & type %in% c("length", "age-to-length-conversion") & datestart == length_na_index)) |>
  FIMS::FIMSFrame()
saveRDS(data_age_length_comp_na, file = testthat::test_path("fixtures", "data_age_length_comp_na.RDS"))
