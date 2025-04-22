# TODO: Document the names/items in each list that are returned
#' Create default parameters for a FIMS model
#'
#' @description
#' This function generates default parameter settings for a Fisheries
#' Integrated Modeling System (FIMS) model, including recruitment, growth,
#' maturity, population, and fleet configurations. It applies default
#' configurations when specific module settings are not provided by the user.
#' @param data An S4 object. FIMS input data.
#' @param fleets A named list of settings for the fleet module. Each element of
#'   the list should specify a fleet's selectivity form and settings for the
#'   data distribution. If this argument is missing, default values will be
#'   applied for each fleet that is not specified but present in `data` based
#'   on the types of information present for that fleet.
#' @param recruitment A list specifying the settings for the recruitment
#'   module. The default is a Beverton--Holt recruitment relationship with
#'   log-normal recruitment deviations.
#' @param growth A list specifying the settings for the growth module. The
#'   default is `"EWAAgrowth"`.
#' @param maturity A list specifying the settings for the maturity module. The
#'   default is `"LogisticMaturity"`.
#' @return
#' A list containing the following two entries:
#' \describe{
#'   \item{\code{parameters}:}{A list of parameter inputs for the FIMS
#'     model.}
#'   \item{\code{modules}:}{A list of modules with default or user-provided
#'     settings.}
#' }
#' @export
#' @seealso
#' * [FIMSFrame()]
#' * [update_parameters()]
#' @examples
#' \dontrun{
#' # Load the example dataset and create a FIMS data frame
#' data("data1")
#' fims_frame <- FIMSFrame(data1)
#'
#' # Define fleets specifications for each fleet in the example dataset
#' fleet1 <- survey1 <- list(
#'   selectivity = list(form = "LogisticSelectivity"),
#'   data_distribution = c(
#'     Index = "DlnormDistribution",
#'     AgeComp = "DmultinomDistribution"
#'   )
#' )
#'
#' # Create a list of default parameters given the fleet specifications set up
#' # above, recruitment, growth, and maturity specifications
#' default_parameters <- fims_frame |>
#'   create_default_parameters(
#'     fleets = list(fleet1 = fleet1, survey1 = survey1),
#'     recruitment = list(form = "BevertonHoltRecruitment"),
#'     growth = list(form = "EWAAgrowth"),
#'     maturity = list(form = "LogisticMaturity")
#'   )
#'
#' # Do the same as above except, model fleet1 with double logistic selectivity
#' # and do not specify the recruitment, growth, and maturity specifications
#' # because everything specified above were default arguments
#' parameters_with_double_logistic <- fims_frame |>
#'   create_default_parameters(
#'     fleets = list(
#'       fleet1 = list(
#'         selectivity = list(form = "DoubleLogisticSelectivity"),
#'         data_distribution = c(
#'           Index = "DlnormDistribution",
#'           AgeComp = "DmultinomDistribution",
#'           LengthComp = "DmultinomDistribution"
#'         )
#'       ),
#'       survey1 = survey1
#'     )
#'   )
#' 
#' # Compare the parameters for fleet1 in each set up
#' default_fleet1 <- purrr::map_df(
#'   default_parameters[["parameters"]][["fleet1"]],
#'   \(x) length(x)
#' ) |>
#'   tidyr::pivot_longer(cols = dplyr::everything())
#' updated_fleet1 <- purrr::map_df(
#'   parameters_with_double_logistic[["parameters"]][["fleet1"]],
#'   \(x) length(x)
#' ) |>
#'   tidyr::pivot_longer(cols = dplyr::everything())
#' dplyr::full_join(default_fleet1, updated_fleet1, by = "name")
#' knitr::kable(dplyr::full_join(default_fleet1, updated_fleet1, by = "name"))
#' }
create_default_parameters <- function(
    data,
    fleets,
    populations = list(
      population1 = list(
        recruitment = list(form = "BevertonHoltRecruitment"),
        # TODO: Rename EWAAgrowth to not use an acronym
        growth = list(form = "EWAAgrowth"),
        maturity = list(form = "LogisticMaturity")) 
    )
)
{
  # FIXME: use default values if there are no fleets info passed into the
  # function or a fleet is not present but it has data? Maybe we don't want the
  # latter because it could be that we want to drop a fleet from a model but we
  # don't want to alter the data?

  # Check for fleet names that do not match those in the data object
  fleet_names <- names(fleets)
  mismatch_fleet_names <- fleet_names[
    !(fleet_names %in% unique(dplyr::pull(get_data(data), name)))
  ]
  if (length(mismatch_fleet_names) > 0) {
    cli::cli_abort(c(
      "i" = "The name of the fleets for selectivity settings must match
            the fleet names present in the {.var data}.",
      "x" = "The following {length(mismatch_fleet_names)} fleet name{?s}
            {?is/are} missing from the data: {mismatch_fleet_names}."
    ))
  }

  # Create module list
  module_list <- list(
    fleets = fleets,
    populations = populations
  )

  # Create fleet parameters
  fleets_temp <- list()
  for (i in 1:length(fleets)) {
    fleets_temp <- c(
      fleets_temp,
      create_default_fleet(
        fleets = fleets,
        fleet_name = names(fleets)[i],
        data = data
      )
    )
  }

  populations_temp <- list()
  for(i in 1:length(populations)){

    # Create recruitment parameters
     recruitment_temp <- create_default_recruitment(
      recruitment = populations[[i]][["recruitment"]],
      data = data,
      input_type = populations[[i]][["recruitment"]][["form"]]
    )

    # Create maturity parameters
    maturity_temp <- create_default_maturity(
      form = populations[[i]][["maturity"]][["form"]]
    )

    # Create population parameters
    # Handle population parameters based on recruitment form
    if (populations[[i]][["recruitment"]][["form"]] == "BevertonHoltRecruitment") {
      log_rzero <- recruitment_temp[["population1"]][["recruitment"]][[
        paste0(populations[[i]][["recruitment"]][["form"]], ".log_rzero.value")
      ]]
    }
    population_temp <- create_default_Population(data, log_rzero)

    populations_temp <- c(
      populations_temp,
      recruitment_temp,
      maturity_temp,
      population_temp
    )
  }

  # Compile output
  output <- list(
    parameters = c(
      fleets_temp,
      populations_temp),
    modules = module_list
  )
  return(output)
}



