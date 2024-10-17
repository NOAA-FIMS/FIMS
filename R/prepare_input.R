#' Prepare Initial Input for A FIMS Model
#'
#' This function prepares the initial input parameters for a FIMS model. It sets
#' up selectivity, recruitment, growth, and maturity modules
#' based on the provided data and specified forms.
#'
#' @param data An object containing input data.
#' @param selectivity A character vector indicating the selectivity models
#'   to be used. Default is `c("LogisticSelectivity")`.
#' @param recruitment_form A character vector indicating the recruitment
#'   model to be used. Default is `c("BevertonHoltRecruitment")`.
#' @param recruitment_distribution A character vector indicating the recruitment
#'   distribution model to be used. Default is `c("TMBDnormDistribution")`.
#' @param growth A character vector indicating the growth model to be used.
#'   Default is `c("EWAAgrowth")`.
#' @param maturity A character vector indicating the maturity model to be used.
#'   Default is `c("LogisticMaturity")`.
#'
#' @return A list containing:
#'   \item{parameter_input_list}{A list of prepared parameter inputs for the
#'   specified models.}
#'   \item{module_list}{A list containing the names of the selectivity,
#'   recruitment, growth, and maturity models used.}
#'
#' @examples
#' initial_input <- prepare_input(
#'   data = data_mile1,
#'   selectivity = c("LogisticSelectivity", "AnotherSelectivity"),
#'   recruitment_form = c("BevertonHoltRecruitment"),
#'   growth = c("EWAAgrowth"),
#'   maturity = c("LogisticMaturity")
#' )
#'
#' print(initial_input)
prepare_input <- function(
    data,
    fleets,
    recruitment = list(
      form = "BevertonHoltRecruitment",
      process_distribution = c(log_devs = "TMBDnormDistribution")
    ),
    growth = c("EWAAgrowth"),
    maturity = c("LogisticMaturity")) {
  # TODO: if there is no fleets info passed into the function,
  # Use default values for fleets in the data@data
  if (length(fleets) == 0) {
    cli::cli_bullets(c(
      "No fleet settings were provided, FIMS will apply default settings for
      all fleets listed in the data. These include:",
      "i" = "LogisticSelectivity for selectivity module",
      "i" = "TMBDlnormDistribution for index data",
      "i" = "TMBDmultinomDistribution for age composition data"
    ))
  }

  fleet_names <- names(fleets)

  mismatch_fleet_names <- fleet_names[!(fleet_names %in% unique(data@data$name))]
  if (length(mismatch_fleet_names > 0)) {
    cli::cli_abort(c(
      "The name of the fleets for selectivity settings must match
      the fleet names from the data:",
      "i" = "There {?is/are} {length(missmatch_fleet_names)}
            fleet{?s} missing from the data.",
      "x" = "The following fleet name{?s} {?is/are} missing from the data:
            {paste(missmatch_fleet_names, collapse = ', ')}"
    ))
  }

  module_list <- list(
    fleets = fleets,
    recruitment = recruitment,
    growth = growth,
    maturity = maturity
  )

  parameter_input <- list()

  for (i in 1:length(fleets)) {
    selectivity_temp <- prepare_selectivity_default(names(fleets)[i], fleets[[i]]$selectivity)
    parameter_input <- c(parameter_input, selectivity_temp)
  }


  recruitment_temp <- prepare_recruitment_default(
    recruitment = recruitment,
    data = data
  )
  parameter_input <- c(parameter_input, recruitment_temp)

  growth_temp <- prepare_growth_default(
    form = growth,
    data = data
  )
  parameter_input <- c(parameter_input, growth_temp)

  maturity_temp <- prepare_maturity_default(form = maturity)
  parameter_input <- c(parameter_input, maturity_temp)

  output <- list(
    parameter_input_list = parameter_input,
    module_list = module_list
  )
  return(output)
}

prepare_Logistic_default <- function() {
  # Can pass data to this function in the future to
  # dynamically set up initial values
  default <- list(
    inflection_point.value = 2,
    inflection_point.estimated = TRUE,
    slope.value = 1,
    slope.estimated = TRUE
  )
  attr(default, "types") = rep("Parameter", length(default))
  return(default)
}

prepare_EWAAgrowth_default <- function(data) {
  default <- list(
    ages = data@ages,
    weights = rep(0.001, data@n_ages)
  )
  attr(default, "types") = rep("data", length(default))
  return(default)
}

