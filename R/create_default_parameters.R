#' Create Default Parameters for a FIMS Model
#'
#' @description
#' This function generates default parameter settings for a Fisheries Integrated
#' Modeling System (FIMS) model, including recruitment, growth, maturity, population,
#' and fleet configurations. It applies default configurations when
#' specific module settings are not provided by the user.
#' @param data An S4 object. FIMS input data.
#' @param fleets A named list of fleet configurations. Each element should specify
#' a fleet's selectivity form and data distribution settings.
#' If empty, default values will be applied based on the fleet types in the input data.
#' @param recruitment A list specifying recruitment settings. Default is a
#' Beverton-Holt recruitment with log-normal recruitment deviations.
#' @param growth A list specifying growth model settings. Default is "EWAAgrowth".
#' @param maturity A list specifying maturity model settings. Default is "LogisticMaturity".
#' @name create_default_parameters
#' @return A list containing:
#'   \item{parameters}{A list of parameter inputs for the FIMS model.}
#'   \item{modules}{A list of modules with default or user-provided settings.}
#' @export
create_default_parameters <- function(
    data,
    fleets,
    recruitment = list(
      form = "BevertonHoltRecruitment",
      process_distribution = c(log_devs = "TMBDnormDistribution")
    ),
    growth = list(form = "EWAAgrowth"),
    maturity = list(form = "LogisticMaturity")) {

  # Check if data is an object from FIMSFrame class
  if (!is(data, "FIMSFrame")) {
    cli::cli_abort("The {.var data} argument must be an object created by {.fn FIMS::FIMSFrame}.")
  }

  # TODO: if there is no fleets info passed into the function,
  # Use default values for fleets in the data@data
  # if (length(fleets) == 0) {
  #   cli::cli_bullets(c(
  #     "No fleet settings were provided, FIMS will apply default settings for
  #     all fleets listed in the data. These include:",
  #     "i" = "LogisticSelectivity for selectivity module",
  #     "i" = "TMBDlnormDistribution for index data",
  #     "i" = "TMBDmultinomDistribution for age composition data"
  #   ))
  # }

  # Check for fleet names that do not match those in the data object
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

  # Create module list
  module_list <- list(
    fleets = fleets,
    recruitment = recruitment,
    growth = growth,
    maturity = maturity
  )

  # Create parameter inputs
  parameter_input <- list()

  # Create fleet parameters
  for (i in 1:length(fleets)) {
    fleet_temp <- create_default_fleet(
      fleets = fleets,
      fleet_name = names(fleets)[i],
      data = data
    )
    parameter_input <- c(parameter_input, fleet_temp)
  }

  # Create recruitment parameters
  recruitment_temp <- create_default_recruitment(
    recruitment = recruitment,
    data = data
  )
  parameter_input <- c(parameter_input, recruitment_temp)

  # Create maturity parameters
  maturity_temp <- create_default_maturity(form = maturity$form)
  parameter_input <- c(parameter_input, maturity_temp)

  # Handle population parameters based on recruitment form
  if (recruitment[["form"]] == "BevertonHoltRecruitment") {
    log_rzero <-
      recruitment_temp[["recruitment"]][[paste0(recruitment$form, ".log_rzero.value")]]
  } else {
    cli::cli_abort("Unsupported recruitment form {.var recruitment[['form']]}")
  }

  # Create population parameters
  population_temp <- create_default_Population(data, log_rzero)
  parameter_input <- c(parameter_input, population_temp)

  # Compile output
  output <- list(
    parameters = parameter_input,
    modules = module_list
  )
  return(output)
}

#' Create Default Population Parameters
#'
#' @description
#' This function sets up default parameters for a population module.
#' @details
#' The log of the initial numbers at age (log_init_naa.value) is set based on
#' unexploited recruitment and natural mortality.
#' @param data An S4 object. FIMS input data.
#' @param log_rzero A numeric value representing the log of unexploited recruitment.
#' @rdname create_default_parameters
#' @return A named list of default population parameters, including initial numbers
#' at age and natural mortality rate.
#' @export
create_default_Population <- function(data, log_rzero) {

  # Check if data is an object from FIMSFrame class
  if (!is(data, "FIMSFrame")) {
    cli::cli_abort("The {.var data} argument must be an object created by {.fn FIMS::FIMSFrame}.")
  }

  # Check if log_rzero is numeric
  if (!is.numeric(log_rzero) || length(log_rzero) != 1) {
    cli::cli_abort("The {.var log_rzero} argument must be a single numeric value.")
  }

  # Extract necessary values from data
  n_years <- slot(data, "n_years")
  n_ages <- slot(data, "n_ages")

  # Set natural mortality rate
  M_value <- 0.2

  # Calculate initial numbers at age based on log_rzero and M_value
  init_naa <- exp(log_rzero) * exp(-(slot(data, "ages") - 1) * M_value)
  init_naa[n_ages] <- init_naa[n_ages] / M_value # sum of infinite series

  # Create a list of default parameters
  default <- list(
    log_M.value = rep(log(M_value), n_years * n_ages),
    log_M.estimated = FALSE,
    log_init_naa.value = log(init_naa),
    log_init_naa.estimated = TRUE
  )

  # Name the list elements
  names(default) <- paste0("Population.", names(default))
  # Wrap the default parameters in a population list for output
  population_list <- list(default)
  names(population_list) <- "population"
  return(population_list)
}