#' @ Create a process model object
#'
#' @description This function creates a process model object used to store the
#' model parameters, type, method, log standard deviation, initial values, and
#' mapping.
#'
#' @param par A character string specifying the parameter name.
#' @param type A character string specifying the type of process model. Options are:
#' - iid: univariate normal distribution
#' - AR1: first-order autoregressive process - not implemented yet
#' - AR2: second-order autoregressive process - not implemented yet
#' - RW: a random walk process - not implemented yet
#' @param method A character string specifying the method used for the process
#'   model. Options are: 
#' - Fixed Effect: fixed effects estimation
#' - Penalized MLE: penalized maximum likelihood estimation
#' - Random Effect ConstVar: random effects with constant variance
#' - Random Effect Full: random effects with estimated variance
#' @param log_sd A numeric value specifying the log standard deviation.
#' @param init A numeric value specifying the initial value for the process
#'   model.
#' @param map A list specifying the mapping for the process model.
#' @return A list representing the process model object.
#' @export
#' @example
#'  process_model(
#'   par = "log_devs",
#'   type = "iid",
#'   method = "Random Effect ConstVar",
#'   sd = .01,  
#'   init = NULL)  
process_model <- function(
  par, 
  type = c("iid"), 
  method = c("Fixed Effect", "Penalized MLE", "Random Effect ConstVar", 
             "Random Effect Full"), 
  sd = NULL, 
  init = NULL, 
  map = list()){
  
  type <- rlang::arg_match(type)
  method <- rlang::arg_match(method)

  # Validate inputs
   if (!is.character(par)) {
    cli::cli_abort(c(
      "i" = "{.var par} needs to be a character representing a parameter in the FIMS model.",
      "x" = "{.var par} is type {typeof(par)}."
    ))
  }
  if (!is.null(sd) && !is.numeric(sd)) {
    cli::cli_abort(c(
      "i" = "{.var sd} needs to be a numeric value.",
      "x" = "{.var sd} is type {typeof(sd)}."
    ))
  }
  if (!is.null(init) && !is.numeric(init)) {
    cli::cli_abort(c(
      "i" = "{.var init} needs to be a numeric value.",
      "x" = "{.var init} is type {typeof(init)}."
    ))
  }
  if (!is.list(map)) {
    cli::cli_abort(c(
      "i" = "{.var map} needs to be a list.",
      "x" = "{.var map} is type {typeof(map)}."
    ))
  }
  if (method == "Fixed Effect" && !is.null(sd)) {
    cli::cli_abort(c(
      "i" = "{.var sd} should not be specified when method is set to {.var Fixed Effect}.",
      "i" = "The Fixed Effect method does not apply a distribution to the process and does not estimate sd.",
      "i" = "If you want to use a distribution, please set method to {.var Penalized MLE}, {.var Random Effect ConstVar}, or {.var Random Effect Full}.",
      "x" = "{.var sd} is {sd}}."
    ))
  }
  
  if (method == "Penalized MLE" && is.null(sd)) {
    cli::cli_abort(c(
      "i" = "{.var sd} needs to be specified when method is set to {.var Penalized MLE}.",
      "x" = "{.var sd} is NULL."
    ))
  }

  if (method == "Random Effects ConstVar" && is.null(log_sd)) {
    cli::cli_abort(c(
      "i" = "{.var sd} needs to be specified when method is set to {.var Random Effects ConstVar}.",
      "x" = "{.var sd} is NULL."
    ))
  }

  
  # Create a list to represent the process model
  process_model_object <- list(
    par = par,
    type = type,
    method = method,
    sd = sd,
    init = init,
    map = map
  )
  
  # Assign a class to the process model object for future extensions
  class(process_model_object) <- "FIMSProcessModel"
  
  return(process_model_object)
}

