#' Get the value of a slot from a data object based on a module field name.
#' @export
get_data_slot <- function(field_name, data) {
  # Get the slot names from the data object
  slot_names <- slotNames(data)
  # Remove underscores from slot names for matching
  slot_names_clean <- gsub("_", "", slot_names)

  # Find the index of the matching slot
  match_index <- which(slot_names_clean == field_name)

  # If no match is found, throw an error
  if (length(match_index) == 0) {
    cli::cli_abort(c(
      "Module field {field_name} not found in the data."
    ))
  }

  # Return the value of the matched slot
  output <- slot(data, slot_names[match_index])
}

#' Set Parameter Vector Values from Module Input.
#' @export
set_param_vector <- function(param_vector_name, module_input) {
  # Retrieve the names of the value and estimated slots for the parameter vector
  param_vector_value_name <- grep(paste0(param_vector_name, ".value"), names(module_input), value = TRUE)
  param_vector_estimated_name <- grep(paste0(param_vector_name, ".estimated"), names(module_input), value = TRUE)

  # Check for the presence of value and estimation information
  if (length(param_vector_value_name) == 0 || length(param_vector_estimated_name) == 0) {
    cli::cli_abort(c("Missing value or estimation information for field {param_vector_name}."))
  }

  # Extract the value of the parameter vector
  param_vector_value <- module_input[[param_vector_value_name]]

  # Create a new ParameterVector object
  param_vector_module <- methods::new(ParameterVector, param_vector_value, length(param_vector_value))

  # Set the estimation information for the parameter vector
  param_vector_module$set_all_estimable(module_input[[param_vector_estimated_name]])

  return(param_vector_module)
}

#' Get Rcpp Modules
#' @export
get_rcpp_modules <- function(objs){

  # Filter objects to find those that inherit from classes starting with "Rcpp_"
  rcpp_names <- names(Filter(function(i) {
    any(startsWith(class(i), "Rcpp_"))
  }, objs))

  rcpp_objs <- objs[rcpp_names]

}
