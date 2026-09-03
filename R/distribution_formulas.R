#' Validity checks for distributions
#'
#' This function checks the validity of arguments passed to functions that
#' relate to distributions within the Fisheries Integrated Modeling System
#' (FIMS). This function is designed to fail early only once, otherwise it goes
#' through many checks before reporting the results in an attempt to give the
#' user the most information possible. If it were to fail on every mistake,
#' then the user might have to iterate through multiple changes to their input
#' values. Sometimes, their mistakes might take quite a bit of time to make it
#' to this function or worse they might be running things on the cloud and not
#' have immediate access to the report. Therefore, we feel that providing the
#' most information possible is the best way forward.
#'
#' @param args A named list of input arguments that must contain at least
#'   `family` and `sd`. `data_type` is only needed for some upstream functions.
#' @seealso
#' This function is used in the following functions:
#' * [initialize_data_distribution()]
#' * [initialize_process_distribution()]
#' @noRd
#' @return
#' If successful, `TRUE` is invisibly returned. If unsuccessful,
#' [cli::cli_abort()] is used to return the relevant error messages.
check_distribution_validity <- function(args) {
  # Separate objects from args
  family <- args[["family"]]
  sd <- args[["sd"]]
  check_present <- purrr::map_vec(list("family" = family, "sd" = sd), is.null)
  
  # Only process distributions are currently validated here.
  available_distributions <- c(
    "gaussian", "lognormal", "dnorm", "dlnorm", "Dnorm", "Dlnorm"
  )
  elements_of_sd <- c("value", "estimation_status")

  # Start a bulleted list of errors and add to it in each if statement
  abort_bullets <- c(
    " " = "The following errors were found in the input argument {.var args}."
  )
  if (any(check_present)) {
    bad <- names(check_present[unlist(check_present)])
    abort_bullets <- c(
      abort_bullets,
      "x" = "{.var {bad}} {cli::qty(length(bad))} {?is/are} missing from
             {.var args}."
    )
    # Abort early because not all of the necessary items were in args
    cli::cli_abort(abort_bullets)
  }

  # Checks related to the family class
  if (inherits(family, "family")) {
    family_name <- family[["family"]]
  } else if (is.character(family) && length(family) == 1L && !is.na(family)) {
    family_name <- family
  } else {
    abort_bullets <- c(
      abort_bullets,
      "x" = "The class of {.var family} is incorrect.",
      "i" = "{.var family} should be a distribution name or an object of
             class {.var family}, e.g., `family = 'dnorm'` or
             `family = gaussian()`, instead of {class(family)}."
    )
  }
  if (exists("family_name", inherits = FALSE)) {
    if (!(family_name %in% available_distributions)) {
      abort_bullets <- c(
        abort_bullets,
        "x" = "FIMS currently does not allow the family to be
              {.code {family_name}}.",
        "i" = "The families available for process distributions are
              {.code {available_distributions}}."
      )
    }
  }

  # Checks related to standard deviation
  # Check if sd has both elements and if yes, then go onto the else statement
  # for major checks
  if (!all(elements_of_sd %in% names(sd))) {
    abort_bullets <- c(
      abort_bullets,
      "x" = "{.var {elements_of_sd}} need to be present in sd.",
      "i" = "Only {.code {names(sd)}} {cli::qty(length(sd))} {?is/are} present."
    )
  } else {
    if (!all(sd[["value"]] > 0, na.rm = TRUE)) {
      abort_bullets <- c(
        abort_bullets,
        "x" = "Values passed to {.var sd} are out of bounds.",
        "i" = "Values passed to {.var sd} {cli::qty(length(sd[['value']]))}
               {?is/are} {.code {sd[['value']]}}.",
        "i" = "All standard deviation (sd) values need to be positive."
      )
    }
    if (
      length(sd[["estimation_status"]]) > 1 &&
        length(sd[["value"]]) != length(sd[["estimation_status"]])
    ) {
      sd_length <- length(sd[["value"]])
      est_length <- length(sd[["estimation_status"]])
      abort_bullets <- c(
        abort_bullets,
        "x" = "The sizes of {.var value} and {.var estimation_status} within {.var sd}
               must match if more than one value is specified for the latter.",
        "i" = "The length of {.var sd[['value']]} is {.code {sd_length}}.",
        "i" = "The length of {.var sd[['estimation_status']]} is
               {.code {est_length}}."
      )
    }
  }

  # Return error messages if more than just the default is present
  if (length(abort_bullets) == 1) {
    invisible(TRUE)
  } else {
    cli::cli_abort(abort_bullets)
  }
}

