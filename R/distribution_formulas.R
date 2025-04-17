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
  # Optional argument data_type
  data_type <- args[["data_type"]]
  check_present <- purrr::map_vec(list("family" = family, "sd" = sd), is.null)

  # Set up global rules
  # FIXME: Move this to a data item in the package so it can be used everywhere
  # Could do a call to all data objects in the package and get unique types that
  # are available
  data_type_names <- c("landings", "index", "agecomp", "lengthcomp")
  if (is.null(data_type)) {
    available_distributions <- c("lognormal", "gaussian")
  } else {
    available_distributions <- switch(
      EXPR = ifelse(grepl("comp", data_type), "composition", data_type),
      "landings" = c("lognormal", "gaussian"),
      "index" = c("lognormal", "gaussian"),
      "composition" = c("multinomial"),
      "unavailable data type"
    )
  }
  elements_of_sd <- c("value", "estimated")

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
  }
  if (
    !(family[["family"]] %in% available_distributions) ||
      "unavailable data type" %in% available_distributions
  ) {
    ifelse_type <- ifelse(
      is.null(data_type),
      "distribution",
      paste(data_type, "data")
    )
    abort_bullets <- c(
      abort_bullets,
      "x" = "FIMS currently does not allow the family to be
             {.code {family[['family']]}}.",
      "i" = "The families available for this {ifelse_type} are
             {.code {available_distributions}}."
    )
  }

  # Checks related to the type of data
  if (!is.null(data_type)) {
    if (!(data_type %in% data_type_names)) {
      abort_bullets <- c(
        abort_bullets,
        "x" = "The specified {.var data_type} of {.var {data_type}} is not
               available.",
        "i" = "Allowed values for {.var data_type} are
               {.code {data_type_names}}."
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
      length(sd[["estimated"]]) > 1 &&
        length(sd[["value"]]) != length(sd[["estimated"]])
    ) {
      sd_length <- length(sd[["value"]])
      est_length <- length(sd[["estimated"]])
      abort_bullets <- c(
        abort_bullets,
        "x" = "The sizes of {.var value} and {.var estimated} within {.var sd}
               must match if more than one value is specified for the latter.",
        "i" = "The length of {.var sd[['value']]} is {.code {sd_length}}.",
        "i" = "The length of {.var sd[['estimated']]} is
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

#' Return name of expected value
#'
#' The combination of data type, family, and link lead to a specific name for
#' the expected value within the code base. This function looks at the
#' combination of these three objects and specifies the appropriate string for
#' its name going forward.
#' @inheritParams initialize_data_distribution
#' @noRd
#' @return
#' A string specifying the name of the expected value.
#'
get_expected_name <- function(family, data_type) {
  # TODO: Think about if the name of the expected value should change based on
  # the link or if it should stay the same? Keeping track of different names in
  # the code base might be too complex for the output as well
  family_string <- family[["family"]]
  link_string <- family[["link"]]
  expected_name <- dplyr::case_when(
    data_type == "landings" &&
      grepl("lognormal|gaussian", family_string) &&
      link_string == "log" ~ "log_landings_expected",
    data_type == "landings" &&
      grepl("lognormal|gaussian", family_string) &&
      link_string == "identity" ~ "landings_expected",
    data_type == "index" &&
      grepl("lognormal|gaussian", family_string) &&
      link_string == "log" ~ "log_index_expected",
    data_type == "index" &&
      grepl("lognormal|gaussian", family_string) &&
      link_string == "identity" ~ "index_expected",
    grepl("agecomp", data_type) ~ "agecomp_proportion",
    grepl("lengthcomp", data_type) ~ "lengthcomp_proportion",
  )
  # Check combination of entries was okay and led to valid name
  if (is.na(expected_name)) {
    cli::cli_abort(c(
      "x" = "The combination of data type, family, and link are incompatible in
             some way.",
      "i" = "{.var data_type} is {.var {data_type}}.",
      "i" = "The family is {.var {family_string}}.",
      "i" = "The link is {.var {link_string}}."
    ))
  }
  return(expected_name)
}

#' Set up a new distribution for a data type or a process
#'
#' Use [methods::new()] to set up a distribution within an existing module with
#' the necessary linkages between the two. For example, a fleet module will need
#' a distributional assumption for parts of the data associated with it, which
#' requires the use of `initialize_data_distribution()`, and a recruitment
#' module, like the Beverton--Holt stock--recruit relationship, will need a
#' distribution associated with the recruitment deviations, which requires
#' `initialize_process_distribution()`.
#' @param module An identifier to a C++ fleet module that is linked to the data
#'   of interest.
#' @param family A description of the error distribution and link function to
#'   be used in the model. The argument takes a family class, e.g.,
#'   `stats::gaussian(link = "identity")`.
#' @param sd A list of length two. The first entry is named `"value"` and it
#'   stores the initial values (scalar or vector) for the relevant standard
#'   deviations. The default is `value = 1`. The second entry is named
#'  `"estimated"` and it stores a vector of booleans (default = FALSE) is a
#'   scalar indicating whether or not standard deviation is estimated. If
#'   `"value"` is a vector and `"estimated"` is a scalar, the single value
#'   specified `"estimated"` value will be repeated to match the length of
#'   `value`. Otherwise, the dimensions of the two must match.
#' @param data_type A string specifying the type of data that the
#'   distribution will be fit to. Allowable types include
#'   `r toString(formals(initialize_data_distribution)[["data_type"]])`
#'   and the default is
#'   `r toString(formals(initialize_data_distribution)[["data_type"]][1])`.
#' @param par A string specifying the parameter name the distribution applies
#'   to. Parameters must be members of the specified module. Use
#'   `methods::show(module)` to obtain names of parameters within the module.
#' @param is_random_effect A boolean indicating whether or not the process is
#'   estimated as a random effect.
#' @return
#' A reference class. is returned. Use [methods::show()] to view the various
#' Rcpp class fields, methods, and documentation.
#' @keywords distribution
#' @export
#' @examples
#' \dontrun{
#' # Set up a new data distribution
#' n_years <- 30
#' # Create a new fleet module
#' fleet <- methods::new(Fleet)
#' # Create a distribution for the fleet module
#' fleet_distribution <- initialize_data_distribution(
#'   module = fishing_fleet,
#'   family = lognormal(link = "log"),
#'   sd = list(
#'     value = rep(sqrt(log(0.01^2 + 1)), n_years),
#'     estimated = rep(FALSE, n_years) # Could also be a single FALSE
#'   ),
#'   data_type = "index"
#' )
#'
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
#'   sd = list(value = 0.4, estimated = FALSE),
#'   is_random_effect = FALSE
#' )
#' }
initialize_data_distribution <- function(
    module,
    family,
    sd = list(value = 1, estimated = FALSE),
    # FIXME: Move this argument to second to match where par is in
    # initialize_process_distribution
    data_type = c("landings", "index", "agecomp", "lengthcomp")) {
  data_type <- rlang::arg_match(data_type)
  # FIXME: Make the available families a data object
  # Could also make the matrix of distributions available per type as a
  # data frame where the check could use the stored object.

  # validity check on user input
  args <- list(
    family = family,
    sd = sd,
    data_type = data_type
  )
  check_distribution_validity(args)

  # assign name of observed data based on data_type
  obs_id_name <- glue::glue("observed_{data_type}_data_id")

  # Set up distribution based on `family` argument`
  if (family[["family"]] == "lognormal") {
    # create new Rcpp module
    new_module <- methods::new(DlnormDistribution)

    # populate logged standard deviation parameter with log of input
    # Using resize() and then assigning value to each element of log_sd diretly
    # is correct, as creating a new ParameterVector for log_sd here would
    # trigger an error in integration tests with wrappers.
    new_module$log_sd$resize(length(sd[["value"]]))
    purrr::walk(
      seq_along(sd[["value"]]),
      \(x) new_module[["log_sd"]][x][["value"]] <- log(sd[["value"]][x])
    )

    # setup whether or not sd parameter is estimated
    if (length(sd[["value"]]) > 1 && length(sd[["estimated"]]) == 1) {
      new_module$log_sd$set_all_estimable(sd[["estimated"]])
    } else {
      for (i in 1:seq_along(sd[["estimated"]])) {
        new_module$log_sd[i]$estimated <- sd[["estimated"]][i]
      }
    }
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
    if (length(sd[["value"]]) > 1 && length(sd[["estimated"]]) == 1) {
      new_module$log_sd$set_all_estimable(sd[["estimated"]])
    } else {
      for (i in 1:seq_along(sd[["estimated"]])) {
        new_module$log_sd[i]$estimated <- sd[["estimated"]][i]
      }
    }
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
    family,
    sd = list(value = 1, estimated = FALSE),
    is_random_effect = FALSE) {
  # validity check on user input
  args <- list(family = family, sd = sd)
  check_distribution_validity(args)

  # Set up distribution based on `family` argument`
  if (family[["family"]] == "lognormal") {
    # create new Rcpp module
    new_module <- methods::new(DlnormDistribution)

    # populate logged standard deviation parameter with log of input
    new_module$log_sd$resize(length(sd[["value"]]))
    purrr::walk(
      seq_along(sd[["value"]]),
      \(x) new_module[["log_sd"]][x][["value"]] <- log(sd[["value"]][x])
    )

    # setup whether or not sd parameter is estimated
    if (length(sd[["value"]]) > 1 && length(sd[["estimated"]]) == 1) {
      new_module$log_sd$set_all_estimable(sd[["estimated"]])
    } else {
      for (i in 1:seq_along(sd[["estimated"]])) {
        new_module$log_sd[i]$estimated <- sd[["estimated"]][i]
      }
    }
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
    if (length(sd[["value"]]) > 1 && length(sd[["estimated"]]) == 1) {
      new_module$log_sd$set_all_estimable(sd[["estimated"]])
    } else {
      for (i in 1:seq_along(sd[["estimated"]])) {
        new_module$log_sd[i]$estimated <- sd[["estimated"]][i]
      }
    }
  }

  # indicate whether or not parameter is treated as a random effect in the model
  module$field(par)$set_all_random(is_random_effect)

  n_dim <- length(module$field(par))

  # create new Rcpp modules
  new_module$x$resize(n_dim)
  new_module$expected_values$resize(n_dim)

  # initialize values with 0
  # these are overwritten in the code later by user input
  for (i in 1:n_dim) {
    new_module$x[i]$value <- 0
    new_module$expected_values[i]$value <- 0
  }

  # setup links to parameter
  new_module$set_distribution_links(
    "random_effects",
    module$field(par)$get_id()
  )

  return(new_module)
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