#' Create default population parameters
#'
#' @description
#' This function sets up default parameters for a population module.
#' @details
#' The natural log of the initial numbers at age (`log_init_naa.value`) is set based on
#' unexploited recruitment and natural mortality.
#' @param data An S4 object. FIMS input data.
#' @param log_rzero A numeric value representing the natural log of unexploited
#'   recruitment.
#' @return
#' A named list of default population parameters, including initial numbers at
#' age and natural mortality rate.
#' @noRd
create_default_Population <- function(data, log_rzero) {
  # Input checks
  # Check if log_rzero is numeric
  if (!is.numeric(log_rzero) || length(log_rzero) != 1) {
    local_bullets <- c(
      "i" = "{.var log_rzero} argument must be a single numeric value.",
      "x" = "{.var log_rzero} has a length of {length(log_rzero)}.",
      "x" = "{.var log_rzero} is of the class {class(log_rzero)}."
    )
    names(local_bullets)[2] <- ifelse(length(log_rzero) > 1, "x", "i")
    names(local_bullets)[3] <- ifelse(inherits(log_rzero, "numeric"), "i", "x")
    cli::cli_abort(local_bullets)
  }

  # Extract necessary values from data
  n_years <- get_n_years(data)
  n_ages <- get_n_ages(data)

  # Set natural mortality rate
  M_value <- 0.2

  # Calculate initial numbers at age based on log_rzero and M_value
  init_naa <- exp(log_rzero) * exp(-(get_ages(data) - 1) * M_value)
  init_naa[n_ages] <- init_naa[n_ages] / M_value # sum of infinite series

  # Create a list of default parameters
  form_default <- list(
    log_M.value = rep(log(M_value), n_years * n_ages),
    log_M.estimatation_type = "constant",
    log_init_naa.value = log(init_naa),
    log_init_naa.estimatation_type = "fixed_effects"
  )

  # Name the list elements
  names(form_default) <- paste0("Population.", names(form_default))
  # Wrap the default parameters in a population1 list for output
  default <- list(list("population" = form_default))
  names(default) <- "population1"

  return(default)
}

#' Create default logistic parameters
#'
#' @description
#' This function sets up default parameters for a logistic function. There are
#' two specified parameters, the inflection point and slope.
#' @return
#' A list containing the default logistic parameters, with inflection_point and
#' slope values and their estimation status.
#' @noRd
create_default_Logistic <- function() {
  default <- list(
    inflection_point.value = 2,
    inflection_point.estimation_type = "fixed_effects",
    slope.value = 1,
    slope.estimation_type = "fixed_effects"
  )
  return(default)
}