#' Set up a distribution for a fleet's data
#'
#' This helper creates a distribution module and wires it to the observed data
#' carried by a fleet. 
#'
#' @param module A fleet [fims_module], created with [create_fleet()] and
#'   linked to the data of interest.
#' @param data_type A string specifying the observed data type the distribution
#'   applies to. Allowable types include
#'   `r glue::glue_collapse(sprintf('"%s"', eval(formals(initialize_data_distribution)[["data_type"]])), sep = ", ", last = ", and ")`
#'   and the default is
#'   `r eval(formals(initialize_data_distribution)[["data_type"]])[1]`.
#' @param uncertainty A character vector of formulas describing the uncertainty
#'   model for each observation. Each string is parsed by
#'   `parse_data_distribution()` and checked for valid families before the
#'   distribution is linked to the fleet's observed data. These formulas should
#'   name the same quantities the model expects to read from the fleet or
#'   associated modules.
#' @return A [fims_module] distribution object. Internally it holds the C++
#'   external pointer that FIMS uses to represent the distribution, and it is
#'   registered with the model so it can be included when the model is built.
#' @keywords distribution
#' @export
#' @seealso
#' * [create_distribution()]
#' * [set_distribution_links()]
#' * [initialize_process_distribution()]
#' @examples
#' \dontrun{
#' # Create a fleet and the observed index data it carries.
#' fleet <- create_fleet()
#' index_data <- create_data("index", n_years = 30)
#' set_fleet_observed_data(fleet, index = index_data)
#'
#' # Create a distribution for the fleet's index observations.
#' sd_log <- rep(sqrt(log(0.01^2 + 1)), 30)
#' fleet_distribution <- initialize_data_distribution(
#'   module = fleet,
#'   uncertainty = glue::glue(
#'     "~dlnorm(meanlog = log_index_expected, sdlog = {sd_log})"
#'   ),
#'   data_type = "index"
#' )
#' }
initialize_data_distribution <- function(
  module,
  data_type = c("catch", "index", "age_comp", "length_comp"),
  uncertainty
) {
  data_type <- rlang::arg_match(data_type)
  uncertainty_split <- parse_data_distribution(uncertainty) |>
    validate_distribution_families()
  distribution_family <- unique(stats::na.omit(uncertainty_split[["family"]]))

  # Set up distribution based on `family` argument`
  maker <- .fims_module_names[["Distribution"]]
  if (is.null(maker)) {
    cli::cli_abort(c(
      "{.val {module_name}} is not a module FIMS knows how to create.",
      "i" = "Add it to {.var .fims_module_names} in {.file R/fims_interface.R}."
    ))
  }
  distribution_module <- maker[["create"]](
    if (length(distribution_family) == 1) distribution_family else NA_character_
  )

  ancillary_parameter <- dplyr::filter(
    FIMS::fims_distributions,
    .data$family == .env$distribution_family
  ) |>
    dplyr::pull(.data$other_parameters) |>
    unlist()

  if (ancillary_parameter == "sdlog" | ancillary_parameter == "sd") {
    set_variable_vector(distribution_module, "log_sd",
      unlist(uncertainty_split[[ancillary_parameter]], use.names = FALSE) |> log(),
      estimation_status = "assumed_known"
    )
  }

  # setup link to observed data
  set_distribution_observed_data(
    distribution_module,
    get_fleet_observed_data_ids(module)[[data_type]]
  )

  # setup link to expected values
  set_distribution_links( distribution_module, "data",
    get_variable_vector_id(module,
                           unique(stats::na.omit(uncertainty_split[["link"]])))
  )
  
  return(distribution_module)
}

