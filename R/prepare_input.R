#' #' Prepare Initial Input for A FIMS Model
#' #'
#' #' This function prepares the initial input parameters for a FIMS model. It sets
#' #' up selectivity, recruitment, growth, and maturity modules
#' #' based on the provided data and specified forms.
#' #'
#' #' @param data An object containing input data.
#' #' @param selectivity A character vector indicating the selectivity models
#' #'   to be used. Default is `c("LogisticSelectivity")`.
#' #' @param recruitment_form A character vector indicating the recruitment
#' #'   model to be used. Default is `c("BevertonHoltRecruitment")`.
#' #' @param recruitment_distribution A character vector indicating the recruitment
#' #'   distribution model to be used. Default is `c("TMBDnormDistribution")`.
#' #' @param growth A character vector indicating the growth model to be used.
#' #'   Default is `c("EWAAgrowth")`.
#' #' @param maturity A character vector indicating the maturity model to be used.
#' #'   Default is `c("LogisticMaturity")`.
#' #'
#' #' @return A list containing:
#' #'   \item{parameter_input_list}{A list of prepared parameter inputs for the
#' #'   specified models.}
#' #'   \item{module_list}{A list containing the names of the selectivity,
#' #'   recruitment, growth, and maturity models used.}
#' #'
#' #' @examples
#' #' initial_input <- prepare_input(
#' #'   data = data_mile1,
#' #'   selectivity = c("LogisticSelectivity", "AnotherSelectivity"),
#' #'   recruitment_form = c("BevertonHoltRecruitment"),
#' #'   growth = c("EWAAgrowth"),
#' #'   maturity = c("LogisticMaturity")
#' #' )
#' #'
#' #' print(initial_input)
#' prepare_input <- function(
#'     data,
#'     fleets,
#'     recruitment = list(
#'       form = "BevertonHoltRecruitment",
#'       process_distribution = c(log_devs = "TMBDnormDistribution")
#'     ),
#'     growth = c("EWAAgrowth"),
#'     maturity = c("LogisticMaturity")) {
#'   # TODO: if there is no fleets info passed into the function,
#'   # Use default values for fleets in the data@data
#'   if (length(fleets) == 0) {
#'     cli::cli_bullets(c(
#'       "No fleet settings were provided, FIMS will apply default settings for
#'       all fleets listed in the data. These include:",
#'       "i" = "LogisticSelectivity for selectivity module",
#'       "i" = "TMBDlnormDistribution for index data",
#'       "i" = "TMBDmultinomDistribution for age composition data"
#'     ))
#'   }
#'
#'   fleet_names <- names(fleets)
#'
#'   mismatch_fleet_names <- fleet_names[!(fleet_names %in% unique(data@data$name))]
#'   if (length(mismatch_fleet_names > 0)) {
#'     cli::cli_abort(c(
#'       "The name of the fleets for selectivity settings must match
#'       the fleet names from the data:",
#'       "i" = "There {?is/are} {length(missmatch_fleet_names)}
#'             fleet{?s} missing from the data.",
#'       "x" = "The following fleet name{?s} {?is/are} missing from the data:
#'             {paste(missmatch_fleet_names, collapse = ', ')}"
#'     ))
#'   }
#'
#'   module_list <- list(
#'     fleets = fleets,
#'     recruitment = recruitment,
#'     growth = growth,
#'     maturity = maturity
#'   )
#'
#'   parameter_input <- list()
#'
#'   for (i in 1:length(fleets)) {
#'     fleet_temp <- prepare_fleet_default(names(fleets)[i],
#'                                         fleets[[i]]$selectivity,
#'                                         data)
#'     parameter_input <- c(parameter_input, fleet_temp)
#'   }
#'
#'
#'   recruitment_temp <- prepare_recruitment_default(
#'     recruitment = recruitment,
#'     data = data
#'   )
#'   parameter_input <- c(parameter_input, recruitment_temp)
#'
#'   growth_temp <- prepare_growth_default(
#'     form = growth,
#'     data = data
#'   )
#'   parameter_input <- c(parameter_input, growth_temp)
#'
#'   maturity_temp <- prepare_maturity_default(form = maturity)
#'   parameter_input <- c(parameter_input, maturity_temp)
#'
#'   if (recruitment[["form"]] == "BevertonHoltRecruitment"){
#'     log_rzero <-
#'       recruitment_temp[["recruitment"]][[paste0(recruitment$form, ".log_rzero.value")]]
#'   }
#'
#'   population_temp <- prepare_Population_default(data, log_rzero)
#'   parameter_input <- c(parameter_input, population_temp)
#'
#'   output <- list(
#'     parameter_input_list = parameter_input,
#'     module_list = module_list
#'   )
#'   return(output)
#' }
#'
#' prepare_Population_default <- function(data, log_rzero){
#'   n_years <- slot(data, "n_years")
#'   n_ages <- slot(data, "n_ages")
#'
#'   M_value = 0.2
#'
#'   # initial numbers at age based on R0 + mortality; following FIMS case
#'   # study NWFSC-petrale
#'   init_naa <- exp(log_rzero) * exp(-(slot(data, "ages") - 1) * M_value)
#'   init_naa[n_ages] <- init_naa[n_ages] / M_value # sum of infinite series
#'
#'   default <- list(
#'     log_M.value = rep(log(M_value), n_years * n_ages),
#'     log_M.estimated = FALSE,
#'     log_init_naa.value = log(init_naa),
#'     log_init_naa.estimated = TRUE
#'   )
#'
#'   names(default) <- paste0("Population.", names(default))
#'   population_list <- list(default)
#'   names(population_list) <- "population"
#'   return(population_list)
#' }
#'
#' prepare_Logistic_default <- function() {
#'   # Can pass data to this function in the future to
#'   # dynamically set up initial values
#'   default <- list(
#'     inflection_point.value = 2,
#'     inflection_point.estimated = TRUE,
#'     slope.value = 1,
#'     slope.estimated = TRUE
#'   )
#'   # attr(default, "types") = rep("Parameter", length(default))
#'   return(default)
#' }
#'
#' prepare_EWAAgrowth_default <- function(data) {
#'   default <- list(
#'     ages = data@ages,
#'     weights = m_weight_at_age(data)
#'   )
#'   attr(default, "types") = rep("data", length(default))
#'   return(default)
#' }
#'
#' prepare_selectivity_default <- function(fleet_name, form) {
#'   # Can pass data to this function in the future to
#'   # dynamically set up initial values
#'   default <- switch(form,
#'                          "LogisticSelectivity" = prepare_Logistic_default(),
#'                          "DoubleLogisticSelectivity" = prepare_DoubleLogistic_default()
#'   )
#'
#'   names(default) <- paste0(form, ".", names(default))
#'
#'   return(default)
#' }
#'
#' prepare_fleet_default <- function(fleet_name,
#'                                   form, data){
#'
#'   selectivity_default <-
#'     prepare_selectivity_default(fleet_name, form)
#'
#'   fleet_types <- data@data |>
#'     subset(name == fleet_name) |>
#'     (\(x) x$type)() |>
#'     unique()
#'
#'   process_default <- list(
#'     is_survey = !("landings" %in% fleet_types),
#'     log_Fmort = if ("landings" %in% fleet_types) log(rep(0.00001, data@n_years)) else NULL,
#'     estimate_Fmort = "landings" %in% fleet_types,
#'     log_q = if ("landings" %in% fleet_types) 0 else 1,
#'     estimate_q = !("landings" %in% fleet_types)
#'   )
#'   names(process_default) <- paste0("Fleet.", names(process_default))
#'
#'   index_distribution <- fleets[[fleet_name]][["data_distribution"]]["Index"]
#'
#'   index_uncertainty <- data@data |>
#'     subset(name == fleet_name) |>
#'     subset(type %in% c("landings", "index")) |>
#'     (\(x) x$uncertainty)()
#'
#'   index_distribution_default <- switch(index_distribution,
#'                                  "TMBDnormDistribution" = prepare_TMBDnormDistribution_default(
#'                                    value = index_uncertainty
#'                                  ),
#'                                  "TMBDlnormDistribution" = prepare_TMBDlnormDistribution_default(
#'                                    value = index_uncertainty
#'                                  ))
#'   names(index_distribution_default) <- paste0(index_distribution, ".", names(index_distribution_default))
#'
#'   default <- list(c(selectivity_default, process_default, index_distribution_default))
#'
#'   names(default) <- fleet_name
#'   return(default)
#' }
#'
#' prepare_maturity_default <- function(form) {
#'   # Can pass data to this function in the future to
#'   # dynamically set up initial values
#'   default <- list(switch(form,
#'                          "LogisticMaturity" = prepare_Logistic_default()
#'   ))
#'
#'   names(default) <- "maturity"
#'   names(default[["maturity"]]) <- paste0(form, ".", names(default[["maturity"]]))
#'
#'   return(default)
#' }
#'
#' prepare_growth_default <- function(form, data) {
#'   default <- list(switch(form,
#'                          "EWAAgrowth" = prepare_EWAAgrowth_default(data)
#'   ))
#'   names(default) <- "growth"
#'   names(default[["growth"]]) <- paste0(form, ".", names(default[["growth"]]))
#'   return(default)
#' }
#'
#' prepare_BevertonHoltRecruitment_default <- function(data) {
#'   default <- list(
#'     log_rzero.value = log(1e+06),
#'     log_rzero.estimated = TRUE,
#'     logit_steep.value = -log(1.0 - 0.75) + log(0.75 - 0.2),
#'     logit_steep.estimated = TRUE,
#'     log_devs = rep(0.0, data@n_years),
#'     estimate_log_devs = TRUE
#'   )
#'   attr(default, "types") = c(
#'     rep(c("Parameter", "Parameter", "ParameterVector"),
#'         each = 2))
#'   return(default)
#' }
#'
#' prepare_TMBDnormDistribution_default <- function(value = 0.1) {
#'   default <- list(
#'     log_sd.value = value,
#'     log_sd.estimated = rep(FALSE, length(value))# ,
#'     # x = rep(0, n_obs),
#'     # expected_values = rep(0, n_obs)
#'   )
#'   attr(default, "types") = rep("ParameterVector", length(default))
#'   return(default)
#' }
#' prepare_TMBDlnormDistribution_default <- function(value = 0.1) {
#'   default <- list(
#'     log_logsd.value = log(value),
#'     log_logsd.estimated = rep(FALSE, length(value))
#'   )
#'   attr(default, "types") = rep("ParameterVector", length(default))
#'   return(default)
#' }
#'
#' prepare_recruitment_default <- function(recruitment,
#'                                         data) {
#'   form <- recruitment[["form"]]
#'   process_default <- switch(form,
#'                             "BevertonHoltRecruitment" = prepare_BevertonHoltRecruitment_default(data)
#'   )
#'   names(process_default) <- paste0(form, ".", names(process_default))
#'
#'   distribution_input <- recruitment[["process_distribution"]]
#'   if (length(distribution_input) > 0) {
#'
#'     distribution_default <- switch(distribution_input,
#'                                    "TMBDnormDistribution" = prepare_TMBDnormDistribution_default())
#'     names(distribution_default) <- paste0(distribution_input, ".", names(distribution_default))
#'   }
#'
#'   default <- list(c(process_default, distribution_default))
#'   attr(default, "types") <- c(attr(process_default, "types"),
#'                               attr(distribution_default, "types"))
#'   names(default) <- "recruitment"
#'   return(default)
#' }
#'
#' #' Update Input Parameters for A FIMS Model
#' #'
#' #' This function updates the input parameters of a FIMS model.
#' #' It allows users to modify specific parameters by providing new values, while
#' #' retaining the existing module list from the current input.
#' #'
#' #' @param input A list containing the current input parameters, including:
#' #'   \item{parameter_input}{A vector of parameter inputs.}
#' #'   \item{module_list}{A list of module names used in the model.}
#' #' @param ... Named arguments representing the new parameter values to be
#' #'   updated in the existing parameter input. The names of these arguments
#' #'   should correspond to the parameter names in the input list.
#' #'
#' #' @return A list containing:
#' #'   \item{parameter_input_list}{A list of updated parameter inputs that
#' #'   includes any modifications made by the user.}
#' #'   \item{module_list}{The unchanged list of module names from the current
#' #'   input.}
#' #'
#' #' @examples
#' #' \dontrun{
#' #' updated_input <- update_input(
#' #'   input = initial_input,
#' #'   LogisticMaturity.slope.value = 1.5,
#' #'   LogisticMaturity.slope.estimated = FALSE
#' #' )
#' #'
#' #' print(updated_input)
#' #' }
#' #'
#' #' @export
#' update_input <- function(input, update_arg) {
#'   current_input <- input
#'   updated_input <- input$parameter_input_list
#'   module_names <- names(updated_input)
#'   for (i in seq_along(module_names)){
#'     args <- names(update_arg[[module_names[i]]])
#'     updated_input[[module_names[i]]][args] <- update_arg[[module_names[i]]]
#'   }
#'
#'   # new_parameter_input <- modifyList(input$parameter_input_list, updated_input)
#'   new_input <- list(
#'     parameter_input_list = updated_input,
#'     module_list = current_input$module_list
#'   )
#'   return(new_input)
#' }
#'
#' setup_module <- function(module_name, input, data) {
#'   # Extract module list and parameter list for the module
#'   module_list <- input[["module_list"]]
#'
#'   if (module_name == "fleets"){
#'     # Get fleet names from the module list
#'     fleet_names <- names(module_list[["fleets"]])
#'
#'     # Initialize a list for fleets and assign fleet names
#'     fleets <- vector(mode = "list", length = length(fleet_names))
#'     names(fleets) <- fleet_names
#'
#'     # Populate parameter_list for each fleet using lapply for simplicity
#'     parameter_list <- lapply(fleet_names, function(fleet) {
#'       input[["parameter_input_list"]][[fleet]]
#'     })
#'
#'     names(parameter_list) <- fleet_names
#'
#'   } else {
#'     parameter_list <- input[["parameter_input_list"]][[module_name]]
#'   }
#'
#'
#'
#'
#'   # Get module form class name and create a new instance
#'   if (module_name == "recruitment"){
#'     module_form_name <- module_list[[module_name]]$form
#'   } else{
#'     module_form_name <- module_list[[module_name]]
#'   }
#'   module_form <- get(module_form_name)
#'   field_names <- names(module_form@fields)
#'   module_object <- new(module_form)
#'   distribution_object <- NULL
#'
#'   # Loop through the parameters and populate the module object
#'   for (i in seq_along(field_names)){
#'
#'     if (module_name == "growth"){
#'       param_value <- grep(field_names[i], names(parameter_list), value = TRUE)
#'       module_object[[field_names[i]]] <- parameter_list[[param_value]]
#'     } else if (module_name == "recruitment"){
#'       if (field_names[i] == c("estimate_log_devs")) {
#'         param_value <- grep(paste0("\\.", field_names[i]), names(parameter_list), value = TRUE)
#'         module_object[[field_names[i]]] <- parameter_list[[param_value]]
#'       } else if (field_names[i] == c("log_devs")) {
#'         param_value <- grep(paste0("\\.", field_names[i]), names(parameter_list), value = TRUE)
#'         module_object[[field_names[i]]] <- new(ParameterVector, parameter_list[[param_value]], data@n_years)
#'         # fill() can only fill in one value at a time.
#'         # module_object[[field_names[i]]]$resize(length(parameter_list[[param_value]]))
#'         # module_object[[field_names[i]]]$fill(parameter_list[[param_value]])
#'       } else {
#'         param_value <- grep(paste0(field_names[i], ".value"), names(parameter_list), value = TRUE)
#'         module_object[[field_names[i]]]$fill(parameter_list[[param_value]])
#'         param_estimated <- grep(paste0(field_names[i], ".estimated"), names(parameter_list), value = TRUE)
#'         module_object[[field_names[i]]]$set_all_estimable(parameter_list[[param_value]])
#'       }
#'
#'       distribution_name <- get(module_list[[module_name]][["process_distribution"]])
#'       distribution_object <- new(distribution_name)
#'       distribution_fields <- names(distribution_name@fields)
#'
#'       for (j in seq_along(distribution_fields)){
#'
#'         if (distribution_fields[j] == "log_sd"){
#'           param_value <- grep(paste0(distribution_fields[j], ".value"), names(parameter_list), value = TRUE)
#'           distribution_object[[distribution_fields[j]]]$fill(parameter_list[[param_value]])
#'           param_estimated <- grep(paste0(distribution_fields[j], ".estimated"), names(parameter_list), value = TRUE)
#'           distribution_object[[distribution_fields[j]]]$set_all_estimable(parameter_list[[param_value]])
#'         } else {
#'           param_value <- grep(paste0("\\.", distribution_fields[j]), names(parameter_list), value = TRUE)
#'           if (length(param_value) > 0){
#'             module_object[[distribution_fields[j]]] <- new(ParameterVector, parameter_list[[param_value]], data@n_years)
#'           }
#'
#'         }
#'
#'       }
#'       distribution_object$set_distribution_links(
#'         "random_effects",
#'         module_object[[names(module_list[[module_name]][["process_distribution"]])]]$get_id()
#'       )
#'
#'
#'
#'     } else{
#'       param_value <- grep(paste0(field_names[i], ".value"), names(parameter_list), value = TRUE)
#'       module_object[[field_names[i]]]$fill(parameter_list[[param_value]])
#'       param_estimated <- grep(paste0(field_names[i], ".estimated"), names(parameter_list), value = TRUE)
#'       module_object[[field_names[i]]]$set_all_estimable(parameter_list[[param_value]])
#'
#'     }
#'   }
#'
#'   # Return the created model object
#'   return(list(
#'     module_object = module_object,
#'     distribution_object = distribution_object
#'   ))
#' }
#'
#' setup_input <- function(input, data) {
#'   # module_list <- input[["module_list"]]
#'   # parameter_list <- input[["parameter_input_list"]]
#'
#'   clear()
#'
#'   maturity <- setup_module(
#'     module_name = "maturity",
#'     input = input,
#'     data = data
#'   )
#'
#'   growth <- setup_module(
#'     module_name = "growth",
#'     input = input,
#'     data = data
#'   )
#'
#'   recruitment <- setup_module(
#'     module_name = "recruitment",
#'     input = input,
#'     data = data
#'   )
#'
#'   fleets <- setup_fleet(
#'     input = input,
#'     data = data
#'   )
#'
#'
#'
#' }
#'
#' setup_fleet <- function(input, data) {
#'
#'   module_name <- "fleets"
#'   fleet_names <- names(input[["module_list"]][["fleets"]])
#'
#'   # Initialize lists to store fleet-related objects
#'   index_m <- vector("list", length(fleet_names))
#'   index_distribution_m <- vector("list", length(fleet_names))
#'   age_comp_m <- vector("list", length(fleet_names))
#'   age_comp_uncertainty <- vector("list", length(fleet_names))
#'   age_comp_distribution_m <- vector("list", length(fleet_names))
#'   selectivity_m <- vector("list", length(fleet_names))
#'   fleet_m <- vector("list", length(fleet_names))
#'
#'   for (i in seq_along(fleet_names)) {
#'     fleet_name <- fleet_names[i]
#'     fleet_type <- unique(subset(data@data, name == fleet_name)$type)
#'
#'     fleet_parameters <- input[["parameter_input_list"]][[fleet_name]]
#'
#'     # Handle Selectivity
#'     selectivity_name <- input[["module_list"]][[module_name]][[fleet_name]][["selectivity"]]
#'     selectivity_m[[i]] <- initialize_selectivity(selectivity_name, fleet_parameters, data@n_years)
#'
#'     # Initialize Fleet object
#'     fleet_m[[i]] <- initialize_fleet(fleet_name, fleet_parameters, data, selectivity_m[[i]])
#'
#'     # Setup Index data
#'     index_m[[i]] <- initialize_index(fleet_type, data, fleet_name)
#'
#'     # Setup Index Distribution
#'     index_distribution_m[[i]] <- initialize_distribution(
#'       input, module_name, fleet_name, "Index", fleet_parameters, index_m[[i]], fleet_m[[i]], data@n_years
#'     )
#'
#'     # Setup Age Composition data
#'     age_comp_m[[i]] <- initialize_age_comp(fleet_name, data)
#'     age_comp_uncertainty[[i]] <- subset(data@data, name == fleet_name & type == "age")$uncertainty
#'     age_comp_m[[i]]$age_comp_data <- FIMS::m_agecomp(data, fleet_name) * age_comp_uncertainty[[i]]
#'
#'     # Setup Age Composition Distribution
#'     age_comp_distribution_m[[i]] <- initialize_distribution(
#'       input, module_name, fleet_name, "AgeComp", fleet_parameters, age_comp_m[[i]], fleet_m[[i]], data@n_years
#'     )
#'   }
#'
#'   return(fleet_m)
#' }
#'
#' # Helper function to initialize selectivity
#' initialize_selectivity <- function(selectivity_name, fleet_parameters, n_years) {
#'   selectivity_form <- get(selectivity_name)
#'   selectivity_m <- new(selectivity_form)
#'   selectivity_fields <- names(selectivity_form@fields)
#'
#'   selectivity_parameters <- fleet_parameters[grep(selectivity_name, names(fleet_parameters))]
#'
#'   for (field in selectivity_fields) {
#'     param_value_name <- grep(paste0(field, ".value"), names(selectivity_parameters), value = TRUE)
#'     param_estimated_name <- grep(paste0(field, ".estimated"), names(selectivity_parameters), value = TRUE)
#'
#'     if (length(param_value_name) > 0) {
#'       param_value <- selectivity_parameters[[param_value_name]]
#'       selectivity_m[[field]] <- new(ParameterVector, param_value, n_years)
#'       estimate_param <- unique(selectivity_parameters[[param_estimated_name]])
#'       selectivity_m[[field]]$set_all_estimable(estimate_param)
#'     }
#'   }
#'
#'   return(selectivity_m)
#' }
#'
#' # Helper function to initialize fleet
#' initialize_fleet <- function(fleet_name, fleet_parameters, data, selectivity_m) {
#'   fleet_form <- get("Fleet")
#'   fleet_m <- methods::new(fleet_form)
#'
#'   fleet_fields <- c("nages", "nyears", "is_survey", "log_Fmort", "log_q", "estimate_q")
#'
#'   for (field in fleet_fields) {
#'     if (field %in% c("nages", "nyears")) {
#'       fleet_m[[field]] <- ifelse(field == "nages", data@n_ages, data@n_years)
#'     } else if (field %in% c("is_survey", "estimate_q")){
#'       fleet_m[[field]] <- fleet_parameters[[paste0("Fleet.", field)]]
#'     } else {
#'       param_value <- fleet_parameters[[paste0("Fleet.", field)]]
#'
#'       if (!is.null(param_value)) {
#'         fleet_m[[field]] <- new(ParameterVector, param_value, length(param_value))
#'         param_estimated <- fleet_parameters[[paste0("Fleet.estimate_", sub("log_", "", field))]]
#'         fleet_m[[field]]$set_all_estimable(param_estimated)
#'       }
#'     }
#'   }
#'
#'   fleet_m$SetSelectivity(selectivity_m$get_id())
#'   return(fleet_m)
#' }
#'
#' # Helper function to initialize index
#' initialize_index <- function(fleet_type, data, fleet_name) {
#'   index_m <- methods::new(Index, data@n_years)
#'
#'   if ("landings" %in% fleet_type) {
#'     index_m[["index_data"]] <- FIMS::m_landings(data)
#'   } else {
#'     index_m[["index_data"]] <- FIMS::m_index(data, fleet_name)
#'   }
#'
#'   return(index_m)
#' }
#'
#' # Helper function to initialize distribution (Index/AgeComp)
#' initialize_distribution <- function(input, module_name, fleet_name, dist_type, fleet_parameters, observed_data, fleet_obj, n_years) {
#'   distribution_name <- input[["module_list"]][[module_name]][[fleet_name]][["data_distribution"]][[dist_type]]
#'   distribution_form <- get(distribution_name)
#'
#'   distribution_m <- methods::new(distribution_form)
#'   distribution_fields <- names(distribution_form@fields)
#'   distribution_parameters <- fleet_parameters[grep(distribution_name, names(fleet_parameters))]
#'
#'   for (field in distribution_fields) {
#'     param_value_name <- grep(paste0(field, ".value"), names(distribution_parameters), value = TRUE)
#'     param_estimated_name <- grep(paste0(field, ".estimated"), names(distribution_parameters), value = TRUE)
#'
#'     if (length(param_value_name) > 0) {
#'       param_value <- distribution_parameters[[param_value_name]]
#'       distribution_m[[field]] <- new(ParameterVector, param_value, n_years)
#'       estimate_param <- unique(distribution_parameters[[param_estimated_name]])
#'       distribution_m[[field]]$set_all_estimable(estimate_param)
#'     }
#'   }
#'
#'   distribution_m$set_observed_data(observed_data$get_id())
#'   distribution_m$set_distribution_links("data", fleet_obj$log_expected_index$get_id())
#'
#'   return(distribution_m)
#' }
#'
#' # Helper function to initialize age composition
#' initialize_age_comp <- function(fleet_name, data) {
#'   age_comp_m <- methods::new(AgeComp, data@n_years, data@n_ages)
#'   age_comp_m$age_comp_data <- FIMS::m_agecomp(data, fleet_name)
#'
#'   return(age_comp_m)
#' }
#'
#' # setup_fleet <- function(input, data){
#' #
#' #   module_name <- "fleets"
#' #   fleet_names <- names(input[["module_list"]][["fleets"]])
#' #
#' #   # Initialize lists to store fleet-related objects
#' #   index_m <- index_distribution_m <-
#' #     age_comp_m <- age_comp_uncertainty <- age_comp_distribution_m <-
#' #     selectivity_m <- fleet_m <- vector(mode = "list", length = length(fleet_names))
#' #
#' #   for (i in seq_along(fleet_names)){
#' #     fleet_name <- fleet_names[i]
#' #
#' #     fleet_type <- data@data |>
#' #       subset(name == fleet_names[i]) |>
#' #       (\(x) x$type)() |>
#' #       unique()
#' #
#' #     fleet_parameters <- input[["parameter_input_list"]][[fleet_name]]
#' #
#' #     # Handle selectivity
#' #     selectivity_name <- input[["module_list"]][[module_name]][[fleet_name]][["selectivity"]]
#' #     selectivity_form <- get(selectivity_name)
#' #     selectivity_m[[i]] <- new(selectivity_form)
#' #     selectivity_fields <- names(selectivity_form@fields)
#' #
#' #     selectivity_parameters <- fleet_parameters[grep(selectivity_name, names(fleet_parameters))]
#' #
#' #     for (field in selectivity_fields){
#' #       param_value_name <- grep(paste0(field, ".value"), names(selectivity_parameters), value = TRUE)
#' #       param_estimated_name <- grep(paste0(field, ".estimated"), names(selectivity_parameters), value = TRUE)
#' #       if (length(param_value_name) > 0) {
#' #         param_value <- selectivity_parameters[[param_value_name]]
#' #         selectivity_m[[field]] <-
#' #           new(ParameterVector, param_value, data@n_years)
#' #         estimate_param <- unique(selectivity_parameters[[param_estimated_name]])
#' #         selectivity_m[[field]]$set_all_estimable(estimate_param)
#' #       }
#' #
#' #     }
#' #
#' #     module_char <- "Fleet"
#' #     fleet_form <- get(module_char)
#' #     fleet_m[[i]] <- methods::new(fleet_form)
#' #     fleet_m_parameters <- fleet_parameters[grep(module_char, names(fleet_parameters))]
#' #     # TODO: Update the Rcpp interface for the fleet to include fewer fields that
#' #     # users can access, and use ParameterVector for both log_Fmort and log_q.
#' #     # This would allow for dynamical value passing. For example,
#' #
#' #     # fleet_fields <- names(fleet_form@fields)
#' #     fleet_fields <- c("nages", "nyears", "is_survey", "log_Fmort",
#' #                       "log_q", "estimate_q")
#' #
#' #     for (field in fleet_fields){
#' #       if (field %in% c("nages", "nyears")){
#' #         # TODO: Update the fleet interface to consistently use n_ages and n_years,
#' #         # as done in the S4 data_mile1 object. For example, we can change following
#' #         # two lines of code to:
#' #         # fleet_m[[i]][[field]] <- slot(data, field)
#' #         fleet_m[[i]][["nages"]] <- data@n_ages
#' #         fleet_m[[i]][["nyears"]] <- data@n_years
#' #       } else if (field %in% c("is_survey", "estimate_q")){
#' #         fleet_m[[i]][[field]] <- fleet_m_parameters[[paste0("Fleet.", field)]]
#' #       } else if (field %in% c("log_Fmort")){
#' #         param_value <- fleet_m_parameters[[paste0("Fleet.", field)]]
#' #         if (!is.null(param_value)) {
#' #           fleet_m[[i]][[field]] <- methods::new(
#' #             ParameterVector,
#' #             param_value,
#' #             length(param_value))
#' #           param_estimated <- fleet_m_parameters[["Fleet.estimate_F"]]
#' #           fleet_m[[i]][[field]]$set_all_estimable(param_estimated)
#' #         }
#' #       } else if (field == "log_q"){
#' #         param_value <- fleet_m_parameters[[paste0("Fleet.", field)]]
#' #         fleet_m[[i]][[field]] <- methods::new(
#' #           ParameterVector,
#' #           param_value,
#' #           length(param_value))
#' #         param_estimated <- fleet_m_parameters[["Fleet.estimate_q"]]
#' #         fleet_m[[i]][[field]]$set_all_estimable(param_estimated)
#' #       }
#' #
#' #     }
#' #
#' #     fleet_m[[i]]$SetSelectivity(selectivity_m[[i]]$get_id())
#' #
#' #     index_m[[i]] <- methods::new(Index, data@n_years)
#' #     if ("landings" %in% fleet_type){
#' #       index_m[[i]][["index_data"]] <- FIMS::m_landings(data)
#' #     } else {
#' #       index_m[[i]][["index_data"]] <- FIMS::m_index(data, fleet_names[i])
#' #     }
#' #
#' #     index_distribution_name <- input[["module_list"]][[module_name]][[fleet_names[i]]][["data_distribution"]]["Index"]
#' #     index_distribution_form <- get(index_distribution_name)
#' #     index_distribution_m <- new(index_distribution_form)
#' #     index_distribution_fields <- names(index_distribution_form@fields)
#' #     index_distribution_parameters <- fleet_parameters[grep(index_distribution_name, names(fleet_parameters))]
#' #
#' #     for (field in index_distribution_fields){
#' #       param_value_name <- grep(paste0(field, ".value"), names(index_distribution_parameters), value = TRUE)
#' #       param_estimated_name <- grep(paste0(field, ".estimated"), names(index_distribution_parameters), value = TRUE)
#' #       if (length(param_value_name) > 0) {
#' #         param_value <- index_distribution_parameters[[param_value_name]]
#' #         index_distribution_m[[field]] <-
#' #           new(ParameterVector, param_value, data@n_years)
#' #         estimate_param <- unique(index_distribution_parameters[[param_estimated_name]])
#' #         index_distribution_m[[field]]$set_all_estimable(estimate_param)
#' #         index_distribution_m$set_observed_data(index_m[[i]]$get_id())
#' #         index_distribution_m$set_distribution_links("data", fleet_m[[i]]$log_expected_index$get_id())
#' #       }
#' #
#' #     }
#' #
#' #     age_comp_uncertainty[[i]] <- data@data |>
#' #       subset(name == fleet_names[i]) |>
#' #       subset(type == "age") |>
#' #       (\(x) x$uncertainty)()
#' #
#' #     age_comp_m[[i]] <- methods::new(AgeComp, data@n_years, data@n_ages)
#' #     age_comp_m[[i]]$age_comp_data <- FIMS::m_agecomp(data, fleet_names[i]) *
#' #       age_comp_uncertainty[[i]]
#' #
#' #     age_comp_distribution_name <- input[["module_list"]][[module_name]][[fleet_names[i]]][["data_distribution"]]["AgeComp"]
#' #     age_comp_distribution_form <- get(age_comp_distribution_name)
#' #     age_comp_distribution_m[[i]] <- methods::new(age_comp_distribution_form)
#' #     age_comp_distribution_m[[i]]$set_observed_data(age_comp_m[[i]]$get_id())
#' #     age_comp_distribution_m[[i]]$set_distribution_links("data",
#' #                                                         fleet_m[[i]]$proportion_catch_numbers_at_age$get_id())
#' #   }
#' #   return(fleet_m)
#' #
#' # }
#'
#'