#' Create Default Logistic Parameters
#'
#' @description
#' This function creates a default configuration for logistic form.
#' It sets up default values for parameters related to the inflection point and slope.
#' @rdname create_default_parameters
#' @return A list containing the default logistic parameters, with inflection_point
#'   and slope values and their estimation status.
#' @export
create_default_Logistic <- function() {
  # Generate default list with inflection point and slope parameters
  default <- list(
    inflection_point.value = 2,
    inflection_point.estimated = TRUE,
    slope.value = 1,
    slope.estimated = TRUE
  )
  return(default)
}

# TODO: Add create_default_DoubleLogistic()

#' Create Default Selectivity Parameters
#'
#' @description
#' This function creates default fleet selectivity parameters based on the provided
#' selectivity form. It supports different selectivity forms such as LogisticSelectivity
#' and DoubleLogisticSelectivity.
#' @param fleet_name A character. Name of the fleet.
#' @param form A character. Selectivity form (e.g., LogisticSelectivity and DoubleLogisticSelectivity).
#' @rdname create_default_parameters
#' @return A list containing default selectivity parameter values for the specified form.
#' @export
create_default_selectivity <- function(fleet_name, form) {
  # Validate input fleet_name
  if (!is.character(fleet_name) || nchar(fleet_name) == 0) {
    cli::cli_abort("The {.var fleet_name} argument must be a non-empty character string.")
  }

  # Validate selectivity form
  valid_forms <- c("LogisticSelectivity", "DoubleLogisticSelectivity")
  if (!form %in% valid_forms) {
    cli::cli_abort(c(
      "Invalid selectivity form: ",
      "x" = "The selectivity form {form} is missing from the supported forms:
            {paste(valid_forms, collapse = ', ')}", "."))
  }

  default <- switch(form,
    "LogisticSelectivity" = create_default_Logistic(),
    "DoubleLogisticSelectivity" = create_default_DoubleLogistic()
  )

  names(default) <- paste0(form, ".", names(default))

  return(default)
}

#' Create Default Fleet Parameters
#'
#' @description
#' This function creates default parameters for a given fleet. It compiles selectivity
#' parameters along with index and agecomp distributions.
#'
#' @param fleets A list of fleet configurations.
#' @param fleet_name A character. Name of the fleet.
#' @param data An S4 object. FIMS input data.
#' @rdname create_default_parameters
#' @return A list with default parameters for the fleet.
#' @export
create_default_fleet <- function(fleets,
                                 fleet_name,
                                 data) {


  # Check if the fleet_name exists in fleets list
  if (!fleet_name %in% names(fleets)) {
    cli::cli_abort("Fleet name {fleet_name} does not exist in the fleets list.")
  }

  # Check if data is an object from FIMSFrame class
  if (!is(data, "FIMSFrame")) {
    cli::cli_abort("The {.var data} argument must be an object created by {.fn FIMS::FIMSFrame}.")
  }

  # Create default selectivity parameters
  selectivity_form <- fleets[[fleet_name]][["selectivity"]][["form"]]
  selectivity_default <-
    create_default_selectivity(fleet_name, form = selectivity_form)

  # Get fleet types from the data object
  fleet_types <- data@data |>
    subset(name == fleet_name) |>
    (\(x) x$type)() |>
    unique()

  # Determine default Fleet parameters based on fleet type
  process_default <- if ("landings" %in% fleet_types) {
    list(
      log_Fmort.value = log(rep(0.00001, data@n_years)),
      log_Fmort.estimated = TRUE
    )
  } else {
    list(
      log_q.value = 0,
      log_q.estimated = TRUE
    )
  }

  names(process_default) <- paste0("Fleet.", names(process_default))

  # Create index distribution defaults
  index_distribution <- fleets[[fleet_name]][["data_distribution"]]["Index"]

  # TODO: remove this section. User should update index_uncertainty from data
  # object, not this default list.
  index_uncertainty <- data@data |>
    subset(name == fleet_name) |>
    subset(type %in% c("landings", "index")) |>
    (\(x) x$uncertainty)()

  index_distribution_default <- switch(index_distribution,
    "TMBDnormDistribution" = create_default_TMBDnormDistribution(
      value = index_uncertainty, input_type = "data", data = data
    ),
    "TMBDlnormDistribution" = create_default_TMBDlnormDistribution(
      value = index_uncertainty, input_type = "data", data = data
    )
  )
  names(index_distribution_default) <- paste0(index_distribution, ".", names(index_distribution_default))

  # Compile all default parameters into a single list
  default <- list(c(selectivity_default, process_default, index_distribution_default))

  names(default) <- fleet_name
  return(default)
}

#' Create Default Maturity Parameters
#'
#' @description
#' This function creates default parameters for maturity based on the specified
#' form (e.g., LogisticMaturity).
#' @param form A character. The form of maturity (e.g., LogisticMaturity).
#' @rdname create_default_parameters
#' @return A list containing the default maturity parameters.
#' @export
create_default_maturity <- function(form) {

  # Check if the form is a character string
  if (!is.character(form) || length(form) != 1) {
    cli::cli_abort("The {.var form} argument must be a non-empty character string.")
  }

  # Validate the provided maturity form
  valid_forms <- c("LogisticMaturity")
  if (!form %in% valid_forms) {
    cli::cli_abort(c(
      "Invalid maturity form: ",
      "x" = "The maturity form {form} is missing from the supported form:
            {paste(valid_forms, collapse = ', ')}", "."))
  }

  # Default parameters setup
  # Currently only supports "LogisticMaturity", but can be extended
  default <- list(switch(form,
    "LogisticMaturity" = create_default_Logistic()
  ))

  names(default) <- "maturity"
  names(default[["maturity"]]) <- paste0(form, ".", names(default[["maturity"]]))

  return(default)
}

#' Create Default Beverton-Holt Recruitment parameters
#'
#' @description
#' This function sets up default parameters for Beverton-Holt recruitment parameters.
#' @param data An S4 object. FIMS input data.
#' @rdname create_default_parameters
#' @return A list containing default recruitment parameters.
#' @export
create_default_BevertonHoltRecruitment <- function(data) {
  # Check if data is an object from FIMSFrame class
  if (!is(data, "FIMSFrame")) {
    cli::cli_abort("The {.var data} argument must be an object created by {.fn FIMS::FIMSFrame}.")
  }

  # Create default parameters for Beverton-Holt recruitment
  default <- list(
    log_rzero.value = log(1e+06),
    log_rzero.estimated = TRUE,
    logit_steep.value = -log(1.0 - 0.75) + log(0.75 - 0.2),
    logit_steep.estimated = FALSE,
    log_devs.value = rep(0.0, data@n_years),
    log_devs.estimated = TRUE,
    estimate_log_devs = TRUE
  )
  return(default)
}

#' Create Default TMBDnormDistribution Parameters
#'
#' @description
#' Create default parameters for TMBDnormDistribution.
#' @param value Default value for `log_sd`.
#' @param data An S4 object. FIMS input data.
#' @param input_type A character. Specifies if input is data or process.
#' @rdname create_default_parameters
#' @return A list of default parameters for TMBDnormDistribution.
#' @export
create_default_TMBDnormDistribution <- function(value = log(0.4), data, input_type = "data") {
  # Check if input_type is valid
  valid_input_types <- c("data", "process")
  if (!input_type %in% valid_input_types) {
    cli::cli_abort(c(
      "Invalid input_type: ",
      "x" = "The input_type {input_type} is missing from the supported types:
            {paste(valid_input_types, collapse = ', ')}", "."))
  }

  # Check if data is an object from FIMSFrame class
  if (!is(data, "FIMSFrame")) {
    cli::cli_abort("The {.var data} argument must be an object created by {.fn FIMS::FIMSFrame}.")
  }

  # Create default parameters
  default <- list(
    log_sd.value = value,
    log_sd.estimated = FALSE
  )

  # If input_type is 'process', add additional parameters
  if (input_type == "process") {
    default <- c(
      default,
      list(
        x.value = rep(0, data@n_years),
        x.estimated = FALSE,
        expected_values.value = rep(0, data@n_years),
        expected_values.estimated = FALSE
      )
    )
  }
  return(default)
}

#' Create Default TMBDlnormDistribution Parameters
#'
#' @description
#' Create default parameters for TMBDlnormDistribution.
#' @param value Default value for `log_sd`.
#' @param data An S4 object. FIMS input data.
#' @param input_type A character. Specifies if input is data or process.
#' @rdname create_default_parameters
#' @return A list of default parameters for TMBDlnormDistribution.
#' @export
create_default_TMBDlnormDistribution <- function(value = 0.1, data, input_type = "data") {

  # Validate input value
  if (!is.numeric(value) || any(value <= 0, na.rm = TRUE)) {
    cli::cli_abort("The {.var value} argument must be positive numeric values.")
  }

  # Check if data is an object from FIMSFrame class
  if (!is(data, "FIMSFrame")) {
    cli::cli_abort("The {.var data} argument must be an object created by {.fn FIMS::FIMSFrame}.")
  }

  # Check if input_type is valid
  valid_input_types <- c("data", "process")
  if (!input_type %in% valid_input_types) {
    cli::cli_abort(c(
      "Invalid input_type: ",
      "x" = "The input_type {input_type} is missing from the supported types:
            {paste(valid_input_types, collapse = ', ')}", "."))
  }

  # Create the default list with log standard deviation
  default <- list(
    log_sd.value = log(value),
    log_sd.estimated = FALSE
  )

  # Add additional parameters if input_type is "process"
  if (input_type == "process") {
    default <- c(
      default,
      list(
        x.value = rep(0, data@n_years),
        x.estimated = FALSE,
        expected_values.value = rep(0, data@n_years),
        expected_values.estimated = FALSE
      )
    )
  }
  return(default)
}

#' Create Default Recruitment Parameters
#'
#' Create default recruitment parameters based on recruitment form and process distribution.
#'
#' @param recruitment A list with recruitment details, including form and process distribution type.
#' @param data An S4 object. FIMS input data.
#' @rdname create_default_parameters
#' @return A list with the default parameters for recruitment.
#' @export
create_default_recruitment <- function(recruitment, data) {

  # Check if recruitment is a list
  if (!is.list(recruitment)) {
    cli::cli_abort("The {.var recruitment} argument must be a list.")
  }

  # Check if form is provided and valid
  form <- recruitment[["form"]]
  valid_forms <- c("BevertonHoltRecruitment")
  if (is.null(form) || !form %in% valid_forms) {
    cli::cli_abort(c(
      "Invalid or missing recruitment form: ",
      "i" = "The supported forms:
            {paste(valid_forms, collapse = ', ')}", "."))
  }

  # Create default parameters based on the recruitment form
  process_default <- switch(form,
    "BevertonHoltRecruitment" = create_default_BevertonHoltRecruitment(data)
  )
  names(process_default) <- paste0(form, ".", names(process_default))

  # Create default distribution parameters based on the distribution type
  distribution_input <- recruitment[["process_distribution"]]
  distribution_default <- NULL
  if (!is.null(distribution_input)) {
    distribution_default <- switch(distribution_input,
      "TMBDnormDistribution" = create_default_TMBDnormDistribution(
        value = 0.1,
        data = data,
        input_type = "process"
      )
    )
    names(distribution_default) <- paste0(distribution_input, ".", names(distribution_default))
  }

  # Combine process and distribution defaults into a single list
  default <- list(c(process_default, distribution_default))
  names(default) <- "recruitment"
  return(default)
}

#' Update Input Parameters for a FIMS Model
#'
#' @description
#' This function updates the input parameters of a FIMS model.
#' It allows users to modify specific parameters by providing new values, while
#' retaining the existing modules information from the current input.
#' @param current_parameters A list containing the current input parameters, including:
#'   \item{parameters}{A list of parameter inputs.}
#'   \item{modules}{A list of module names used in the model.}
#' @param modified_parameters A named list representing new parameter values to update.
#' @rdname create_default_parameters
#' @return A list containing:
#'   \item{parameters}{A list of updated parameter inputs that
#'   includes any modifications made by the user.}
#'   \item{modules}{The unchanged list of module names from the current
#'   input.}
#' @export
update_parameters <- function(current_parameters, modified_parameters) {

  # Check if current_parameters is a list with required components
  if (!is.list(current_parameters) || !all(c("parameters", "modules") %in% names(current_parameters))) {
    cli::cli_abort("The {.var current_parameters} argument must be a list containing parameters and modules.")
  }

  # Check if modified_parameters is a named list
  if (!is.list(modified_parameters) || is.null(names(modified_parameters))) {
    cli::cli_abort("The {.var modified_parameters} argument must be must be a named list.")
  }

  new_param_input <- current_parameters$parameters
  module_names <- names(new_param_input)

  # Update parameters for each module based on modified_parameters
  for (i in seq_along(module_names)) {
    parameter_names <- names(modified_parameters[[module_names[i]]])
    new_param_input[[module_names[i]]][parameter_names] <- modified_parameters[[module_names[i]]]
  }

  # Create a new list for updated input
  new_input <- list(
    parameters = new_param_input,
    modules = current_parameters$modules
  )
  return(new_input)
}