#' Set up a new distribution for a process
#'
#' Use [methods::new()] to set up a distribution within an existing module with
#' the necessary linkages between the two. For example, a recruitment
#' module, like the Beverton--Holt stock--recruit relationship, will need a
#' distribution associated with the recruitment deviations.
#' @param module An identifier to a C++ fleet module that is linked to the data
#'   of interest.
#' @param family A description of the error distribution and link function to
#'   be used in the model. The argument takes a family class, e.g.,
#'   `stats::gaussian(link = "identity")`.
#' @param sd A list of length two. The first entry is named `"value"` and it
#'   stores the initial values (scalar or vector) for the relevant standard
#'   deviations. The default is `value = 1`. The second entry is named
#'  `"estimation_status"` and it stores a vector of booleans (default =
#'   "assumed_known") is a string indicating whether or not standard deviation is
#'   estimated as a fixed effect or assumed known. If `"value"` is a vector and
#'   `"estimation_status"` is a scalar, the single value specified
#'   `"estimation_status"` value will be repeated to match the length of `value`.
#'   Otherwise, the dimensions of the two must match.
#' @param par A string specifying the parameter name the distribution applies
#'   to. Parameters must be members of the specified module. Use
#'   `methods::show(module)` to obtain names of parameters within the module.
#' @return
#' A reference class. is returned. Use [methods::show()] to view the various
#' Rcpp class fields, methods, and documentation.
#' @keywords distribution
#' @export
#' @seealso
#' * [initialize_data_distribution()]
#' @examples
#' \dontrun{
#' # Set up a new process distribution
#' # Create a new recruitment module
#' recruitment <- methods::new(BevertonHoltRecruitment)
#' # view parameter names of the recruitment module
#' methods::show(BevertonHoltRecruitment)
#' # Create a distribution for the recruitment module
#' recruitment_distribution <- initialize_process_distribution(
#'   module = recruitment,
#'   par = "log_devs",
#'   family = gaussian(),
#'   sd = list(value = 0.4, estimation_status = "assumed_known")
#' )
#' }
initialize_process_distribution <- function(
  module,
  par,
  family = "dnorm",
  sd = tibble::tibble(
    value = 1,
    estimation_status = "fixed_effects"
  )
) {
  # validity check on user input
  args <- list(family = family, sd = sd)
  check_distribution_validity(args)

  if (!is.element(par, c("log_devs", "log_r"))) {
    return()
  }
  expected <- switch(paste0(par),
    "log_devs" = NULL,
    "log_r" = "log_expected_recruitment"
  )

  distribution_family <- if (inherits(family, "family")) {
    switch(family[["family"]],
      gaussian = "dnorm",
      lognormal = "dlnorm",
      family[["family"]]
    )
  } else {
    switch(family,
      Dnorm = "dnorm",
      Dlnorm = "dlnorm",
      family
    )
  }

  # Set up distribution based on `family` argument`
  maker <- .fims_module_names[["Distribution"]]
  if (is.null(maker)) {
    cli::cli_abort(c(
      "{.val {module_name}} is not a module FIMS knows how to create.",
      "i" = "Add it to {.var .fims_module_names} in {.file R/fims_interface.R}."
    ))
  }
  distribution_module <- maker[["create"]](
    if (length(distribution_family) == 1) distribution_family else NA_character_
  )

  
  if (distribution_family == "dnorm" | distribution_family == "dlnorm") {
    set_variable_vector(distribution_module, "log_sd",
      sd[["value"]] |> log(),
      estimation_status = sd[["estimation_status"]]
    )
  }

    #   if (length(sd[["value"]]) > 1 && length(sd[["estimation_status"]]) == 1) {
    #     if (sd[["estimation_status"]] == "assumed_known") {
    #       new_module$log_sd$set_estimation_status(c("assumed_known"))
    #     } else {
    #       new_module$log_sd$set_estimation_status(c("fixed_effects"))
    #     }
    #   } else {
    #     for (i in seq_along(sd[["estimation_status"]])) {
    #       new_module$log_sd[i]$set_estimation_status(sd[["estimation_status"]][i])
    #     }
    #   }
    # }

  n_dim <- length(get_variable_vector(module, par)[["values"]])

  # set distribution observed and expected values to 0
  set_variable_vector(distribution_module, "observed_values", 
                      rep(0, n_dim), "derived_quantity")
  set_variable_vector(distribution_module, "expected_values", 
                      rep(0, n_dim), "derived_quantity")

  # setup links to parameter
  if (is.null(expected)) {
    set_distribution_links(distribution_module, 
                           "random_effects",
                           get_variable_vector_id(module, par)
    )
  } else {
    set_distribution_links(distribution_module, 
                           "random_effects",
                           c(get_variable_vector_id(module, par),
                             get_variable_vector_id(module, expected))
    )
  }

  return(distribution_module)
}

#' @rdname initialize_process_distribution
#' @keywords distribution
#' @export
initialize_process_structure <- function(module, par) {
  if (!is.element(par, c("log_devs", "log_r"))) {
    return()
  }

  process_module <- create_recruitment(par)

  set_recruitment_process(module, get_module_id(process_module))

  return(process_module)
}

#' Distributions not available in the stats package
#'
#' Family objects provide a convenient way to specify the details of the models
#' used by functions such as [stats::glm()]. These functions within this
#' package are not available within the stats package but are designed in a
#' similar manner.
#'
#' @param link A string specifying the model link function. For example,
#'   `"identity"` or `"log"` are appropriate names for the [stats::gaussian()]
#'   distribution. `"log"` and `"logit"` are the defaults for the lognormal and
#'   the multinomial, respectively.
#' @return
#' An object of class `family` (which has a concise print method). This
#' particular family has a truncated length compared to other distributions in
#' [stats::family()].
#' \item{family}{character: the family name.}
#' \item{link}{character: the link name.}
#'
#' @seealso
#' * [stats::family()]
#' * [stats::gaussian()]
#' * [stats::glm()]
#' * [stats::power()]
#' * [stats::make.link()]
#' @keywords distribution
#' @export
#' @examples
#' a_family <- multinomial()
#' a_family[["family"]]
#' a_family[["link"]]
lognormal <- function(link = "log") {
  family_class <- c(
    list(family = "lognormal", link = link),
    stats::make.link(link)
  )
  class(family_class) <- "family"
  return(family_class)
}

#' @rdname lognormal
#' @keywords distribution
#' @export
multinomial <- function(link = "logit") {
  family_class <- c(
    list(family = "multinomial", link = link),
    stats::make.link(link)
  )
  class(family_class) <- "family"
  return(family_class)
}
