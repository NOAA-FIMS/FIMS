# The general structure for each `load_*` function is a "load or create" pattern:
# - It defines a path to a fixture file (.RDS).
# - It checks if the file exists.
# - If the file does not exist, it generates the necessary data object (e.g.,
#    a FIMSFrame, a parameter tibble, or a FIMS fit object) and saves it to the
#    path.
# - It then reads and returns the object from the .RDS file.
# This ensures that test data is generated only once and is readily available for
# subsequent test runs, speeding up the process.
# The function uses tryCatch() to handle potential errors during file operations.
# If an error occurs, it provides a clear message about the failure and suggests
# checking the relevant code logic. This helps in debugging issues related to
# file creation or loading.

# To regenerate all .RDS fixture files in tests/testthat/fixtures,
# delete the existing .RDS files and run the following command in the R console:
# FIMS:::remove_test_data(); load_all_data(); load_all_fits()

# When adding new data for testing, please add a corresponding `load_data_*()`
# function to this script and call it in `load_all_data()`.
# When adding a new fitted model for testing, please add a corresponding
# `load_fit_*()` function to this script and call it in `load_all_fits()`.

#' Load or Create Parameters for Model Comparison Project
#'
#' This function loads a pre-saved set of modified parameters for integration tests.
#' If the parameter file does not exist, it generates them by starting with default
#' configurations and then updating specific values (e.g., mortality, selectivity,
#' recruitment deviations) using data from a saved operating model output.
#' The resulting parameter set is then saved as an .RDS file for future use.
#'
#' @return A tibble containing the modified model parameters for testing.
load_parameters_model_comparison_project <- function() {
  # Use tryCatch to handle potential errors during file operations. If an error
  # occurs, it provides a clear message about the failure and suggests checking
  # the relevant code logic. This helps in debugging issues related to file creation
  # or loading.
  tryCatch({
    path <- testthat::test_path("fixtures", "parameters_model_comparison_project.RDS")
  
    if (!file.exists(path)) {
      # Load data1 for the integration test
      data_age_length_comp <- FIMSFrame(data1)

      # Create default parameters
      default_parameters <- create_default_configurations(
        data = data_age_length_comp
      ) |>
        create_default_parameters(
          data = data_age_length_comp
        )

      # Load integration test data from the OM
      # Load necessary data for the integration test
      load(test_path("fixtures", "integration_test_data.RData"))
      
      # Set the iteration ID to 1 for accessing specific input/output list
      iter_id <- 1

      modified_parameters <- default_parameters |>
        tidyr::unnest(cols = data) |>
        # Update log_Fmort initial values for Fleet1
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
            # TODO: integration tests fail after setting recruitment log_devs all estimable.
            # We need to debug the issue, then change constant to fixed_effects.
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
        file = path,
        compress = FALSE
      )
    }

    readRDS(path)
  }, error = function(e) {
    cli::cli_abort(
      c(
        "x" = "Failed to create or load {.val {path}}.",
        "i" = "Please check the code logic in `load_parameters_model_comparison_project()`.",
        "i" = "File to check: `tests/testthat/helper-integration-tests-setup-run.R`.",
        "!" = "Original error: {.emph {e$message}}"
      ),
      call = NULL
    )
  })
}

#' Load or Create Age Composition Data Fixture
#'
#' This function loads a FIMSFrame object containing age composition data.
#' If the data file does not exist, it creates one from raw data components
#' and saves it as an .RDS file for use in tests.
#'
#' @param with_na A logical value. If `TRUE`, it generates and loads a dataset
#'   where one year of age composition data is missing. Defaults to `FALSE`.
#' @return A `FIMSFrame` object with age composition data.
load_data_age_comp <- function(with_na = FALSE) {
  tryCatch({
    if (with_na) {
      path <- testthat::test_path("fixtures", "data_age_comp_na.RDS")
    } else {
      path <- testthat::test_path("fixtures", "data_age_comps.RDS")
    }
    
    if (!file.exists(path)) {
      load(test_path("fixtures", "integration_test_data_components.RData"))
      
      # Generate dataset with only age composition data
      data_age_comp_raw <- rbind(
        landings_data,
        index_data,
        age_data,
        weight_at_age_data
      )

      if (with_na) {
        na_index <- as.Date("2-01-01")
        data_age_comp_raw <- data_age_comp_raw |>
          dplyr::filter(!(name == "fleet1" & type == "age_comp" & datestart == na_index))
      }

      fims_data <- FIMS::FIMSFrame(data_age_comp_raw)
      saveRDS(fims_data, file = path)
    }
    readRDS(path)
  }, error = function(e) {
    cli::cli_abort(
      c(
        "x" = "Failed to create or load {.val {path}}.",
        "i" = "Please check the code logic in `load_data_age_comp()`.",
        "i" = "File to check: `tests/testthat/helper-integration-tests-setup-run.R`.",
        "!" = "Original error: {.emph {e$message}}"
      ),
      .call = NULL
    )
  })
}

