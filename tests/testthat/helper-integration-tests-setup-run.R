# This file generates multiple test datasets for use in tests located in
# tests/testthat.

#' Prepare FIMS input data for integration tests
#'
#' This function prepares the input data for integration tests by generating
#' datasets containing only age composition, only length composition data, or
#' data with missing values. The integration_test_data_components.RData is
#' generated after running the script R/data1.R.
#'
#' @return None. The function saves the generated datasets as RDS files in the
#' specified directory.
#' @examples
#' \dontrun{
#' prepare_test_data()
#' }
prepare_test_data <- function() {
  # Set up FIMS data ----
  # The section generates datasets containing only age composition, only length
  # composition data, or data with missing values.
  # The integration_test_data_components.RData is generated after running the
  # script R/data1.R.
  # Load required integration test data components
  load(test_path("fixtures", "integration_test_data_components.RData"))

  # Generate dataset with only age composition data
  data_age_comp_raw <- rbind(
    landings_data,
    index_data,
    age_data,
    weightatage_data
  )
  data_age_comp <- FIMS::FIMSFrame(data_age_comp_raw)
  saveRDS(
    data_age_comp,
    file = testthat::test_path("fixtures", "data_age_comp.RDS")
  )

  # Generate dataset with only length composition data
  data_length_comp_raw <- rbind(landings_data, index_data, weightatage_data) |>
    dplyr::mutate(
      length = NA,
      .after = "age"
    ) |>
    rbind(length_comp_data, length_age_data)
  data_length_comp <- FIMS::FIMSFrame(data_length_comp_raw)
  saveRDS(
    data_length_comp,
    file = testthat::test_path("fixtures", "data_length_comp.RDS")
  )

  # Missing year for all data sets is year 0002, i.e., yyyy-mm-dd
  na_index <- as.Date("2-01-01")

  # Generate dataset with missing age composition for fleet1
  data_age_comp_na <- data_age_comp_raw |>
    dplyr::filter(!(name == "fleet1" & type == "age" & datestart == na_index)) |>
    FIMS::FIMSFrame()
  saveRDS(
    data_age_comp_na,
    file = testthat::test_path("fixtures", "data_age_comp_na.RDS")
  )

  # Generate dataset with missing length composition, age-to-length-conversion,
  # and index for survey1
  data_length_comp_na <- data_length_comp_raw |>
    dplyr::filter(
      !(name == "survey1" &
        type %in% c("index", "length", "age-to-length-conversion") &
        datestart == na_index
      )
    ) |>
    FIMS::FIMSFrame()
  saveRDS(
    data_length_comp_na,
    file = testthat::test_path("fixtures", "data_length_comp_na.RDS")
  )

  # Generate dataset with missing values in age composition for survey1
  # Missing values for length composition for fleet1 year 0012
  length_na_index <- as.Date("12-01-01")
  data_age_length_comp_raw <- rbind(
    landings_data,
    index_data,
    age_data,
    weightatage_data
  )
  data_age_length_comp_na <- data_age_length_comp_raw |>
    dplyr::filter(
      !(name == "survey1" & type %in% c("age") & datestart == na_index)
    ) |>
    dplyr::filter(
      !(name == "fleet1" &
        type %in% c("length", "age-to-length-conversion") &
        datestart == length_na_index
      )
    ) |>
    FIMS::FIMSFrame()
  saveRDS(
    data_age_length_comp_na,
    file = testthat::test_path("fixtures", "data_age_length_comp_na.RDS")
  )

  # Set up FIMS deterministic and estimation runs results ----
  # This section generates multiple fit results for use in tests located in
  # tests/testthat. For example, it creates fits from FIMS runs with only age
  # composition, only length composition data, or data with missing values.

  # Load necessary data for the integration test
  load(test_path("fixtures", "integration_test_data.RData"))

  # Set the iteration ID to 1 for accessing specific input/output list
  iter_id <- 1

  # Extract model input and output data for the specified iteration
  om_input <- om_input_list[[iter_id]]
  om_output <- om_output_list[[iter_id]]
  em_input <- em_input_list[[iter_id]]

  # Define modified parameters for different modules
  modified_parameters <- vector(mode = "list", length = length(iter_id))
  modified_parameters[[iter_id]] <- list(
    fleet1 = list(
      Fleet.log_Fmort.value = log(om_output_list[[iter_id]][["f"]])
    ),
    survey1 = list(
      LogisticSelectivity.inflection_point.value = 1.5,
      LogisticSelectivity.slope.value = 2,
      Fleet.log_q.value = log(om_output_list[[iter_id]][["survey_q"]][["survey1"]])
    ),
    recruitment = list(
      BevertonHoltRecruitment.log_rzero.value = log(om_input_list[[iter_id]][["R0"]]),
      BevertonHoltRecruitment.log_devs.value = om_input_list[[iter_id]][["logR.resid"]][-1],
      # TODO: integration tests fail after setting BevertonHoltRecruitment.log_devs.estimated
      # to TRUE. We need to debug the issue, then update the line below accordingly.
      BevertonHoltRecruitment.log_devs.estimated = FALSE,
      DnormDistribution.log_sd.value = om_input_list[[iter_id]][["logR_sd"]]
    ),
    maturity = list(
      LogisticMaturity.inflection_point.value = om_input_list[[iter_id]][["A50.mat"]],
      LogisticMaturity.inflection_point.estimated = FALSE,
      LogisticMaturity.slope.value = om_input_list[[iter_id]][["slope.mat"]],
      LogisticMaturity.slope.estimated = FALSE
    ),
    population = list(
      Population.log_init_naa.value = log(om_output_list[[iter_id]][["N.age"]][1, ])
    )
  )
  saveRDS(
    modified_parameters,
    file = testthat::test_path("fixtures", "parameters_model_comparison_project.RDS")
  )

  ## Deterministic run with age and length comp using wrappers ----
  # Run FIMS using the setup_and_run_FIMS_with_wrappers function
  deterministic_age_length_comp <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = FALSE,
    modified_parameters = modified_parameters
  )
  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    deterministic_age_length_comp,
    file = testthat::test_path("fixtures", "deterministic_age_length_comp.RDS")
  )

  ## Estimation run with age and length comp using wrappers ----
  # Run FIMS using the setup_and_run_FIMS_with_wrappers function
  fit_age_length_comp <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = TRUE,
    modified_parameters = modified_parameters
  )

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_age_length_comp,
    file = testthat::test_path("fixtures", "fit_age_length_comp.RDS")
  )

  ## Estimation run with age comp only using wrappers ----
  # Load test data for age composition from an RDS file
  data_age_comp <- readRDS(test_path("fixtures", "data_age_comp.RDS"))

  # Define fleet and survey specifications
  fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Landings = "DlnormDistribution",
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution"
    )
  )

  # Run FIMS model with following steps
  # * Create default parameters with fleet1 and survey1 specifications
  # * Update parameters if any modifications are provided
  # * Initialize FIMS with the provided data and parameters
  # * Fit the FIMS model with optimization enabled
  fit_agecomp <- data_age_comp |>
    create_default_parameters(
      fleets = list(fleet1 = fleet1, survey1 = survey1)
    ) |>
    update_parameters(modified_parameters = modified_parameters[[iter_id]]) |>
    initialize_fims(data = data_age_comp) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_agecomp,
    file = testthat::test_path("fixtures", "fit_agecomp.RDS")
  )

  # Load a second dataset that contains missing age composition data
  data_age_comp_na <- readRDS(test_path("fixtures", "data_age_comp_na.RDS"))
  # Fit the FIMS model using the second dataset (with missing values)
  fit_agecomp_na <- data_age_comp_na |>
    create_default_parameters(
      fleets = list(fleet1 = fleet1, survey1 = survey1)
    ) |>
    update_parameters(modified_parameters = modified_parameters[[iter_id]]) |>
    initialize_fims(data = data_age_comp_na) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_agecomp_na,
    file = testthat::test_path("fixtures", "fit_agecomp_na.RDS")
  )

  ## Estimation run with length comp only using wrappers ----
  # Load test data for length composition from an RDS file
  data_length_comp <- readRDS(test_path("fixtures", "data_length_comp.RDS"))
  # Define fleet1 and survey1 specifications
  fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Landings = "DlnormDistribution",
      Index = "DlnormDistribution",
      LengthComp = "DmultinomDistribution"
    )
  )

  # Run FIMS model with following steps:
  # * Create default parameters with fleet1 and survey1 specifications
  # * Update parameters if any modifications are provided
  # * Initialize FIMS with the provided data and parameters
  # * Fit the FIMS model with optimization enabled
  fit_lengthcomp <- data_length_comp |>
    create_default_parameters(
      fleets = list(fleet1 = fleet1, survey1 = survey1)
    ) |>
    update_parameters(modified_parameters = modified_parameters[[iter_id]]) |>
    initialize_fims(data = data_length_comp) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_lengthcomp,
    file = testthat::test_path("fixtures", "fit_lengthcomp.RDS")
  )

  # Load a second dataset that contains missing length composition data
  data_length_comp_na <- readRDS(test_path("fixtures", "data_length_comp_na.RDS"))
  # Fit the FIMS model using the second dataset (with missing values)
  fit_lengthcomp_na <- data_length_comp_na |>
    create_default_parameters(
      fleets = list(fleet1 = fleet1, survey1 = survey1)
    ) |>
    update_parameters(modified_parameters = modified_parameters[[iter_id]]) |>
    initialize_fims(data = data_length_comp_na) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_lengthcomp_na,
    file = testthat::test_path("fixtures", "fit_lengthcomp_na.RDS")
  )

  ## Estimation run with age and length comp with NAs ----
  # Load test data with both age and length composition data, which contains missing values
  data_age_length_comp_na <- readRDS(test_path("fixtures", "data_age_length_comp_na.RDS"))
  # Define fleet1 and survey1 specifications
  fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Landings = "DlnormDistribution",
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution",
      LengthComp = "DmultinomDistribution"
    )
  )

  # Run FIMS model with the following steps:
  # * Create default parameters with fleet1 and survey1 specifications
  # * Update parameters if any modifications are provided
  # * Initialize FIMS with the provided data (age and length composition with missing values)
  # * Fit the FIMS model with optimization enabled
  fit_age_length_comp_na <- data_age_length_comp_na |>
    create_default_parameters(
      fleets = list(fleet1 = fleet1, survey1 = survey1)
    ) |>
    update_parameters(modified_parameters = modified_parameters[[iter_id]]) |>
    initialize_fims(data = data_age_length_comp_na) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_age_length_comp_na,
    file = testthat::test_path("fixtures", "fit_age_length_comp_na.RDS")
  )
}
