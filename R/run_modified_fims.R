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
#' Data after the last year kept, including catch, are removed, so the model
#' ends in that year. Weight at age for the next year is kept, because biomass
#' is also calculated at the start of that year. A data type of a fleet with
#' no observations left, e.g., age compositions that started after the last
#' year, is removed.
#'
#' @param years_to_remove number of years to remove
#' @param data full dataset used in base model run
#' @param parameters input parameters used in base FIMS model. Rows with a
#'   `timing` after the last year kept, e.g., `log_Fmort`, `log_devs`, and
#'   `log_M`, are removed.
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
  # FIMSFrame() adds -999 rows for years without data, so the last year of
  # the peel has rows even if nothing was observed in it
  if ("FIMSFrame" %in% methods::is(data)) {
    fims_frame <- data
  } else {
    fims_frame <- FIMS::FIMSFrame(data)
  }
  data_to_use <- fims_frame@data

  # Remove all data after the last year of the peel, including catch, so the
  # peel is not fit to data from the years it removes. FIMSFrame() sets the
  # model years from the data, so the peel also ends in that year.
  if (years_to_remove == 0) {
    data_mod <- data_to_use
    parameters_mod <- parameters
  } else {
    last_year <- FIMS::get_end_year(fims_frame) - years_to_remove
    # Rows without a timing, e.g., length_bin rows, apply to every year.
    # Biomass is also calculated at the start of the year after the last year,
    # which needs the weight at age in that year.
    data_mod <- data_to_use |>
      dplyr::filter(
        is.na(.data[["timing"]]) |
          .data[["timing"]] <= last_year |
          (.data[["type"]] == "weight_at_age" &
            .data[["timing"]] == last_year + 1)
      )
    # A data type of a fleet with only -999 rows left, e.g., age compositions
    # that started after the last year, is removed as if it were never
    # collected. The likelihood is set up from the observed rows, so it cannot
    # be set up from -999 rows alone.
    data_mod <- data_mod |>
      dplyr::filter(
        !all(.data[["observed"]] %in% -999),
        .by = c("fleet", "type")
      )
    # Parameters with a timing, e.g., log_Fmort, log_devs, and log_M, keep
    # only their values for the years of the peel
    parameters_mod <- parameters |>
      dplyr::filter(
        is.na(.data[["timing"]]) | .data[["timing"]] <= last_year
      )
  }
  # convert to FIMSFrame format
  data_model <- FIMS::FIMSFrame(data_mod)

  # report the year removed being run
  cli::cli_alert_info(
    "running model with {years_to_remove} years of data removed"
  )

  fit <- parameters_mod |>
    FIMS::initialize_fims(data = data_model) |>
    FIMS::fit_fims(optimize = TRUE)

  return(fit)
}