#' Load or Create a Fitted Model with Age Composition Data
#'
#' This function loads a saved object that was fitted using only age
#' composition data. If the file does not exist, it runs the model fitting
#' process and saves the result as an .RDS fixture for future tests.
#'
#' @param with_na A logical value passed to `load_data_age_comp` to specify
#'   whether to use data with missing values. Defaults to `FALSE`.
#' @return An object containing the model results.
load_fit_age_comp <- function(with_na = FALSE) {
  tryCatch({
    if (with_na) {
      path <- testthat::test_path("fixtures", "fit_age_comp_na.RDS")
    } else {
      path <- testthat::test_path("fixtures", "fit_age_comp.RDS")
    }

    if (!file.exists(path)) {
      data_age_comp <- load_data_age_comp(with_na = with_na)

      parameters <- load_parameters_model_comparison_project() |>
        # TODO: delete the code below when log_devs estimation error fixed
        dplyr::mutate(
          estimation_type = dplyr::if_else(
            label == "log_devs" & module_type == "BevertonHolt",
            "constant",
            estimation_type
          )
        )
      fit_age_comp <- parameters |>
        # remove rows that have module_type == LengthComp
        dplyr::rows_delete(
          y = tibble::tibble(module_type = "LengthComp")
        ) |>
        initialize_fims(data = data_age_comp) |>
        fit_fims(optimize = TRUE)
      clear()

      # Save FIMS results as a test fixture for additional fimsfit tests
      saveRDS(
        fit_age_comp,
        file = path,
        compress = FALSE
      )
    }
    readRDS(path)
  }, error = function(e) {
    cli::cli_abort(
      c(
        "x" = "Failed to create or load {.val {path}}.",
        "i" = "Please check the code logic in the `load_fit_age_comp()`.",
        "i" = "File to check: `tests/testthat/helper-integration-tests-setup-run.R`.",
        "i" = "Other functions in the same file that may be relevant include:
          1) `load_data_age_comp()` for loading or creating age composition data;
          2) `load_parameters_model_comparison_project()` for loading or creating
          a parameters tibble",
        "!" = "Original error: {.emph {e$message}}"
      ),
      .call = NULL
    )
  })
}

#' Load or Create Length Composition Data Fixture
#'
#' This function loads a FIMSFrame object containing length composition data.
#' If the data file does not exist, it creates one from raw data components
#' and saves it as an .RDS file for use in tests.
#'
#' @param with_na A logical value. If `TRUE`, it generates and loads a dataset
#'   where one year of survey data is missing. Defaults to `FALSE`.
#' @return A `FIMSFrame` object with length composition data.
load_data_length_comp <- function(with_na = FALSE) {
  tryCatch({
    if (with_na) {
      path <- testthat::test_path("fixtures", "data_length_comp_na.RDS")
    } else {
      path <- testthat::test_path("fixtures", "data_length_comp.RDS")
    }
    
    if (!file.exists(path)) {
      load(test_path("fixtures", "integration_test_data_components.RData"))

      # Generate dataset with only length composition data
      data_length_comp_raw <- rbind(
        landings_data, 
        index_data, 
        weight_at_age_data
      ) |>
        dplyr::mutate(
          length = NA,
          .after = "age"
        ) |>
        rbind(length_comp_data, length_age_data)

      if (with_na) {
        na_index <- as.Date("2-01-01")
        
        data_length_comp_raw <- data_length_comp_raw |>
          dplyr::filter(
            !(name == "survey1" &
                type %in% c("index", "length_comp", "age-to-length-conversion") &
                datestart == na_index
            )
          )
      }

      fims_data <- FIMS::FIMSFrame(data_length_comp_raw)
      saveRDS(fims_data, file = path)
    }
    readRDS(path)
  }, error = function(e) {
    cli::cli_abort(
      c(
        "x" = "Failed to create or load {.val {path}}.",
        "i" = "Please check the code logic in `load_data_length_comp()`.",
        "i" = "File to check: `tests/testthat/helper-integration-tests-setup-run.R`.",
        "!" = "Original error: {.emph {e$message}}"
      ),
      .call = NULL
    )
  })
}