#' Create default double logistic parameters
#'
#' @description
#' This function sets up default parameters for a double logistic function.
#' There four specified parameters, two for the ascending and two for the
#' descending inflection points and slopes.
#' @return
#' A list containing the default double logistic parameters,
#' inflection_point_asc, slope_asc, inflection_point_desc, and slope_desc
#' values and their estimation status.
#' @noRd
create_default_DoubleLogistic <- function() {
  logistic_defaults <- create_default_Logistic()
  default <- structure(
    rep(logistic_defaults, 2),
    names = c(
      gsub("\\.", "_asc.", names(logistic_defaults)),
      gsub("\\.", "_desc.", names(logistic_defaults))
    )
  )
  # TODO: Determine if this should really be 4?
  default[["inflection_point_desc.value"]] <- 4

  return(default)
}

#' Create default selectivity parameters
#'
#' @description
#' This function sets up default parameters for a selectivity module.
#' @param form A string specifying the desired form of selectivity. Allowable
#'   forms include `r toString(formals(create_default_selectivity)[["form"]])`
#'   and the default is
#'   `r toString(formals(create_default_selectivity)[["form"]][1])`.
#' @return
#' A list is returned with the default parameter values for the specified form
#' of selectivity.
#' @noRd
create_default_selectivity <- function(
    form = c("LogisticSelectivity", "DoubleLogisticSelectivity")) {
  # Input checks
  form <- rlang::arg_match(form)
  # NOTE: All new forms of selectivity must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  form_default <- switch(form,
    "LogisticSelectivity" = create_default_Logistic(),
    "DoubleLogisticSelectivity" = create_default_DoubleLogistic()
  )
  names(form_default) <- paste0(form, ".", names(form_default))
  default <- list(form_default)
  names(default) <- "selectivity"
  return(default)
}

#' Create default fleet parameters
#'
#' @description
#' This function sets up default parameters for a fleet module. It compiles
#' selectivity parameters along with distributions for each type of data that
#' are present for the given fleet.
#'
#' @param fleets A list of fleet configurations.
#' @param fleet_name A character. Name of the fleet.
#' @param data An S4 object. FIMS input data.
#' @return
#' A list with default parameters for the fleet.
#' @noRd
create_default_fleet <- function(fleets,
                                 fleet_name,
                                 data) {
  # Input checks
  if (length(fleet_name) > 1) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} should have a length of 1.",
      "x" = "{.var fleet_name} has a length of {length(fleet_name)}."
    ))
  }
  if (!inherits(fleet_name, "character")) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} should be a string.",
      "x" = "{.var fleet_name} is a {class(fleet_name)}."
    ))
  }
  if (!fleet_name %in% names(fleets)) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} should be present in the names of {.var fleets}.",
      "x" = "{.var {fleet_name}} is not in {names(fleets)}."
    ))
  }

  # Create default selectivity parameters
  selectivity_default <- create_default_selectivity(
    form = fleets[[fleet_name]][["selectivity"]][["form"]]
  )

  # Get types of data for this fleet from the data object
  data_types_present <- get_data(data) |>
    dplyr::filter(name == fleet_name) |>
    dplyr::pull(type) |>
    unique()

  # Determine default fleet parameters based on types of data present
  # FIXME: allow for a fleet to have both landings and index data
  form_default <- if ("landings" %in% data_types_present) {
    list(
      log_Fmort.value = log(rep(0.00001, get_n_years(data))),
      log_Fmort.estimation_type = "fixed_effects"
    )
  } else {
    list(
      log_q.value = 0,
      log_q.estimation_type = "fixed_effects"
    )
  }

  names(form_default) <- paste0("Fleet.", names(form_default))
  fleet_default = list(form_default)
  names(fleet_default) <- "fleet"

  # Create index distribution defaults
  index_distribution <- fleets[[fleet_name]][["data_distribution"]]["Index"]

  # FIXME: Will this work if both landings and index data are present?
  index_uncertainty <- get_data(data) |>
    dplyr::filter(name == fleet_name, type %in% c("landings", "index")) |>
    dplyr::arrange(dplyr::desc(type)) |>
    dplyr::pull(uncertainty)

  index_distribution_default <- switch(index_distribution,
    "DnormDistribution" = create_default_DnormDistribution(
      value = index_uncertainty,
      input_type = "data",
      data = data
    ),
    "DlnormDistribution" = create_default_DlnormDistribution(
      value = index_uncertainty,
      input_type = "data",
      data = data
    )
  )
  names(index_distribution_default) <- paste0(
    index_distribution,
    ".",
    names(index_distribution_default)
  )
  index_distribution_default <- list("distribution" = index_distribution_default)

  # Compile all default parameters into a single list
  default <- list(c(
    selectivity_default,
    fleet_default,
    index_distribution_default
  ))

  names(default) <- fleet_name
  return(default)
}

