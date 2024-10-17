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
  # parameter_input_names <- names(unlist(parameter_input, recursive = TRUE))

  flat_list <- unlist(parameter_input, recursive = TRUE)
  logical_names <- grep("estimated", names(flat_list), value = TRUE)
  parameter_input_list <- as.list(flat_list)
  parameter_input_list[logical_names] <- as.logical(parameter_input_list[logical_names])

  output <- list(
    parameter_input_list = parameter_input_list,
    module_list = module_list
  )
  return(output)
}

prepare_Logistic_default <- function() {
  # Can pass data to this function in the future to
  # dynamically set up initial values
  default <- list(
    inflection_point = list(
      value = 2, estimated = TRUE
    ),
    slope = list(
      value = 1, estimated = TRUE
    )
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
  names(default) <- form

  fleet_selectivity <- list(default)
  names(fleet_selectivity) <- fleet_name

  return(fleet_selectivity)
}

prepare_maturity_default <- function(form) {
  # Can pass data to this function in the future to
  # dynamically set up initial values
  default <- list(switch(form,
    "LogisticMaturity" = prepare_Logistic_default()
  ))
  names(default) <- form
  return(default)
}

prepare_growth_default <- function(form, data) {
  default <- list(switch(form,
    "EWAAgrowth" = prepare_EWAAgrowth_default(data)
  ))
  names(default) <- form
  return(default)
}

prepare_BevertonHoltRecruitment_default <- function(data) {
  default <- list(
    log_rzero = list(
      value = log(1e+06), estimated = TRUE
    ),
    logit_steep = list(
      value = -log(1.0 - 0.75) + log(0.75 - 0.2), estimated = TRUE
    ),
    log_devs = list(
      value = rep(0.0, data@n_years),
      nyears = data@n_years,
      estimated = TRUE
    )
  )
  attr(default, "types") = c("Parameter", "Parameter", "ParameterVector")
  return(default)
}

prepare_TMBDnormDistribution_default <- function(n_obs) {
  default <- list(
    log_sd = list(
      # Hard-coded a single value for the dimensions of ParameterVector
      value = rep(log(0.4), 1), estimated = FALSE
    ),
    x = list(value = rep(0, n_obs)),
    expected_values = list(value = rep(0, n_obs))
  )
  attr(default, "types") = rep("ParameterVector", length(default))
  distribution_default <- list(default)
  names(distribution_default) <- "TMBDnormDistribution"
  return(distribution_default)
}

prepare_recruitment_default <- function(recruitment,
                                        data) {
  form <- recruitment[["form"]]
  process_default <- list(switch(form,
    "BevertonHoltRecruitment" = prepare_BevertonHoltRecruitment_default(data)
  ))
  names(process_default) <- form

  distribution_input <- recruitment[["process_distribution"]]
  distribution <- list()
  if (length(distribution_input) > 0) {
    for (i in seq_along(distribution_input)) {
      distribution_default <- list(switch(distribution_input[i],
        "TMBDnormDistribution" = prepare_TMBDnormDistribution_default(
          n_obs = data@n_years
        )
      ))
      names(distribution_default) <- paste0(names(distribution_input)[i])

      distribution[i] <- list(distribution_default)
    }
  }
  names(distribution) <- "Recruitment"
  default <- c(process_default, distribution)
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
  updated_input <- update_arg
  new_parameter_input <- modifyList(input$parameter_input_list, updated_input)
  new_input <- list(
    parameter_input_list = new_parameter_input,
    module_list = current_input$module_list
  )
  return(new_input)
}

setup_module <- function(module_name, input) {
  module_list <- input$module_list
  parameter_list <- input$parameter_input_list
  parameter_sections <- strsplit(names(parameter_list), "\\.")

  model_form_char <- module_list[[module_name]]
  model_form <- get(model_form_char)

  # Identify parameter IDs for the model
  fleet_name <- names(model_form_char)
  if (!is.null(fleet_name)) {
    parameter_ids <- which(sapply(parameter_sections, function(x) all(c(module_list[[module_name]][module_id], fleet_name) %in% x)))
  } else {
    parameter_ids <- which(sapply(parameter_sections, function(x) all(c(module_list[[module_name]][["form"]]) %in% x)))
  }

  # Extract the parameters for the model
  parameter_input <- parameter_list[parameter_ids]
  param_names <- sapply(strsplit(names(parameter_input), "\\."), `[`, 2)
  unique_param_names <- unique(param_names)


  # parameter_keys <- grep(paste0("^", model_form_char, "\\."), names(parameter_input), value = TRUE)
  #
  # param_names <- unique(sub("\\.value$|\\.estimated$", "", parameter_keys))
  # unique_param_names <- unique(sub(paste0(model_form_char, "."), "", param_names))
  parameter_objects <- list()
  for (i in seq_along(unique_param_names)) {
    parameter_objects[[i]] <- new(Parameter)
    parameter_objects[[i]][["value"]] <-
      parameter_input[[paste0(model_form_char, ".", unique_param_names[i], ".value")]]

    parameter_objects[[i]][["estimated"]] <-
      parameter_input[[paste0(model_form_char, ".", unique_param_names[i], ".estimated")]]
  }

  # Create the model object using do.call
  module_object <- do.call(new, c(model_form, parameter_objects))

  # Return the created model object
  return(list(module_object = module_object, parameter_objects = parameter_objects))
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
  module_list <- input[["module_list"]]
  parameter_list <- input[["parameter_input_list"]]
  fleet_name <- names(module_list[["fleets"]])

  clear()

  maturity <- setup_module(
    module_name = "maturity",
    input = input
  )

  recruitment_form <- module_list[["recruitment"]][["form"]]
  recruitment <- setup_recruitment(
    form = recruitment_form,
    input = input
  )

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
weight_names <- paste0("EWAAgrowth.weights", 1:12)
weights <- setNames(om_input_list[[1]]$W.mt, weight_names)

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
    update_arg = c(
      list(
        LogisticMaturity.slope.value = 1.5,
        LogisticMaturity.slope.estimated = FALSE,
        survey1.LogisticSelectivity.inflection_point.estimated = FALSE
      ),
      as.list(weights) # Pass the weights as a named list
    )
  )

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




