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
    fleet_name,
    selectivity = c("LogisticSelectivity"),
    recruitment_form = c("BevertonHoltRecruitment"),
    recruitment_distribution = c("TMBDnormDistribution"),
    growth = c("EWAAgrowth"),
    maturity = c("LogisticMaturity")) {


  names(selectivity) <- fleet_name
  module_list <- list(
    selectivity = selectivity,
    recruitment_form = recruitment_form,
    recruitment_distribution = recruitment_distribution,
    growth = growth,
    maturity = maturity
  )

  if (length(selectivity) != length(fleet_name)) {
    cli::cli_abort(c(
      "Number of selectivity forms must match the number of fleets:",
      "i" = "There {?is/are} {length(fleet_name)} fleet{?s}.",
      "x" = "{length(selectivity)} selectivity form{?s} {?has/have} been provided."
    ))
  }

  parameter_input <- list()

  for (i in 1:length(selectivity)) {
    selectivity_temp <- prepare_selectivity_default(fleet_name[i], selectivity[i])
    parameter_input <- c(parameter_input, selectivity_temp)
  }

  recruitment_temp <- prepare_recruitment_default(
    recruitment_form = recruitment_form,
    recruitment_distribution = recruitment_distribution,
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

  flat_list <- unlist(parameter_input, recursive = TRUE)
  parameter_input_list <- as.list(flat_list)

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
  return(default)
}

prepare_DoubleLogistic_default <- function() {
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
  return(default)
}

prepare_EWAAgrowth_default <- function(data) {
  default <- list(
    ages = data@ages,
    weights = rep(0.001, data@n_ages)
  )
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
      value = rep(0.01, data@n_years),
      nyears = data@n_years,
      estimated = TRUE
    )
  )
  return(default)
}

prepare_TMBDnormDistribution_default <- function(data) {
  default <- list(
    log_sd = list(
      value = log(0.4), estimated = FALSE
    ),
    x = list(value = 0),
    expected_values = list(value = 0)
  )
  return(default)
}

prepare_recruitment_default <- function(recruitment_form,
                                        recruitment_distribution,
                                        data) {
  recruitment_default <- list(switch(recruitment_form,
    "BevertonHoltRecruitment" = prepare_BevertonHoltRecruitment_default(data)
  ))
  names(recruitment_default) <- recruitment_form

  if (recruitment_default[[recruitment_form]]$log_devs$estimated == TRUE) {
    distribution_default <- list(switch(recruitment_distribution,
      "TMBDnormDistribution" = prepare_TMBDnormDistribution_default(data)
    ))
    names(distribution_default) <- paste0("Recruitment", recruitment_distribution)
  }

  default <- c(recruitment_default, distribution_default)
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
update_input <- function(input, ...) {
  current_input <- input
  updated_input <- list(...)
  parameter_vector <- as.vector(input$parameter_input)
  new_parameter_input <- `[<-`(input$parameter_input, names(updated_input), updated_input)
  new_input <- list(
    parameter_input_list = new_parameter_input,
    module_list = current_input$module_list
  )
  return(new_input)
}

setup_module <- function(module_name, input, module_id = 1) {
  module_list <- input$module_list
  parameter_list <- input$parameter_input_list
  module_names <- strsplit(names(parameter_list), "\\.")

  model_form <- get(module_list[[module_name]][module_id])

  # Identify parameter IDs for the model
  fleet_name <- names(module_list[[module_name]][module_id])
  if (!is.null(fleet_name)) {
    parameter_ids <- which(sapply(module_names, function(x) all(c(module_list[[module_name]][module_id], fleet_name) %in% x)))
  } else {
    parameter_ids <- which(sapply(module_names, function(x) all(c(module_list[[module_name]][module_id]) %in% x)))
  }

  # Extract the parameters for the model
  parameter_input <- parameter_list[parameter_ids]

  # Determine the number of parameters
  num_parameters <- length(parameter_input)

  # Create a list of Parameter objects
  parameter_objects <- vector("list", num_parameters / 2) # Assuming each pair consists of value and estimated

  # Populate the list with Parameter objects
  for (i in seq(1, num_parameters, by = 2)) {
    parameter_objects[[ceiling(i / 2)]] <- new(
      Parameter,
      unlist(parameter_input[i]),
      unlist(parameter_input[i + 1])
    )
  }

  # Create the model object using do.call
  module_object <- do.call(new, c(model_form, parameter_objects))

  # Return the created model object
  return(module_object)
}

setup_input <- function(data, input) {
  module_list <- input[["module_list"]]
  parameter_list <- input$parameter_input_list
  fleet_name <- names(module_list[["selectivity"]])

  clear()

  maturity <- setup_module("maturity", input)

  fishing_selectivity <- setup_module("selectivity", input, module_id = 1)
  survey_selectivity <- setup_module("selectivity", input, module_id = 2)
}

# example
data(data_mile1)
data <- FIMS::FIMSFrame(data_mile1)

initial_input <- prepare_input(
  data = data,
  fleet_name = c("fleet1", "survey1"),
  selectivity = c("LogisticSelectivity", "LogisticSelectivity"),
  recruitment_form = "BevertonHoltRecruitment",
  recruitment_distribution = "TMBDnormDistribution",
  growth = "EWAAgrowth",
  maturity = "LogisticMaturity"
)


updated_input <- update_input(
  input = initial_input,
  LogisticMaturity.slope.value = 1.5,
  LogisticMaturity.slope.estimated = FALSE,
  survey1.LogisticSelectivity.inflection_point.estimated = FALSE
)

setup_input(data, input = updated_input)


setup_recruitment("BevertonHoltRecruitment", data, parameter) |>
  setup_distribution() |>
  setup_growth("EWAAgrowth", data, parameter) |>
  setup_maturity("LogisticMaturity", data, parameter) |>
  setup_selectivity("Fleet1", data, parameter) |>
  setup_fleet("Fleet1", parameter) |>
  setup_selectivity("Survey1", data, parameter) |>
  setup_fleet("Survey1", parameter)