#' Create default maturity parameters
#'
#' @description
#' This function sets up default parameters for a maturity module.
#' @param form A string specifying the form of maturity (e.g.,
#' `"LogisticMaturity"`).
#' @return
#' A list containing the default maturity parameters.
#' @noRd
create_default_maturity <- function(form = c("LogisticMaturity")) {
  # Input checks
  form <- rlang::arg_match(form)

  # NOTE: All new forms of maturity must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  form_default <- list(
    "maturity" = switch(form,
      "LogisticMaturity" = create_default_Logistic()
    )
  )
  names(form_default) <- paste0(form, ".", names(form_default))
  # Wrap the default parameters in a population1 list for output
  default <- list(list("maturity" = form_default))
  names(default) <- "population1"

  return(default)
}

#' Create default Beverton--Holt recruitment parameters
#'
#' @description
#' This function sets up default parameters for a Beverton--Holt recruitment
#' relationship. Parameters include the natural log of unfished recruitment,
#' the logit transformation of the slope of the spawner--recruitment curve to
#' keep it between zero and one, and the time series of spawner--recruitment
#' deviations on the natural log scale.
#' @param data An S4 object. FIMS input data.
#' @return
#' A list containing default recruitment parameters.
#' @noRd
create_default_BevertonHoltRecruitment <- function(data) {
  # Create default parameters for Beverton--Holt recruitment
  default <- list(
    log_rzero.value = log(1e+06),
    log_rzero.estimation_type = "fixed_effects",   
    log_r.value = rep(0.0, get_n_years(data) - 1),
    log_r.estimation_type = "constant",
    logit_steep.value = -log(1.0 - 0.75) + log(0.75 - 0.2),
    logit_steep.estimation_type = "constant",
    log_devs.value = rep(0.0, get_n_years(data) - 1),
    log_devs.estimation_type = "fixed_effects"
  )
  return(default)
}

#' Create default DnormDistribution parameters
#'
#' @description
#' This function sets up default parameters to calculate the density of a
#' normal distribution, i.e., `DnormDistribution`, module.
#' @param value A real number that is passed to `log_sd`. The default value is
#'   `0.1`.
#' @param data An S4 object. FIMS input data.
#' @param input_type A string specifying the input type. The available options
#'   are
#'   `r toString(formals(create_default_DnormDistribution)[["input_type"]])`.
#'   The default is
#'   `r toString(formals(create_default_DnormDistribution)[["input_type"]][1])`.
#' @return
#' A list of default parameters for DnormDistribution.
#' @noRd
create_default_DnormDistribution <- function(
    value = 0.1,
    data,
    input_type = c("data", "process", "prior")) {
  # Input checks
  input_type <- rlang::arg_match(input_type)

  # Create default parameters
  default <- list(
    log_sd.value = value,
    log_sd.estimation_type = "constant"
  )

  # If input_type is 'process', add additional parameters
  if (input_type == "process"| input_type == "prior") {
    default <- c(
      default,
      list(
        x.value = rep(0, get_n_years(data)),
        x.estimation_type = "constant",
        expected_values.value = rep(0, get_n_years(data)),
        expected_values.estimation_type = "constant"
      )
    )
  }
  return(default)
}