#' Load or Create a Fitted Model with Length Composition Data
#'
#' This function loads a saved object that was fitted using only length
#' composition data. If the file does not exist, it runs the model fitting
#' process and saves the result as an .RDS fixture for future tests.
#'
#' @param with_na A logical value passed to `load_data_length_comp` to specify
#'   whether to use data with missing values. Defaults to `FALSE`.
#' @return An object containing the model results.
load_fit_length_comp <- function(with_na = FALSE) {
  tryCatch({
    if (with_na) {
      path <- testthat::test_path("fixtures", "fit_length_comp_na.RDS")
    } else {
      path <- testthat::test_path("fixtures", "fit_length_comp.RDS")
    }

    if (!file.exists(path)) {
      data_length_comp <- load_data_length_comp(with_na = with_na)

      parameters <- load_parameters_model_comparison_project() |>
        # TODO: delete the code below when log_devs estimation error fixed
        dplyr::mutate(
          estimation_type = dplyr::if_else(
            label == "log_devs" & module_type == "BevertonHolt",
            "constant",
            estimation_type
          )
        )
      fit_length_comp <- parameters |>
        # remove rows that have module_type == AgeComp
        dplyr::rows_delete(
          y = tibble::tibble(module_type = "AgeComp")
        ) |>
        initialize_fims(data = data_length_comp) |>
        fit_fims(optimize = TRUE)
      clear()

      # Save FIMS results as a test fixture for additional fimsfit tests
      saveRDS(
        fit_length_comp,
        file = path,
        compress = FALSE
      )
    }
    readRDS(path)
  }, error = function(e) {
    cli::cli_abort(
      c(
        "x" = "Failed to create or load {.val {path}}.",
        "i" = "Please check the code logic in `load_fit_length_comp()`.",
        "i" = "File to check: `tests/testthat/helper-integration-tests-setup-run.R`.",
        "i" = "Other functions in the same file that may be relevant include:
          1) `load_data_length_comp()` for loading or creating length composition data;
          2) `load_parameters_model_comparison_project()` for loading or creating
          a parameters tibble",
        "!" = "Original error: {.emph {e$message}}"
      ),
      .call = NULL
    )
  })
}

#' Load or Create Age and Length Composition Data Fixture
#'
#' This function loads a FIMSFrame object containing both age and length
#' composition data. If the file does not exist, it creates one and saves it
#' as an .RDS file for use in tests.
#'
#' @param with_na A logical value. If `TRUE`, it generates a dataset with
#'   one missing year of age composition and one missing year of length
#'   composition data. Defaults to `FALSE`.
#' @return A `FIMSFrame` object with both age and length composition data.
load_data_age_length_comp <- function(with_na = FALSE) {
  tryCatch({
    if (with_na) {
      path <- testthat::test_path("fixtures", "data_age_length_comp_na.RDS")
    } else {
      path <- testthat::test_path("fixtures", "data_age_length_comp.RDS")
    }
    
    if (!file.exists(path)) {
      load(test_path("fixtures", "integration_test_data_components.RData"))

      data_age_length_comp_raw <- data1
      
      if (with_na) {
        # Missing year for all data sets is year 0002, i.e., yyyy-mm-dd
        age_na_index <- as.Date("2-01-01")
        length_na_index <- as.Date("12-01-01")
        data_age_length_comp_raw <- data_age_length_comp_raw |>
          dplyr::filter(
            !(name == "survey1" & 
                type %in% c("age_comp") & 
                datestart == age_na_index
            )
          ) |>
          dplyr::filter(
            !(name == "fleet1" &
                type %in% c("length_comp", "age-to-length-conversion") &
                datestart == length_na_index
            )
          )
      }

      fims_data <- FIMS::FIMSFrame(data_age_length_comp_raw)
      saveRDS(fims_data, file = path, compress = FALSE)
    }
    readRDS(path)
  }, error = function(e) {
    cli::cli_abort(
      c(
        "x" = "Failed to create or load {.val {path}}.",
        "i" = "Please check the code logic in `load_data_age_length_comp()`.",
        "i" = "File to check: `tests/testthat/helper-integration-tests-setup-run.R`.",
        "!" = "Original error: {.emph {e$message}}"
      ),
      .call = NULL
    )
  })
}

