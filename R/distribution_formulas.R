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
  available_distributions <- c("lognormal", "gaussian")
  elements_of_sd <- c("value", "estimation_type")

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
  if (!inherits(family, "family")) {
    abort_bullets <- c(
      abort_bullets,
      "x" = "The class of {.var family} is incorrect.",
      "i" = "{.var family} should be an object of class {.var family},
             e.g., `family = gaussian()`, instead of {class(family)}."
    )
  } else {
    family_name <- family[["family"]]
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
      length(sd[["estimation_type"]]) > 1 &&
        length(sd[["value"]]) != length(sd[["estimation_type"]])
    ) {
      sd_length <- length(sd[["value"]])
      est_length <- length(sd[["estimation_type"]])
      abort_bullets <- c(
        abort_bullets,
        "x" = "The sizes of {.var value} and {.var estimation_type} within {.var sd}
               must match if more than one value is specified for the latter.",
        "i" = "The length of {.var sd[['value']]} is {.code {sd_length}}.",
        "i" = "The length of {.var sd[['estimation_type']]} is
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

#' Set up a new distribution for a data type
#'
#' Use [methods::new()] to set up a distribution within an existing module with
#' the necessary linkages between the two. For example, a fleet module will need
#' a distributional assumption for parts of the data associated with it, which
#' requires the use of `initialize_data_distribution()`.
#' @param module An identifier to a C++ fleet module that is linked to the data
#'   of interest.
#' @param data_type A string specifying the type of data that the
#'   distribution will be fit to. Allowable types include
#'   `r glue::glue_collapse(sprintf('"%s"', eval(formals(initialize_data_distribution)[["data_type"]])), sep = ", ", last = ", and ")`
#'   and the default is
#'   `r eval(formals(initialize_data_distribution)[["data_type"]])[1]`.
#' @param uncertainty A vector of strings specifying formulas for each data
#'   point. See [FIMSFrame()] for more information on what the formula should
#'   look like.
#' @return
#' A reference class. is returned. Use [methods::show()] to view the various
#' Rcpp class fields, methods, and documentation.
#' @keywords distribution
#' @export
#' @seealso
#' * [initialize_process_distribution()]
#' @examples
#' \dontrun{
#' # Set up a new data distribution
#' n_years <- 30
#' # Create a new fleet module
#' fleet <- methods::new(Fleet)
#' # Create a distribution for the fleet module
#' sd_log <- rep(sqrt(log(0.01^2 + 1)), n_years)
#' fleet_distribution <- initialize_data_distribution(
#'   module = fishing_fleet,
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
  family <- unique(stats::na.omit(uncertainty_split[["family"]]))

  # Set up distribution based on `family` argument`
  new_module <- methods::new(get(gsub(
    "^d(.+)$", "D\\1Distribution",
    family
  )))
  if ("log_sd" %in% names(new_module)) {
    ancillary_parameter <- dplyr::filter(
      FIMS::fims_distributions,
      .data$family == .env$family
    ) |>
      dplyr::pull(.data$other_parameters) |>
      unlist()
    new_module$log_sd[] <- log(
      unlist(uncertainty_split[[ancillary_parameter]], use.names = FALSE)
    )
    new_module$log_sd$set_estimation_types("constant")
  }

  # setup link to observed data
  data_id_name <- glue::glue("GetObserved{snake_to_pascal(data_type)}DataID")
  getter_function <- tryCatch(
    do.call("$", list(module, data_id_name)),
    error = function(e) NULL
  )
  if (is.null(getter_function)) {
    cli::cli_abort(c(
      x = "Could not find observed data name, {.code {data_id_name}} in the
      module",
      i = "Check {.var data_type} and the module class.",
      i = "Options in the module are {grep('Get', names(module), value = TRUE)}"
    ))
  }
  new_module$set_observed_data(getter_function())
  # setup link to expected values
  new_module$set_distribution_links(
    "data",
    module$field(unique(stats::na.omit(uncertainty_split[["link"]])))$get_id()
  )

  return(new_module)
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
#'  `"estimation_type"` and it stores a vector of booleans (default =
#'   "constant") is a string indicating whether or not standard deviation is
#'   estimated as a fixed effect or held constant. If `"value"` is a vector and
#'   `"estimation_type"` is a scalar, the single value specified
#'   `"estimation_type"` value will be repeated to match the length of `value`.
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
#'   sd = list(value = 0.4, estimation_type = "constant")
#' )
#' }
initialize_data_distribution <- function(
  module,
  family = NULL,
  # Create a tibble with value and estimation_type column for sd
  sd = tibble::tibble(
    value = 1,
    estimation_type = "constant"
  ),
  # FIXME: Move this argument to second to match where par is in
  # initialize_process_distribution
  data_type = c("landings", "index", "agecomp", "lengthcomp")
) {
  data_type <- rlang::arg_match(data_type)
  # FIXME: Make the available families a data object
  # Could also make the matrix of distributions available per type as a
  # data frame where the check could use the stored object.

  # validity check on user input
  args <- list(
    family = family,
    sd = sd,
    data_type = data_type,
    module = module
  )
  check_distribution_validity(args)

  # assign name of observed data based on data_type
  obs_id_name <- glue::glue("observed_{data_type}_data_id")

  # Set up distribution based on `family` argument`
  if (family[["family"]] == "lognormal") {
    # create new Rcpp module
    new_module <- methods::new(DlnormDistribution)

    # populate logged standard deviation parameter with log of input
    # Using resize() and then assigning value to each element of log_sd directly
    # is correct, as creating a new VariableVector for log_sd here would
    # trigger an error in integration tests with wrappers.
    new_module$log_sd[] <- log(sd[["value"]])
    new_module$log_sd$set_estimation_types(sd[["estimation_type"]])
  }

  if (family[["family"]] == "gaussian") {
    # create new Rcpp module
    new_module <- methods::new(DnormDistribution)

    # populate logged standard deviation parameter with log of input
    new_module$log_sd[] <- log(sd[["value"]])
    new_module$log_sd$set_estimation_types(sd[["estimation_type"]])
  }

  if (family[["family"]] == "multinomial") {
    # create new Rcpp module
    new_module <- methods::new(DmultinomDistribution)
  }

  # setup link to observed data
  if (data_type == "landings") {
    new_module$set_observed_data(module$GetObservedLandingsDataID())
  }
  if (data_type == "index") {
    new_module$set_observed_data(module$GetObservedIndexDataID())
  }
  if (data_type == "agecomp") {
    new_module$set_observed_data(module$GetObservedAgeCompDataID())
  }
  if (data_type == "lengthcomp") {
    new_module$set_observed_data(module$GetObservedLengthCompDataID())
  }

  # set name of expected values
  expected <- get_expected_name(family, data_type)
  # setup link to expected values
  new_module$set_distribution_links("data", module$field(expected)$get_id())

  return(new_module)
}

#' @rdname initialize_data_distribution
#' @keywords distribution
#' @export
initialize_process_distribution <- function(
  module,
  par,
  family = NULL,
  sd = tibble::tibble(
    value = 1,
    estimation_type = "fixed_effects"
  )
) {
  # validity check on user input
  args <- list(family = family, sd = sd)
  check_distribution_validity(args)

  if (!is.element(par, c("log_devs", "log_r"))) {
    return()
  }
  expected <- switch(paste0(par, "_", class(module)),
    "log_devs_Rcpp_BevertonHoltRecruitment" = NULL,
    "log_r_Rcpp_BevertonHoltRecruitment" = "log_expected_recruitment"
  )

  # Set up distribution based on `family` argument`
  if (family[["family"]] == "lognormal") {
    # create new Rcpp module
    new_module <- methods::new(DlnormDistribution)

    # populate logged standard deviation parameter with log of input
    new_module$log_sd[] <- log(sd[["value"]])

    # setup whether or not sd parameter is estimated
    et <- sd[["estimation_type"]]
    et[is.na(et)] <- "constant"
    new_module$log_sd$set_estimation_types(et)
  }

  if (family[["family"]] == "gaussian") {
    # create new Rcpp module
    new_module <- methods::new(DnormDistribution)

    # populate logged standard deviation parameter with log of input
    new_module$log_sd$resize(length(sd[["value"]]))
    for (i in seq_along(sd[["value"]])) {
      new_module$log_sd[i]$value <- log(sd[["value"]][i])
    }

    # setup whether or not sd parameter is estimated
    et <- sd[["estimation_type"]]
    et[is.na(et)] <- "constant"
    new_module$log_sd$set_estimation_types(et)

    #   if (length(sd[["value"]]) > 1 && length(sd[["estimation_type"]]) == 1) {
    #     if (sd[["estimation_type"]] == "constant") {
    #       new_module$log_sd$set_estimation_types(c("constant"))
    #     } else {
    #       new_module$log_sd$set_estimation_types(c("fixed_effects"))
    #     }
    #   } else {
    #     for (i in seq_along(sd[["estimation_type"]])) {
    #       new_module$log_sd[i]$estimation_type$set(sd[["estimation_type"]][i])
    #     }
    #   }
    # }
  }

  n_dim <- length(module$field(par))

  # create new Rcpp modules
  new_module$observed_values$resize(n_dim)
  new_module$expected_values$resize(n_dim)

  # initialize values with 0
  # these are overwritten in the code later by user input
  for (i in 1:n_dim) {
    new_module$observed_values[i]$value <- 0
    new_module$expected_values[i]$value <- 0
  }

  # setup links to parameter
  if (is.null(expected)) {
    new_module$set_distribution_links(
      "random_effects",
      module$field(par)$get_id()
    )
  } else {
    new_module$set_distribution_links(
      "random_effects",
      c(module$field(par)$get_id(), module$field(expected)$get_id())
    )
  }

  return(new_module)
}

#' @rdname initialize_process_distribution
#' @keywords distribution
#' @export
initialize_process_structure <- function(module, par) {
  if (!is.element(par, c("log_devs", "log_r"))) {
    return()
  }
  new_process_module <- switch(paste0(par, "_", class(module)),
    "log_devs_Rcpp_BevertonHoltRecruitment" = new(LogDevsRecruitmentProcess),
    "log_r_Rcpp_BevertonHoltRecruitment" = new(LogRRecruitmentProcess)
  )

  module$SetRecruitmentProcessID(new_process_module$get_id())

  return(new_process_module)
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
