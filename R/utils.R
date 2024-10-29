#' Retrieve the Value of a Slot from a Data Object
#'
#' @description
#' This function retrieves the value of a specific slot from a data object
#' based on the provided module field name.
#' @param field_name A character string specifying the name of the module field
#'   whose corresponding slot value is to be retrieved.
#' @param data An S4 object containing the data from which the slot value
#'   should be retrieved.
#' @return The value of the specified slot in the data object.
#' @examples
#' data(data_mile1)
#' data <- FIMS::FIMSFrame(data_mile1)
#' get_data_slot("ages", data)
#' @export
get_data_slot <- function(field_name, data) {

  # Check if field_name is a non-empty character string
  if (missing(field_name) || !is.character(field_name) || nchar(field_name) == 0) {
    cli::cli_abort("The {.var field_name} argument must be a non-empty character string.")
  }

  # Check if data is an object from FIMSFrame class
  if (!is(data, "FIMSFrame")) {
    cli::cli_abort("The {.var data} argument must be an object created by {.fn FIMS::FIMSFrame}.")
  }

  # Get the slot names from the data object
  slot_names <- slotNames(data)
  # Remove underscores from slot names for matching
  slot_names_clean <- gsub("_", "", slot_names)

  # Find the index of the matching slot
  match_index <- which(slot_names_clean == field_name)

  # If no match is found, throw an error
  if (length(match_index) == 0) {
    cli::cli_abort(c(
      "{.var field_name} not found in the data object."
    ))
  }

  # Return the value of the matched slot
  output <- slot(data, slot_names[match_index])
  return(output)
}

#' Set Parameter Vector Values Based on Module Input
#'
#' @description
#' This function sets the parameter vector values in a module based on
#' the provided module input, including both initial values and estimation information.
#' @param field A character string specifying the field name of the parameter vector to be updated.
#' @param module A module object in which the parameter vector is to be set.
#' @param module_input A list containing input parameters for the module, including
#'   value and estimation information for the parameter vector.
#' @return Modified module object.
#' @export
set_param_vector <- function(field, module, module_input) {
  # Check if field_name is a non-empty character string
  if (missing(field) || !is.character(field) || nchar(field) == 0) {
    cli::cli_abort("The {.var field} argument must be a non-empty character string.")
  }

  # Check if module is a reference class
  if (!is(module, "refClass")) {
    cli::cli_abort("The {.var module} argument must be a reference class created by {.fn methods::new}.")
  }

  # Check if module_input is a list
  if (!is.list(module_input)) {
    cli::cli_abort("The {.var module_input} argument must be a list.")
  }

  # Identify the name for the parameter value and estimation fields in module_input
  field_value_name <- grep(paste0(field, ".value"), names(module_input), value = TRUE)
  field_estimated_name <- grep(paste0(field, ".estimated"), names(module_input), value = TRUE)

  # Check if both value and estimation information are present
  if (length(field_value_name) == 0 || length(field_estimated_name) == 0) {
    cli::cli_abort(c("Missing value or estimation information for {.var field}."))
  }

  # Extract the value of the parameter vector
  field_value <- module_input[[field_value_name]]

  # Resize the field in the module if it has multiple values
  if (length(field_value) > 1) module[[field]]$resize(length(field_value))

  # Assign each value to the corresponding position in the parameter vector
  for (i in seq_along(field_value)) {
    module[[field]][i][["value"]] <- field_value[i]
  }

  # Set the estimation information for the entire parameter vector
  module[[field]]$set_all_estimable(module_input[[field_estimated_name]])
}
