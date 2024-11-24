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
#' \dontrun{
#' data(data_mile1)
#' data <- FIMS::FIMSFrame(data_mile1)
#' get_data_slot("ages", data)
#' }
get_data_slot <- function(field_name, data) {

  # Check if field_name is a non-empty character string
  if (missing(field_name) || !is.character(field_name) || nchar(field_name) == 0) {
    cli::cli_abort("The {.var field_name} argument must be a non-empty character string.")
  }

  # Check if data is an object from FIMSFrame class
  check_arg_data(data)

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

