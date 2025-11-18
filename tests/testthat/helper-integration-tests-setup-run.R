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
    weight_at_age_data
  )
  data_age_comp <- FIMS::FIMSFrame(data_age_comp_raw)
  saveRDS(
    data_age_comp,
    file = testthat::test_path("fixtures", "data_age_comp.RDS"),
    compress = FALSE
  )

  # Generate dataset with only length composition data
  data_length_comp_raw <- rbind(landings_data, index_data, weight_at_age_data) |>
    dplyr::mutate(
      length = NA,
      .after = "age"
    ) |>
    rbind(length_comp_data, length_age_data)
  data_length_comp <- FIMS::FIMSFrame(data_length_comp_raw)
  saveRDS(
    data_length_comp,
    file = testthat::test_path("fixtures", "data_length_comp.RDS"),
    compress = FALSE
  )

  # Missing year for all data sets is year 0002, i.e., yyyy-mm-dd
  na_index <- 2

  # Generate dataset with missing age composition for fleet1
  data_age_comp_na <- data_age_comp_raw |>
    dplyr::filter(!(name == "fleet1" & type == "age_comp" & timing == na_index)) |>
    FIMS::FIMSFrame()
  saveRDS(
    data_age_comp_na,
    file = testthat::test_path("fixtures", "data_age_comp_na.RDS"),
    compress = FALSE
  )

  # Generate dataset with missing length composition, age-to-length-conversion,
  # and index for survey1
  data_length_comp_na <- data_length_comp_raw |>
    dplyr::filter(
      !(name == "survey1" &
        type %in% c("index", "length_comp", "age-to-length-conversion") &
        timing == na_index
      )
    ) |>
    FIMS::FIMSFrame()
  saveRDS(
    data_length_comp_na,
    file = testthat::test_path("fixtures", "data_length_comp_na.RDS"),
    compress = FALSE
  )

  # Generate dataset with missing values in age composition for survey1
  # Missing values for length composition for fleet1 year 0012
  length_na_index <- 2
  data_age_length_comp_raw <- rbind(
    landings_data,
    index_data,
    age_data,
    weight_at_age_data
  )
  data_age_length_comp_na <- data_age_length_comp_raw |>
    dplyr::filter(
      !(name == "survey1" & type %in% c("age_comp") & timing == na_index)
    ) |>
    dplyr::filter(
      !(name == "fleet1" &
        type %in% c("length_comp", "age-to-length-conversion") &
        timing == length_na_index
      )
    ) |>
    FIMS::FIMSFrame()
  saveRDS(
    data_age_length_comp_na,
    file = testthat::test_path("fixtures", "data_age_length_comp_na.RDS"),
    compress = FALSE
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

  data_age_length_comp <- FIMSFrame(data1)
  default_parameters <- create_default_configurations(
    data = data_age_length_comp
  ) |>
    create_default_parameters(
      data = data_age_length_comp
    )

  modified_parameters <- default_parameters |>
    tidyr::unnest(cols = data) |>
    # Update log_Fmort input values for Fleet1
    dplyr::rows_update(
      tibble::tibble(
        fleet_name = "fleet1",
        label = "log_Fmort",
        time = 1:30,
        value = log(om_output_list[[iter_id]][["f"]]),
      ),
      by = c("fleet_name", "label", "time")
    ) |>
    # Update selectivity parameters and log_q for survey1
    dplyr::rows_update(
      tibble::tibble(
        fleet_name = "survey1",
        label = c("inflection_point", "slope", "log_q"),
        value = c(1.5, 2, log(om_output_list[[iter_id]][["survey_q"]][["survey1"]]))
      ),
      by = c("fleet_name", "label")
    ) |>
    # Update log_devs in the Recruitment module (time steps 2–30)
    dplyr::rows_update(
      tibble::tibble(
        label = "log_devs",
        time = 2:30,
        value = om_input_list[[iter_id]][["logR.resid"]][-1],
        estimation_type = "fixed_effects"
      ),
      by = c("label", "time")
    ) |>
    # Update log_sd for log_devs in the Recruitment module
    dplyr::rows_update(
      tibble::tibble(
        module_name = "Recruitment",
        label = "log_sd",
        value = om_input_list[[iter_id]][["logR_sd"]]
      ),
      by = c("module_name", "label")
    ) |>
    # Update inflection point and slope parameters in the Maturity module
    dplyr::rows_update(
      tibble::tibble(
        module_name = "Maturity",
        label = c("inflection_point", "slope"),
        value = c(
          om_input_list[[iter_id]][["A50.mat"]],
          om_input_list[[iter_id]][["slope.mat"]]
        )
      ),
      by = c("module_name", "label")
    ) |>
    # Update log_init_naa values in the Population module
    dplyr::rows_update(
      tibble::tibble(
        label = "log_init_naa",
        age = 1:12,
        value = log(om_output_list[[iter_id]][["N.age"]][1, ])
      ),
      by = c("label", "age")
    )
  saveRDS(
    modified_parameters,
    file = testthat::test_path("fixtures", "parameters_model_comparison_project.RDS"),
    compress = FALSE
  )

  ## Deterministic run with age and length comp using wrappers ----
  # Run FIMS using the setup_and_run_FIMS_with_wrappers function
  deterministic_age_length_comp <- modified_parameters |>
    initialize_fims(data = data_age_length_comp) |>
    fit_fims(optimize = FALSE)
  
  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    deterministic_age_length_comp,
    file = testthat::test_path("fixtures", "deterministic_age_length_comp.RDS"),
    compress = FALSE
  )

  modified_parameters <- modified_parameters |>
    # Update log_sd for fleet1 landings
    dplyr::rows_update(
      tibble::tibble(
        fleet_name = "fleet1",
        label = "log_sd",
        time = 1:30,
        value = log(sqrt(log(0.06^2 + 1)))
      ),
      by = c("fleet_name", "label", "time")
    )

  saveRDS(
    modified_parameters,
    file = testthat::test_path("fixtures", "parameters_model_comparison_project.RDS"),
    compress = FALSE
  )

  ## Estimation run with age and length comp using wrappers ----
  # Run FIMS using the setup_and_run_FIMS_with_wrappers function    
  fit_age_length_comp <- modified_parameters |>
    initialize_fims(data = data_age_length_comp) |>
    fit_fims(optimize = TRUE)
  clear()
  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_age_length_comp,
    file = testthat::test_path("fixtures", "fit_age_length_comp.RDS"),
    compress = FALSE
  )

  ## Estimation run with age comp only using wrappers ----
  # Load test data for age composition from an RDS file
  data_age_comp <- readRDS(test_path("fixtures", "data_age_comp.RDS"))

  # Run FIMS model
  fit_agecomp <- modified_parameters |>
    # remove rows that have module_type == LengthComp
    dplyr::rows_delete(
      y = tibble::tibble(module_type = "LengthComp")
    ) |>
    initialize_fims(data = data_age_comp) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_agecomp,
    file = testthat::test_path("fixtures", "fit_agecomp.RDS"),
    compress = FALSE
  )

  # Load a second dataset that contains missing age composition data
  data_age_comp_na <- readRDS(test_path("fixtures", "data_age_comp_na.RDS"))
  # Fit the FIMS model using the second dataset (with missing values)
  fit_agecomp_na <- modified_parameters |>
    # remove rows that have module_type == LengthComp
    dplyr::rows_delete(
      y = tibble::tibble(module_type = "LengthComp")
    ) |>
    initialize_fims(data = data_age_comp_na) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_agecomp_na,
    file = testthat::test_path("fixtures", "fit_agecomp_na.RDS"),
    compress = FALSE
  )

  ## Estimation run with length comp only using wrappers ----
  # Load test data for length composition from an RDS file
  data_length_comp <- readRDS(test_path("fixtures", "data_length_comp.RDS"))

  # Run FIMS model
  fit_lengthcomp <- modified_parameters |>
    # remove rows that have module_type == AgeComp
    dplyr::rows_delete(
      y = tibble::tibble(module_type = "AgeComp")
    ) |>
    initialize_fims(data = data_length_comp) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_lengthcomp,
    file = testthat::test_path("fixtures", "fit_lengthcomp.RDS"),
    compress = FALSE
  )

  # Load a second dataset that contains missing length composition data
  data_length_comp_na <- readRDS(test_path("fixtures", "data_length_comp_na.RDS"))
  # Fit the FIMS model using the second dataset (with missing values)
  fit_lengthcomp_na <- modified_parameters |>
    # remove rows that have module_type == LengthComp
    dplyr::rows_delete(
      y = tibble::tibble(module_type = "AgeComp")
    ) |>
    initialize_fims(data = data_length_comp_na) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_lengthcomp_na,
    file = testthat::test_path("fixtures", "fit_lengthcomp_na.RDS"),
    compress = FALSE
  )

  ## Estimation run with age and length comp with NAs ----
  # Load test data with both age and length composition data, which contains missing values
  data_age_length_comp_na <- readRDS(test_path("fixtures", "data_age_length_comp_na.RDS"))
  # Define fleet1 and survey1 specifications

  # Run FIMS model with the following steps:
  # * Create default parameters with fleet1 and survey1 specifications
  # * Update parameters if any modifications are provided
  # * Initialize FIMS with the provided data (age and length composition with missing values)
  # * Fit the FIMS model with optimization enabled
  fit_age_length_comp_na <- initialize_fims(
    parameters = modified_parameters,
    data = data_age_length_comp_na
  ) |>
    fit_fims(optimize = TRUE)

  clear()

  # Save FIMS results as a test fixture for additional fimsfit tests
  saveRDS(
    fit_age_length_comp_na,
    file = testthat::test_path("fixtures", "fit_age_length_comp_na.RDS"),
    compress = FALSE
  )
}