prepare_selectivity_default <- function(fleet_name, form) {
  # Can pass data to this function in the future to
  # dynamically set up initial values
  default <- list(switch(form,
                         "LogisticSelectivity" = prepare_Logistic_default(),
                         "DoubleLogisticSelectivity" = prepare_DoubleLogistic_default()
  ))

  names(default) <- fleet_name
  names(default[[fleet_name]]) <- paste0(form, ".", names(default[[fleet_name]]))

  return(default)
}

prepare_maturity_default <- function(form) {
  # Can pass data to this function in the future to
  # dynamically set up initial values
  default <- list(switch(form,
                         "LogisticMaturity" = prepare_Logistic_default()
  ))

  names(default) <- "maturity"
  names(default[["maturity"]]) <- paste0(form, ".", names(default[["maturity"]]))

  return(default)
}

prepare_growth_default <- function(form, data) {
  default <- list(switch(form,
                         "EWAAgrowth" = prepare_EWAAgrowth_default(data)
  ))
  names(default) <- "growth"
  names(default[["growth"]]) <- paste0(form, ".", names(default[["growth"]]))
  return(default)
}

prepare_BevertonHoltRecruitment_default <- function(data) {
  default <- list(
    log_rzero.value = log(1e+06),
    log_rzero.estimated = TRUE,
    logit_steep.value = -log(1.0 - 0.75) + log(0.75 - 0.2),
    logit_steep.estimated = TRUE,
    log_devs = rep(0.0, data@n_years),
    estimate_log_devs = TRUE
  )
  attr(default, "types") = c(
    rep(c("Parameter", "Parameter", "ParameterVector"),
        each = 2))
  return(default)
}

prepare_TMBDnormDistribution_default <- function(n_obs) {
  default <- list(
    log_sd.value = rep(log(0.4), 1),
    log_sd.estimated = FALSE,
    x = rep(0, n_obs),
    expected_values = rep(0, n_obs)
  )
  attr(default, "types") = rep("ParameterVector", length(default))
  return(default)
}

prepare_recruitment_default <- function(recruitment,
                                        data) {
  form <- recruitment[["form"]]
  process_default <- switch(form,
                            "BevertonHoltRecruitment" = prepare_BevertonHoltRecruitment_default(data)
  )
  names(process_default) <- paste0(form, ".", names(process_default))

  distribution_input <- recruitment[["process_distribution"]]
  if (length(distribution_input) > 0) {

    distribution_default <- switch(distribution_input,
                                   "TMBDnormDistribution" = prepare_TMBDnormDistribution_default(
                                     n_obs = data@n_years
                                   ))
    names(distribution_default) <- paste0(distribution_input, ".", names(distribution_default))
  }

  default <- list(c(process_default, distribution_default))
  attr(default, "types") <- c(attr(process_default, "types"),
                              attr(distribution_default, "types"))
  names(default) <- "recruitment"
  return(default)
}

#' Update Input Parameters for A FIMS Model
#'
#' This function updates the input parameters of a FIMS model.
#' It allows users to modify specific parameters by providing new values, while
#' retaining the existing module list from the current input.
#'
#' @param input A list containing the current input parameters, including:
#'   \item{parameter_input}{A vector of parameter inputs.}
#'   \item{module_list}{A list of module names used in the model.}
#' @param ... Named arguments representing the new parameter values to be
#'   updated in the existing parameter input. The names of these arguments
#'   should correspond to the parameter names in the input list.
#'
#' @return A list containing:
#'   \item{parameter_input_list}{A list of updated parameter inputs that
#'   includes any modifications made by the user.}
#'   \item{module_list}{The unchanged list of module names from the current
#'   input.}
#'
#' @examples
#' \dontrun{
#' updated_input <- update_input(
#'   input = initial_input,
#'   LogisticMaturity.slope.value = 1.5,
#'   LogisticMaturity.slope.estimated = FALSE
#' )
#'
#' print(updated_input)
#' }
#'
#' @export
update_input <- function(input, update_arg) {
  current_input <- input
  updated_input <- input$parameter_input_list
  module_names <- names(updated_input)
  for (i in seq_along(module_names)){
    args <- names(update_arg[[module_names[i]]])
    updated_input[[module_names[i]]][args] <- update_arg[[module_names[i]]]
  }

  # new_parameter_input <- modifyList(input$parameter_input_list, updated_input)
  new_input <- list(
    parameter_input_list = updated_input,
    module_list = current_input$module_list
  )
  return(new_input)
}

