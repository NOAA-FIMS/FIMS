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
#'     recruitment = list(
#'       form = "BevertonHoltRecruitment",
#'       process_distribution = c(log_devs = "DnormDistribution")
#'     ),
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
#'   tidyr::pivot_longer(cols = tidyselect::everything())
#' updated_fleet1 <- purrr::map_df(
#'   parameters_with_double_logistic[["parameters"]][["fleet1"]],
#'   \(x) length(x)
#' ) |>
#'   tidyr::pivot_longer(cols = tidyselect::everything())
#' dplyr::full_join(default_fleet1, updated_fleet1, by = "name")
#' knitr::kable(dplyr::full_join(default_fleet1, updated_fleet1, by = "name"))
#' }
create_default_parameters <- function(
    data,
    fleets,
    recruitment = list(
      form = "BevertonHoltRecruitment",
      process_distribution = c(log_devs = "DnormDistribution")
    ),
    # TODO: Rename EWAAgrowth to not use an acronym
    growth = list(form = "EWAAgrowth"),
    maturity = list(form = "LogisticMaturity")) {
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
    recruitment = recruitment,
    growth = growth,
    maturity = maturity
  )

  # Create fleet parameters
  fleet_temp <- list()
  for (i in 1:length(fleets)) {
    fleet_temp <- c(
      fleet_temp,
      create_default_fleet(
        fleets = fleets,
        fleet_name = names(fleets)[i],
        data = data
      )
    )
  }

  # Create recruitment parameters
  recruitment_temp <- create_default_recruitment(
    recruitment = recruitment,
    data = data,
    input_type = recruitment[["form"]]
  )

  # Create maturity parameters
  maturity_temp <- create_default_maturity(form = maturity$form)

  # Create population parameters
  # Handle population parameters based on recruitment form
  if (recruitment[["form"]] == "BevertonHoltRecruitment") {
    log_rzero <- recruitment_temp[["recruitment"]][[
      paste0(recruitment[["form"]], ".log_rzero.value")
    ]]
  }
  population_temp <- create_default_Population(data, log_rzero)

  # Compile output
  output <- list(
    parameters = c(
      fleet_temp,
      recruitment_temp,
      maturity_temp,
      population_temp
    ),
    modules = module_list
  )
  return(output)
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
    inflection_point.estimated = TRUE,
    slope.value = 1,
    slope.estimated = TRUE
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
  default <- switch(form,
    "LogisticSelectivity" = create_default_Logistic(),
    "DoubleLogisticSelectivity" = create_default_DoubleLogistic()
  )
  names(default) <- paste0(form, ".", names(default))

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

  #Get data likelihood distributions assigned for this fleet
  distribution_names_for_fleet <- names(fleets[[fleet_name]][["data_distribution"]])

  # Determine default fleet parameters based on types of data present
  if ("index" %in% data_types_present &&
                   "Index" %in% distribution_names_for_fleet) {
     q_default <- list(
      log_q.value = 0,
      log_q.estimated = TRUE
     )

     index_distribution <- fleets[[fleet_name]][["data_distribution"]]["Index"]

     index_uncertainty <- get_data(data) |>
       dplyr::filter(name == fleet_name, type %in% c("index")) |>
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

  }else{
    q_default <- list(
      log_q.value = 0,
      log_q.estimated = FALSE
    )

    index_distribution_default <- NULL
  }

  names(q_default) <- paste0("Fleet.", names(q_default))

  if ("landings" %in% data_types_present &&
                   "Landings" %in% distribution_names_for_fleet) {
    log_Fmort_default <- list(
      log_Fmort.value = rep(-3, get_n_years(data)),
      log_Fmort.estimated = TRUE
    )

    landings_distribution <- fleets[[fleet_name]][["data_distribution"]]["Landings"]

    landings_uncertainty <- get_data(data) |>
      dplyr::filter(name == fleet_name, type %in% c("landings")) |>
      dplyr::arrange(dplyr::desc(type)) |>
      dplyr::pull(uncertainty)

    landings_distribution_default <- switch(landings_distribution,
                                         "DnormDistribution" = create_default_DnormDistribution(
                                           value = landings_uncertainty,
                                           input_type = "data",
                                           data = data
                                         ),
                                         "DlnormDistribution" = create_default_DlnormDistribution(
                                           value = landings_uncertainty,
                                           input_type = "data",
                                           data = data
                                         )
    )
    names(landings_distribution_default) <- paste0(
      landings_distribution,
      ".",
      names(landings_distribution_default)
    )

  } else {
    log_Fmort_default <- list(
      log_Fmort.value = rep(-200, get_n_years(data)),
      log_Fmort.estimated = FALSE
    )

    landings_distribution_default <- NULL
  }

  names(log_Fmort_default) <- paste0("Fleet.", names(log_Fmort_default))

  # Compile all default parameters into a single list
  default <- list(c(
    selectivity_default,
    q_default,
    log_Fmort_default,
    index_distribution_default,
    landings_distribution_default
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
  default <- list(
    "maturity" = switch(form,
      "LogisticMaturity" = create_default_Logistic()
    )
  )
  names(default[["maturity"]]) <- paste0(form, ".", names(default[["maturity"]]))

  return(default)
}

#' Create default Beverton--Holt recruitment parameters
#'
#' @description
#' This function sets up default parameters for a Beverton--Holt recruitment
#' relationship. Parameters include the natural log of unfished recruitment,
#' the logit transformation of the slope of the stock--recruitment curve to
#' keep it between zero and one, and the time series of stock--recruitment
#' deviations on the natural log scale.
#' @param data An S4 object. FIMS input data.
#' @return
#' A list containing default recruitment parameters.
#' @noRd
create_default_BevertonHoltRecruitment <- function(data) {
  # Create default parameters for Beverton--Holt recruitment
  default <- list(
    log_rzero.value = log(1e+06),
    log_rzero.estimated = TRUE,
    logit_steep.value = -log(1.0 - 0.75) + log(0.75 - 0.2),
    logit_steep.estimated = FALSE,
    log_devs.value = rep(0.0, get_n_years(data) - 1),
    log_devs.estimated = TRUE,
    estimate_log_devs = TRUE
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
    input_type = c("data", "process")) {
  # Input checks
  input_type <- rlang::arg_match(input_type)

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
        x.value = rep(0, get_n_years(data)),
        x.estimated = FALSE,
        expected_values.value = rep(0, get_n_years(data)),
        expected_values.estimated = FALSE
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
    log_sd.estimated = FALSE
  )

  # Add additional parameters if input_type is "process"
  if (input_type == "process") {
    default <- c(
      default,
      list(
        x.value = rep(0, get_n_years(data)),
        x.estimated = FALSE,
        expected_values.value = rep(0, get_n_years(data)),
        expected_values.estimated = FALSE
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
#' @param recruitment A list with recruitment details, including form and
#'   process distribution type.
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
  process_default <- switch(form,
    "BevertonHoltRecruitment" = create_default_BevertonHoltRecruitment(data)
  )
  names(process_default) <- paste0(form, ".", names(process_default))

  # Create default distribution parameters based on the distribution type
  distribution_input <- recruitment[["process_distribution"]]
  distribution_default <- NULL
  if (!is.null(distribution_input)) {
    distribution_default <- switch(distribution_input,
      "DnormDistribution" = create_default_DnormDistribution(
        data = data,
        input_type = "process"
      )
    )
    names(distribution_default) <- paste0(
      distribution_input,
      ".",
      names(distribution_default)
    )
  }

  # Combine process and distribution defaults into a single list
  default <- list(c(process_default, distribution_default))
  names(default) <- "recruitment"
  return(default)
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
#'     modified_parameters = list(
#'       fleet1 = list(
#'         Fleet.log_Fmort.value = log(c(
#'           0.009459165, 0.027288858, 0.045063639,
#'           0.061017825, 0.048600752, 0.087420554,
#'           0.088447204, 0.186607929, 0.109008958,
#'           0.132704335, 0.150615473, 0.161242955,
#'           0.116640187, 0.169346119, 0.180191913,
#'           0.161240483, 0.314573212, 0.257247574,
#'           0.254887252, 0.251462108, 0.349101406,
#'           0.254107720, 0.418478117, 0.345721184,
#'           0.343685540, 0.314171227, 0.308026829,
#'           0.431745298, 0.328030899, 0.499675368
#'         ))
#'       )
#'     )
#'   )
#'
#' # purrr::map_vec() can be used to compare the length of adjusted parameter 
#' # vectors with defaults for a specific module (e.g., fleet1)
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
 # TODO: is this duplicated code or is this testing something else? If not
 # duplicated, should missing_input be wrong_input?
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
  module_names <- names(new_param_input)

  # Update parameters for each module based on modified_parameters
  for (module_name in module_names) {
    if (module_name %in% names(modified_parameters)) {
      modified_params <- modified_parameters[[module_name]]
      current_params <- new_param_input[[module_name]]

      for (param_name in names(modified_params)) {
        # Check if the parameter exists in current_parameters
        if (!param_name %in% names(current_params)) {
          cli::cli_abort(c(
            "x" = "{param_name} from {module_name} in {.var modified_parameters}
                         does not exist in {.var current_parameters}."
          ))
        }

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
      new_param_input[[module_name]] <- current_params
    }
  }
  # Create a new list for updated input
  new_input <- list(
    parameters = new_param_input,
    modules = current_parameters$modules
  )
  return(new_input)
}