#' Create default DlnormDistribution parameters
#'
#' @description
#' This function sets up default parameters to calculate the density of a
#' log-normal distribution, i.e., `DlnormDistribution`, module.
#' @param value Default value for `log_sd`.
#' @param data An S4 object. FIMS input data.
#' @param input_type A string specifying the input type. The available options
#'   are
#'   `r toString(formals(create_default_DlnormDistribution)[["input_type"]])`.
#'   The default is
#'   `r toString(formals(create_default_DlnormDistribution)[["input_type"]][1])`.
#' @return
#' A list of default parameters for DlnormDistribution.
#' @noRd
create_default_DlnormDistribution <- function(
    value = 0.1,
    data,
    input_type = c("data", "process")) {
  # Input checks
  # TODO: Determine if value can be a vector?
  if (!is.numeric(value) || any(value <= 0, na.rm = TRUE)) {
    cli::cli_abort(c(
      "i" = "Inputs to {.var value} must be positive and numeric.",
      "x" = "{.var value} is {.var {value}}."
    ))
  }
  input_type <- rlang::arg_match(input_type)

  # Create the default list with log standard deviation
  default <- list(
    log_sd.value = log(value),
    log_sd.estimation_type = "constant"
  )

  # Add additional parameters if input_type is "process"
  if (input_type == "process") {
    default <- c(
      default,
      list(
        x.value = rep(0, get_n_years(data)),
        x.estimation_type = "constant",
        expected_values.value = rep(0, get_n_years(data)),
        expected_values.estimation_type = "constant"
      )
    )
  }
  return(default)
}

#' Create default recruitment parameters
#'
#' @description
#' This function sets up default parameters for a recruitment module.
#'
#' @param recruitment A list with recruitment details
#' @param data An S4 object. FIMS input data.
#' @param input_type A string specifying the type of recruitment you want to
#'   use. The available options are
#'   `r toString(formals(create_default_recruitment)[["input_type"]])`. The
#'   default is
#'   `r toString(formals(create_default_recruitment)[["input_type"]][1])`.
#' @return
#' A list with the default parameters for recruitment.
#' @noRd
create_default_recruitment <- function(
    recruitment,
    data,
    input_type = "BevertonHoltRecruitment") {
  # Input checks
  if (!is.list(recruitment)) {
    cli::cli_abort(c(
      "i" = "The {.var recruitment} argument must be a list.",
      "x" = "{.var recruitment} is a {class(recruitment)}."
    ))
  }
  form <- rlang::arg_match(input_type)
  # Create default parameters based on the recruitment form
  # NOTE: All new forms of recruitment must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  form_default <- switch(form,
    "BevertonHoltRecruitment" = create_default_BevertonHoltRecruitment(data)
  )
  names(form_default) <- paste0(form, ".", names(form_default))
  # Wrap the default parameters in a population1 list for output
  default <- list(list("recruitment" = form_default))
  names(default) <- "population1"

  return(default)

}

