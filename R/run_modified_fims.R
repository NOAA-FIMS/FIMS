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
#' library(FIMS)
#' # Use built-in dataset from FIMS
#' data("data_big")
#' data_4_model <- FIMSFrame(data_big)
#' # Create a parameters object
#' parameters <- setup_default_parameters(data = data_4_model)
#' # Fit a FIMS model with 1 year of data removed
#' fit <- run_modified_pars_fims(
#'   new_value = 12.9,
#'   parameter_name = "log_rzero",
#'   parameters = parameters, data = data_big
#' )
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

  # find the parameter; `%in%` rather than `==` because `x == NULL` has length
  # 0, which `filter()` rejects, so a NULL module_name matches every module
  parameter_row <- parameters_to_use |>
    dplyr::filter(
      .data[["label"]] == .env$parameter_name,
      is.null(.env$module_name) |
        .data[["module_name"]] %in% .env$module_name
    )
  if (nrow(parameter_row) == 0) {
    cli::cli_abort(c(
      "Parameter with label {.val {parameter_name}}
      not found in parameters object.",
      "i" = if (!is.null(module_name)) "Searched module {.val {module_name}}."
    ))
  }
  if (nrow(parameter_row) > 1) {
    cli::cli_abort(c(
      "{nrow(parameter_row)} parameters have label {.val {parameter_name}};
      select a single parameter.",
      "i" = if (is.null(module_name)) {
        "Specify {.arg module_name} if the label is in more than 1 module."
      } else {
        "Module {.val {module_name}} has more than 1 row with this label (for
        example, 1 per fleet or year)."
      }
    ))
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
#' library(FIMS)
#' # Use built-in dataset from FIMS
#' data("data_big")
#' data_4_model <- FIMSFrame(data_big)
#' # Create a parameters object
#' parameters <- setup_default_parameters(data = data_4_model)
#' # Fit a FIMS model with 1 year of data removed
#' fit <- run_modified_data_fims(
#'   years_to_remove = 1,
#'   data = data_big,
#'   parameters = parameters
#' )
#' }
run_modified_data_fims <- function(years_to_remove = 0, data, parameters) {
  # check if the input is a FIMSframe object and if so, extract the data
  # this is to avoid the warning:
  # no applicable method for 'filter' applied to an object of class "FIMSFrame"
  if ("FIMSFrame" %in% methods::is(data)) {
    data_to_use <- data@data
  } else {
    data_to_use <- data
  }

  # Remove years from data, but leave catch, weight_at_age,
  # and age_to_length_conversion (if present)
  if (years_to_remove == 0) {
    data_mod <- data_to_use
  } else {
    # exclude weight-at-age from the calculation of the max year of data
    years <- as.integer(format(normalize_timing(data_to_use$timing), "%Y"))
    max_year <- max(years[!data_to_use$type %in% c("weight_at_age", "age_to_length_conversion")], na.rm = TRUE)
    keep <- data_to_use$type %in% c("catch", "age_to_length_conversion", "weight_at_age") |
      (!is.na(years) & years <= max_year - years_to_remove)
    data_mod <- data_to_use[keep, ]
  }
  # convert to FIMSFrame format
  data_model <- FIMS::FIMSFrame(data_mod)

  # report the year removed being run
  cli::cli_alert_info(
    "running model with {years_to_remove} years of data removed"
  )

  # User supplies parameters from base model
  fit <- parameters |>
    FIMS::initialize_fims(data = data_model) |>
    FIMS::fit_fims(optimize = TRUE)

  return(fit)
}
