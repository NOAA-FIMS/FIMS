#' @title Modify Parameters of a FIMS Model
#'
#' @description
#' Modify a parameter input and run a FIMS model.
#' This function is called by run_fims_likelihood()
#'
#' @param new_value The new value to be changed in the FIMS model.
#' @param parameter_name A string specifying the parameter name to modify.
#' This should match a value in the `label` column of the parameters tibble.
#' @param module_name The name of module associated with the parameter to be changed. Default is NULL.
#' @param parameters The tibble of input parameters for a FIMS model
#' @param data A dataframe of input data for FIMS model
#'
#' @return FIMS model fitted to the new parameter input value
#' @export
#' @keywords diagnostics
#' 
#' @examples
#' \dontrun{
#'  library(FIMS)
#' # Use built-in dataset from FIMS
#'  data("data_big")
#'  data_4_model <- FIMSFrame(data_big)
#' # Create a parameters object
#'  parameters <- data_4_model |>
#'  create_default_configurations() |>
#'  create_default_parameters(data = data_4_model)
#' # Fit a FIMS model with 1 year of data removed
#'  fit <- run_modified_pars_fims(new_value = 12.9,
#'    parameter_name = "log_rzero",
#'    parameters = parameters, data = data_big)
#' }

run_modified_pars_fims <- function(
  new_value,
  parameter_name,
  module_name = NULL,
  parameters,
  data
) {
  # Need to load packages for each worker for furrr functions
  # suppressWarnings({
  #   suppressPackageStartupMessages({
  #     require(FIMS, quietly = TRUE)
  #     require(dplyr, quietly = TRUE)
  #     require(tidyr, quietly = TRUE)
  #     require(cli, quietly = TRUE)
  #   })
  # })

  # if parameters is nested, then unnest
  if ("data" %in% names(parameters)) {
    parameters_to_use <- parameters |> tidyr::unnest(cols = data)
  } else {
    parameters_to_use <- parameters
  }

  # find the parameter
  if (!is.null(module_name)) {
    parameter_row <- parameters_to_use |>
      dplyr::filter(
        .data[["module_name"]] == module_name &
          .data[["label"]] == parameter_name
      )
    if (nrow(parameter_row) == 0) {
      cli::cli_abort(
        "Parameter with module name {module_name} and label {parameter_name} not found in parameters object"
      )
    }
  } else {
    parameter_row <- parameters_to_use |>
      dplyr::filter(.data[["label"]] == parameter_name)
    if (nrow(parameter_row) == 0) {
      cli::cli_abort(
        "Parameter with label {parameter_name} not found in parameters object"
      )
    }
    if (nrow(parameter_row) > 1) {
      cli::cli_abort(
        "Multiple parameters with label {parameter_name} found in parameters object, please specify module_name"
      )
    }
  }

  # Update value
  parameter_row[["value"]] <- new_value
  parameter_row[["estimation_type"]] <- "constant"
  parameters_mod <- parameters_to_use |>
    dplyr::rows_update(
      parameter_row,
      by = c("module_name", "label")
    )

  data_model <- FIMS::FIMSFrame(data)

  new_fit <- parameters_mod |>
    FIMS::initialize_fims(data = data_model) |>
    FIMS::fit_fims(optimize = TRUE)

  return(new_fit)
}

#' @title Modify data for a FIMS Model
#'
#' @description
#' Function to remove a given number of years of data and run FIMS model.
#' This function is called by run_fims_retrospective()
#'
#' @param years_to_remove number of years to remove
#' @param data full dataset used in base model run
#' @param parameters input parameters used in base FIMS model
#' @return FIMS model fitted with years of data removed
#' @export
#' @keywords diagnostics
#'
#' @examples
#' \dontrun{
#'  library(FIMS)
#' # Use built-in dataset from FIMS
#'  data("data_big")
#'  data_4_model <- FIMSFrame(data_big)
#' # Create a parameters object
#'  parameters <- data_4_model |>
#'  create_default_configurations() |>
#'  create_default_parameters(data = data_4_model)
#' # Fit a FIMS model with 1 year of data removed
#'  fit <- run_modified_data_fims(years_to_remove = 1, data = data_big, parameters = parameters)
#' }

run_modified_data_fims <- function(years_to_remove = 0, data, parameters) {
  # Need to load packages for each worker for furrr functions
  # suppressWarnings({
  #   suppressPackageStartupMessages({
  #     require(FIMS, quietly = TRUE)
  #     require(dplyr, quietly = TRUE)
  #     require(lubridate, quietly = TRUE)
  #     require(cli, quietly = TRUE)
  #   })
  # })
  # check if the input is a FIMSframe object and if so, extract the data
  # this is to avoid the warning:
  #   no applicable method for 'filter' applied to an object of class "FIMSFrame"
  if ("FIMSFrame" %in% methods::is(data)) {
    data_to_use <- data@data
  } else {
    data_to_use <- data
  }

  # Remove years from data, but leave landings, weight_at_age,
  # and age_to_length_conversion (if present)
  if (years_to_remove == 0) {
    data_mod <- data_to_use
  } else {
    # exclude weight-at-age from the calculation of the max year of data
    max_timing <- data_to_use |>
      dplyr::filter(.data[["type"]] != "weight_at_age") |>
      dplyr::pull(.data[["timing"]]) |>
      max(na.rm = TRUE)
    data_mod <- data_to_use |>
      dplyr::filter(
        (.data[["type"]] %in%
          c("landings", "age_to_length_conversion", "weight_at_age")) |
          .data[["timing"]] <= max_timing - years_to_remove
      )
  }
  # convert to FIMSFrame format
  data_model <- FIMS::FIMSFrame(data_mod)

  # report the year removed being run
  cli::cli_alert_info(
    "running model with {paste(years_to_remove, collapse = ', ')} years of data removed"
  )

  #User supplies parameters from base model
  fit <- parameters |>
    FIMS::initialize_fims(data = data_model) |>
    FIMS::fit_fims(optimize = TRUE)

  return(fit)
}