#' Create default process for a FIMS model
#'
#' @description
#' This function generates default process settings for a Fisheries
#' Integrated Modeling System (FIMS) model, including distribution name, estimation 
#' and random settings. It applies default configurations when specific module settings 
#' are not provided by the user.
#' @param parent_module A string specifying the parent module name or names.
#' @param module Name of the process module.
#' @param form A string specifying the form of the process.
#' @param process object return from process_model function. 
#' @return
#' A list containing the following two entries:
#' \describe{
#'   \item{\code{parameters}:}{A list of parameter inputs for the FIMS
#'     model.}
#'   \item{\code{modules}:}{A list of modules with default or user-provided
#'     settings.}
#' }
#' @seealso
#' * [create_default_parameters()]
#' * [update_parameters()]
#' * [process_model()]
#' @export
update_process <- function(input, data, parent_module = "population1", 
                            module, form, process){

  #TODO: update form if user wants to change it from the default
 #input[[parent_module]][[module]]...
  
  # Create default distribution parameters based on the distribution type
  distribution_input <- process$method
  distribution_default <- NULL
  if(!is.null(process$init)){
    for(i in seq_along(init)){
      input[["parameters"]][[parent_module]][[module]][[form]][[par]][i][["value"]] <- process$init[i]
    }
  }
  if (distribution_input != "Fixed Effect") {
    input[["modules"]][[module]][["process_distribution"]]  <- switch(process$type,
      "iid" = "DnormDistribution"
    )
    names(input[["modules"]][[module]][["process_distribution"]]) <- process$par
    distribution_default <- switch(process$type,
      "iid" = create_default_DnormDistribution(
        data = data,
        input_type = "process"
      )
    )

    if(distribution_input == "Penalized MLE"){
      input[["parameters"]][[module]][[process$par]][["estimation_type"]] <- "fixed_effects"
      distribution_default$log_sd.value <- log(process$sd)
    }
    if(distribution_input == "Random Effect ConstVar"){
      input[["parameters"]][[parent_module]][[module]][[process$par]][["estimation_type"]] <- "random_effects"
      distribution_default$log_sd.value <- log(process$sd)
    }
    if(distribution_input == "Random Effect Full"){
      input[["parameters"]][[parent_module]][[module]][[process$par]][["estimation_type"]] <- "random_effects"
      if(!is.null(process$sd)){
        distribution_default$log_sd.value <- log(process$sd)
      }
    }

    distribution_name <- switch(process$type,
      "iid" = "DnormDistribution"
    )
    names(distribution_default) <- paste0(
      distribution_name,
      ".",
      names(distribution_default)
    )
  }

  input <- c(input, distribution_default)

  return(input)
}
#' Update input parameters for a FIMS model
#'
#' @description
#' This function updates the input parameters of a Fisheries Integrated
#' Modeling System (FIMS) model. It allows users to modify specific parameters
#' by providing new values, while retaining the existing modules information
#' from the current input.
#' @param current_parameters A list containing the current input parameters, including:
#'   \describe{
#'     \item{\code{parameters}:}{A list of parameter inputs.}
#'     \item{\code{modules}:}{A list of module names used in the model.}
#'   }
#' @param modified_parameters A named list representing new parameter values to update.
#' @rdname create_default_parameters
#' @return
#' A list containing:
#' \describe{
#'   \item{parameters}{A list of updated parameter inputs that
#'   includes any modifications made by the user.}
#'   \item{modules}{The unchanged list of module names from the current
#'   input.}
#' }
#' @seealso
#' * [create_default_parameters()]
#' @export
#' @examples
#' \dontrun{
#' # Load the example dataset
#' data("data1")
#' fims_frame <- FIMSFrame(data1)
#'
#' # Define fleets specifications
#' fleet1 <- survey1 <- list(
#'   selectivity = list(form = "LogisticSelectivity"),
#'   data_distribution = c(
#'     Index = "DlnormDistribution",
#'     AgeComp = "DmultinomDistribution"
#'   )
#' )
#'
#' # Create default parameters for the specified fleets
#' default_parameters <- fims_frame |>
#'   create_default_parameters(
#'     fleets = list(fleet1 = fleet1, survey1 = survey1)
#'   )
#' 
#' updated_parameters <- default_parameters |>
#'   update_parameters(
#'      modified_parameters = list(
#'        fleet1 = list(
#'          Fleet.log_Fmort.value = log(c(
#'            0.009459165, 0.027288858, 0.045063639,
#'            0.061017825, 0.048600752, 0.087420554,
#'            0.088447204, 0.186607929, 0.109008958,
#'            0.132704335, 0.150615473, 0.161242955,
#'            0.116640187, 0.169346119, 0.180191913,
#'            0.161240483, 0.314573212, 0.257247574,
#'            0.254887252, 0.251462108, 0.349101406,
#'            0.254107720, 0.418478117, 0.345721184,
#'            0.343685540, 0.314171227, 0.308026829,
#'            0.431745298, 0.328030899, 0.499675368
#'          ))
#'        )
#'      )
#'   )
#'
#' # purrr::map_vec() can be used to compare the length of adjusted parameter vectors with defaults for a specific module (e.g., fleet1)
#' default_fleet1 <- purrr::map_vec(default_parameters[["parameters"]][["fleet1"]], \(x) length(x))
#' updated_fleet1 <- purrr::map_vec(updated_parameters[["parameters"]][["fleet1"]], \(x) length(x))
#'
#' # purrr::map_df() can be used to summarize parameter vector lengths across all modules
#' purrr::map_df(
#'   updated_parameters[["parameters"]], \(x) purrr::map_vec(x, length),
#'   .id = "module"
#' ) |>
#'   tibble::column_to_rownames(var = "module") |>
#'   t()
#' }
update_parameters <- function(current_parameters, modified_parameters) {
  # Input checks
  # Check if current_parameters is a list with required components
  if (
    !is.list(current_parameters) ||
      !all(c("parameters", "modules") %in% names(current_parameters))
  ) {
    cli::cli_abort(c(
      "i" = "{.var current_parameters} argument must be a list containing
            parameters and modules.",
      "x" = "{.var current_parameters} is a {class(current_parameters)}."
    ))
  }
  # Check if modified_parameters is a named list
  if (!is.list(modified_parameters) || is.null(names(modified_parameters))) {
    cli::cli_abort(c(
      "i" = "{.var modified_parameters} argument must be must be a named list.",
      "x" = "{.var modified_parameters} is a {class(modified_parameters)}."
    ))
  }

  # Check if modified_parameters exists in current_parameters
  missing_input <- setdiff(
    names(modified_parameters),
    names(current_parameters[["parameters"]])
  )
  if (length(missing_input) > 0) {
    cli::cli_abort(c(
      "x" = "The following {length(missing_input)} input list{?s} from
            {.var modified_parameters} {?is/are} missing from
            {.var current_parameters}: {missing_input}."
    ))
  }

  wrong_input <- setdiff(
    names(current_parameters[["parameters"]]),
    names(modified_parameters)
  )
  if (length(missing_input) > 0) {
    cli::cli_abort(c(
      "x" = "The following {length(missing_input)} input list{?s} from
            {.var modified_parameters} {?is/are} missing from
            {.var current_parameters}: {missing_input}."
    ))
  }

  new_param_input <- current_parameters[["parameters"]]
  parent_module_names <- names(new_param_input)

  # Update parameters for each module based on modified_parameters
  for (i in seq_along(parent_module_names)) {
    if(parent_module_names[i] %in% names(modified_parameters)) {
      module_names <- names(current_parameters[["parameters"]][[parent_module_names[i]]])
      for(module_name in module_names){
        if (module_name %in% names(modified_parameters[[parent_module_names[i]]])) {
          modified_params <- modified_parameters[[parent_module_names[i]]][[module_name]]
          current_params <- new_param_input[[parent_module_names[i]]][[module_name]]
          for (param_name in names(modified_params)) {
            # Check if the parameter exists in current_parameters
            if (!param_name %in% names(current_params)) {
              cli::cli_abort(c(
                "x" = "{param_name} from {module_name} in {.var modified_parameters}
                            does not exist in {.var current_parameters}."
              ))
            }

            # TODO: default is a scalar but user might want to modify to time-varying,
            # e.g. log_q, log_r
            # Check if the length of the modified and current parameter match
            length_modified_parameter <- length(modified_params[[param_name]])
            length_current_parameter <- length(current_params[[param_name]])
            if (!identical(length_modified_parameter, length_current_parameter)) {
              cli::cli_abort(c(
                "x" = "The length of {.var {param_name}} from {module_name}
                      does not match between {.var modified_parameters} and
                      {.var current_parameters}.",
                "i" = "The parameter name of interest is {.var {param_name}}.",
                "i" = "The length of the modified parameter is
                      {length_modified_parameter}.",
                "i" = "The length of the current parameter is
                      {length_current_parameter}."
              ))
            }

            # Check if the type of the modified and current parameter match
            if (!identical(
              typeof(modified_params[[param_name]]),
              typeof(current_params[[param_name]])
            )) {
              cli::cli_abort(c(
                "x" = "The type of {param_name} from {module_name} does not match
                      between {.var modified_parameters} and
                      {.var current_parameters}."
              ))
            }

            # Update the parameter if checks pass
            current_params[[param_name]] <- modified_params[[param_name]]
          }
          # Assign the updated module parameters back to new_param_input
          new_param_input[[parent_module_names[i]]][[module_name]] <- current_params
        }
      }
    }
  }
  # Create a new list for updated input
  new_input <- list(
    parameters = new_param_input,
    modules = current_parameters$modules
  )
  return(new_input)
}