setup_module <- function(module_name, input, data) {
  # Extract module list and parameter list for the module
  module_list <- input[["module_list"]]

  if (module_name == "fleets"){
    # Get fleet names from the module list
    fleet_names <- names(module_list[["fleets"]])

    # Initialize a list for fleets and assign fleet names
    fleets <- vector(mode = "list", length = length(fleet_names))
    names(fleets) <- fleet_names

    # Populate parameter_list for each fleet using lapply for simplicity
    parameter_list <- lapply(fleet_names, function(fleet) {
      input[["parameter_input_list"]][[fleet]]
    })

    names(parameter_list) <- fleet_names

  } else {
    parameter_list <- input[["parameter_input_list"]][[module_name]]
  }




  # Get module form class name and create a new instance
  if (module_name == "recruitment"){
    module_form_name <- module_list[[module_name]]$form
  } else{
    module_form_name <- module_list[[module_name]]
  }
  module_form <- get(module_form_name)
  field_names <- names(module_form@fields)
  module_object <- new(module_form)
  distribution_object <- NULL

  # Loop through the parameters and populate the module object
  for (i in seq_along(field_names)){

    if (module_name == "growth"){
      param_value <- grep(field_names[i], names(parameter_list), value = TRUE)
      module_object[[field_names[i]]] <- parameter_list[[param_value]]
    } else if (module_name == "recruitment"){
      if (field_names[i] == c("estimate_log_devs")) {
        param_value <- grep(paste0("\\.", field_names[i]), names(parameter_list), value = TRUE)
        module_object[[field_names[i]]] <- parameter_list[[param_value]]
      } else if (field_names[i] == c("log_devs")) {
        param_value <- grep(paste0("\\.", field_names[i]), names(parameter_list), value = TRUE)
        module_object[[field_names[i]]] <- new(ParameterVector, parameter_list[[param_value]], data@n_years)
        # fill() can only fill in one value at a time.
        # module_object[[field_names[i]]]$resize(length(parameter_list[[param_value]]))
        # module_object[[field_names[i]]]$fill(parameter_list[[param_value]])
      } else {
        param_value <- grep(paste0(field_names[i], ".value"), names(parameter_list), value = TRUE)
        module_object[[field_names[i]]]$fill(parameter_list[[param_value]])
        param_estimated <- grep(paste0(field_names[i], ".estimated"), names(parameter_list), value = TRUE)
        module_object[[field_names[i]]]$set_all_estimable(parameter_list[[param_value]])
      }

      distribution_name <- get(module_list[[module_name]][["process_distribution"]])
      distribution_object <- new(distribution_name)
      distribution_fields <- names(distribution_name@fields)

      for (i in seq_along(distribution_fields)){

        if (distribution_fields[i] == "log_sd"){
          param_value <- grep(paste0(distribution_fields[i], ".value"), names(parameter_list), value = TRUE)
          distribution_object[[distribution_fields[i]]]$fill(parameter_list[[param_value]])
          param_estimated <- grep(paste0(distribution_fields[i], ".estimated"), names(parameter_list), value = TRUE)
          distribution_object[[distribution_fields[i]]]$set_all_estimable(parameter_list[[param_value]])
        } else {
          param_value <- grep(paste0("\\.", distribution_fields[i]), names(parameter_list), value = TRUE)
          module_object[[distribution_fields[i]]] <- new(ParameterVector, parameter_list[[param_value]], data@n_years)
        }

      }
      distribution_object$set_distribution_links(
        "random_effects",
        module_object[[names(module_list[[module_name]][["process_distribution"]])]]$get_id()
      )



    } else{
      param_value <- grep(paste0(field_names[i], ".value"), names(parameter_list), value = TRUE)
      module_object[[field_names[i]]]$fill(parameter_list[[param_value]])
      param_estimated <- grep(paste0(field_names[i], ".estimated"), names(parameter_list), value = TRUE)
      module_object[[field_names[i]]]$set_all_estimable(parameter_list[[param_value]])

    }
  }

  # Return the created model object
  return(list(
    module_object = module_object,
    distribution_object = distribution_object
  ))
}

setup_recruitment <- function(input) {
  module_list <- input[["module_list"]]
  parameter_list <- input[["parameter_input_list"]]
  parameter_sections <- strsplit(names(parameter_list), "\\.")

  form <- module_list[["recruitment"]][["form"]]
  module_form <- get(form)

  parameter_ids <- which(sapply(parameter_sections, function(x) all(c(form) %in% x)))
  # Extract the parameters for the model
  parameter_input <- parameter_list[parameter_ids]
  param_names <- sapply(strsplit(names(parameter_input), "\\."), `[`, 2)
  unique_param_names <- unique(param_names)

  module_object <- new(module_form)
  for (i in seq_along(unique_param_names)) {
    module_object[[unique_param_names[i]]][["value"]] <-
      parameter_input[[paste0(form, ".", unique_param_names[i], ".value")]]
  }
  # Create the model object using do.call
  module_object <- do.call(new, c(module_form, parameter_objects))
}