#' Load or Create a Fitted Model with Age and Length Composition Data
#'
#' This function loads a saved object fitted with both age and
#' length composition data. If the file doesn't exist, it runs the fitting
#' process and saves the result as an .RDS fixture.
#'
#' @param with_na A logical value passed to `load_data_age_length_comp` to
#'   specify whether to use data with missing values. Defaults to `FALSE`.
#' @return An object containing the model results.
load_fit_age_length_comp <- function(with_na = FALSE) {
  tryCatch({
    if (with_na) {
      path <- testthat::test_path("fixtures", "fit_age_length_comp_na.RDS")
    } else {
      path <- testthat::test_path("fixtures", "fit_age_length_comp.RDS")
    }

    if (!file.exists(path)) {
      data_age_length_comp <- load_data_age_length_comp(with_na = with_na)

      parameters <- load_parameters_model_comparison_project() |>
        # TODO: delete the code below when log_devs estimation error fixed
        dplyr::mutate(
          estimation_type = dplyr::if_else(
            label == "log_devs" & module_type == "BevertonHolt",
            "constant",
            estimation_type
          )
        )

      fit_age_length_comp <- initialize_fims(
        parameters = parameters,
        data = data_age_length_comp
      ) |>
        fit_fims(optimize = TRUE)
      clear()

      # Save FIMS results as a test fixture for additional fimsfit tests
      saveRDS(
        fit_age_length_comp,
        file = path,
        compress = FALSE
      )
    }
    readRDS(path)
  }, error = function(e) {
    cli::cli_abort(
      c(
        "x" = "Failed to create or load {.val {path}}.",
        "i" = "Please check the code logic in `load_fit_age_length_comp()`",
        "i" = "File to check: `tests/testthat/helper-integration-tests-setup-run.R`.",
        "i" = "Other functions in the same file that may be relevant include:
          1) `load_data_age_length_comp()` for loading or creating data with both
          age and length composition;
          2) `load_parameters_model_comparison_project()` for loading or creating
          a parameters tibble",
        "!" = "Original error: {.emph {e$message}}"
      ),
      .call = NULL
    )
  })
}

#' Load or Create a Deterministic Model Run
#'
#' This function loads a `fimsfit` object from a deterministic run
#' using full age and length composition data. If the
#' file doesn't exist, it runs the model and saves the result.
#'
#' @return An object from a deterministic model evaluation.
load_deterministic_age_length_comp <- function() {
  tryCatch({
    path <- testthat::test_path("fixtures", "deterministic_age_length_comp.RDS")
  
    if (!file.exists(path)) {
      data_age_length_comp <- load_data_age_length_comp(with_na = FALSE)

      parameters <- load_parameters_model_comparison_project()

      deterministic_age_length_comp <- initialize_fims(
        parameters = parameters,
        data = data_age_length_comp
      ) |>
        fit_fims(optimize = FALSE)
      clear()

      # Save FIMS results as a test fixture for additional fimsfit tests
      saveRDS(
        deterministic_age_length_comp,
        file = path,
        compress = FALSE
      )
    }
    readRDS(path)
  }, error = function(e) {
    cli::cli_abort(
      c(
        "x" = "Failed to create or load {.val {path}}.",
        "i" = "Please check the code logic in `load_deterministic_age_length_comp()`",
        "i" = "File to check: `tests/testthat/helper-integration-tests-setup-run.R`.",
        "i" = "Other functions in the same file that may be relevant include:
          1) `load_data_age_length_comp()` for loading or creating data with both
          age and length composition;
          2) `load_parameters_model_comparison_project()` for loading or creating
          a parameters tibble",
        "!" = "Original error: {.emph {e$message}}"
      ),
      .call = NULL
    )
  })
}

#' Utility to Load and Save All Data Fixtures
#'
#' A helper function to generate and save all data fixture files (`.RDS`) needed
#' for testing. It calls the various `load_data_*` helper functions for each data
#' type, both with and without missing values.
#'
#' @return NULL. This function is called for its side effect of creating files.
load_all_data <- function() {
  load_data_age_comp(with_na = FALSE)
  load_data_age_comp(with_na = TRUE)
  load_data_length_comp(with_na = FALSE)
  load_data_length_comp(with_na = TRUE)
  load_data_age_length_comp(with_na = FALSE)
  load_data_age_length_comp(with_na = TRUE)
}

#' Utility to Load and Save All Model Fit Fixtures
#'
#' A helper function to generate and save all fixture files (`.RDS`)
#' needed for testing. It calls the various `load_fit_*` helper functions for
#' each data type, both with and without missing values.
#'
#' @return NULL. This function is called for its side effect of creating files.
load_all_fits <- function() {
  load_fit_age_comp(with_na = FALSE)
  load_fit_age_comp(with_na = TRUE)
  load_fit_length_comp(with_na = FALSE)
  load_fit_length_comp(with_na = TRUE)
  load_fit_age_length_comp(with_na = FALSE)
  load_fit_age_length_comp(with_na = TRUE)
}