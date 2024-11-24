#' Check if the data argument is an object of class `FIMSFrame`
#'
#' @description
#' This function checks whether the provided `data` argument is an object of the
#' class `FIMSFrame`. It ensures that the data adheres to the required structure
#' for use with FIMS-related functions.
#' @param data The object to check.
#' @return NULL If the check passes (i.e., `data` is of class `FIMSFrame`).
#' The function throws an error if the argument is not of class `FIMSFrame`.
#' @examples
#' \dontrun{
#' data("data_mile1")
#' fims_data <- FIMSFrame(data_mile1)
#' check_arg_data(fims_data)
#' check_arg_data(data = 1:10)
#' }
#' @noRd
check_arg_data <- function(data) {
  # Check if data is an object from FIMSFrame class
  if (!is(data, "FIMSFrame")) {
    cli::cli_abort("The {.var data} argument must be an object created by {.fn FIMS::FIMSFrame}.")
  }
}

#' Check if the input is valid
#'
#' @description
#' This function checks if a given input is valid by comparing it against a
#' predefined set of valid options. It ensures that the input matches one of
#' the acceptable options and throws an error with a descriptive
#' message if it does not.
#' @param input A character string representing the input to validate.
#' @param valid_options A character vector of valid options against which the input will be checked.
#' @param arg_name A character string describing the name of the argument or
#' context of the input being validated
#'
#' @return NULL if the input is valid. Throws an error if the input is invalid.
#' @examples
#' \dontrun{
#' valid_selectivity_options <- c("LogisticSelectivity", "DoubleLogisticSelectivity")
#' check_valid_input("LogisticSelectivity", valid_selectivity_options, "form")
#' check_valid_input("InvalidOption", valid_selectivity_options, "form")
#' }
#' @noRd
check_valid_input <- function(input, valid_options, arg_name) {
  # Check if the input is a character string
  if (!is.character(input) || length(input) != 1) {
    cli::cli_abort(c(
      "Invalid `{arg_name}` input:",
      "x" = "The {arg_name} argument must be a non-empty character string."
    ))
  }

  # Check if input is valid
  if (!input %in% valid_options) {
    cli::cli_abort(c(
      "Invalid `{arg_name}` input:",
      "x" = "The `{input}` provided for `{arg_name}` argument is missing from
            the supported options: {valid_options}"
    ))
  }
}