setup_input <- function(data, input) {
  # module_list <- input[["module_list"]]
  # parameter_list <- input[["parameter_input_list"]]

  clear()

  maturity <- setup_module(
    module_name = "maturity",
    input = input,
    data = data
  )

  growth <- setup_module(
    module_name = "growth",
    input = input,
    data = data
  )

  recruitment <- setup_module(
    module_name = "recruitment",
    input = input,
    data = data
  )


  # Get fleet names from the module list
  fleet_names <- names(module_list[["fleets"]])
  nfleets <- length(fleet_names)

  get_fleet_names <- function(data, fleet_type) {
    data |>
      subset(type == fleet_type) |>
      (\(x) x$name)() |>
      unique()
  }



  fishing_fleet_names <- get_fleet_names(data@data, "landings")
  survey_fleet_names <- get_fleet_names(data@data, "index")


  # Initialize an empty list to store fleet objects
  index_m <- index_distribution_m <-
    age_comp_m <- age_distribution_m <-
    selectivity_m <- fleet_m <- vector(mode = "list", length = nfleets)

  for (i in seq_along(fleet_names)){
    index_m[[i]] <- methods::new(Index, data@n_years)
    if (fleet_names[i] %in% fishing_fleet_names) {
      index_m[[i]]$index_data <- FIMS::m_landings(fleet_data)
    } else {
      index_m[[i]]$index_data <- FIMS::m_index(fleet_data, fleet_names[i])
    }

    age_comp_m[[i]] <- methods::new(AgeComp, data@n_years, data@n_ages)
    age_comp_m[[i]]$age_comp_data <-
      FIMS::m_agecomp(data, fleet_names[i]) *
      data@data |>
      subset(type == "age" & name == fleet_names[i]) |>
      (\(x) x$uncertainty)()

    # TODO: check selectivity setup
    selectivity_m[[i]] <- setup_module(
      module_name = "selectivity",
      input = input,
      data = data
    )


  }

  for (i in seq_along(fleetes)){
    fleets[[i]] <- setup_module(
      module_name = "fleets",
      input = input,
      data = data
    )
  }


  fishing_selectivity <- setup_module("selectivity", input, module_id = 1)
  survey_selectivity <- setup_module("selectivity", input, module_id = 2)
}

# example
fleets <- list(
  fleet1 = list(
    selectivity = "LogisticSelectivity",
    data_distribution = c(
      Index = "TMBDlnormDistribution",
      AgeComp = "TMBDmultinomDistribution"
    )
  ),
  survey1 = list(
    selectivity = "LogisticSelectivity",
    data_distribution = c(
      Index = "TMBDlnormDistribution",
      AgeComp = "TMBDmultinomDistribution"
    )
  )
)


load(test_path("fixtures", "integration_test_data.RData"))
update_arg <- list(
  fleet1 = list(
    LogisticSelectivity.inflection_point.estimated = FALSE,
    LogisticSelectivity.slope.value = 3
  ),
  maturity = list(LogisticMaturity.slope.value = 1.5),
  growth = list(EWAAgrowth.weights = om_input_list[[1]]$W.mt)
)

data(data_mile1)
initial_input <- data_mile1 |>
  FIMS::FIMSFrame() |>
  prepare_input(
    fleets = fleets,
    recruitment = list(
      form = "BevertonHoltRecruitment",
      process_distribution = c(log_devs = "TMBDnormDistribution")
    ),
    growth = "EWAAgrowth",
    maturity = "LogisticMaturity"
  ) |>
  update_input(
    update_arg = update_arg
  ) #|>
# jsonlite::toJSON(pretty = TRUE)

initial_input |>
  setup_recruitment()



setup_input(data, input = updated_input)


setup_recruitment("BevertonHoltRecruitment", data, parameter) |>
  setup_distribution() |>
  setup_growth("EWAAgrowth", data, parameter) |>
  setup_maturity("LogisticMaturity", data, parameter) |>
  setup_selectivity("Fleet1", data, parameter) |>
  setup_fleet("Fleet1", parameter) |>
  setup_selectivity("Survey1", data, parameter) |>
  setup_fleet("Survey1", parameter)
#



