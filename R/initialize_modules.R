#' Initialize Population Module.
#' @export
initialize_population <- function(input, data, other_modules){

  # Check if "population" is present in the parameter input list
  if (!"population" %in% names(input[["parameter_input_list"]])) {
    cli::cli_abort("`input[['parameter_input_list']] must contain `population`.")
  }

  # Define the module name and retrieve the associated class and fields
  module_name <- "Population"
  module_value <- get(module_name)
  module_fields <- names(module_value@fields)
  module_methods <- names(module_value@methods)
  module <- new(module_value)

  module_input <- input[["parameter_input_list"]][["population"]]

  for (field in module_fields){
    # TODO:
    # - Update the Population interface to consistently use n_ages and n_years,
    #   as done in the S4 data_mile1 object. So we can update get_data_slot() code to
    #   module[[field]] <- slot(data, field).
    # - Currently hard-coded `estimate_prop_female` to 0.5 and `nseason` to 1.
    #   Update as needed.
    # - Add n_fleets to data_mile1. Should n_fleets include both
    #   fishing and survey fleets? Currently, data_mile1@fleets equals 1.
    module[[field]] <- switch(
      field,
      "ages" = get_data_slot(field_name = field, data = data),
      "nages" = get_data_slot(field_name = field, data = data),
      "proportion_female" = 0.5,
      "estimate_prop_female" = FALSE,
      "nyears" = get_data_slot(field_name = field, data = data),
      "nseasons" = 1,
      "nfleets" = length(input$module_list$fleets),
      set_param_vector(param_vector_name = field, module_input = module_input)
    )
  }

  # for (method in module_methods){
  #   module$setGrowth(other_modules)
  #   module$setMaturity()
  #   module$setRecruitment()
  # }

  # Final message to confirm success
  cli::cli_inform(c(
    "i" = "Population module initialized successfully."
    ))

  # TODO: how to return all modules between pipes and link modules
  # # Retrieve all objects in the environment
  # objs <- mget(ls())
  # modules <- get_rcpp_modules(objs)

  return(modules)
}
