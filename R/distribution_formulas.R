#' Validity checks for distributions
#'
#' Check the validity of arguments passed to [new_data_distribution()] and
#' [new_process_distribution()].
#'
#' @param args A list of input arguments.
#' @noRd
#' @return
#' Nothing is returned if the check is successful, and error messages are
#' returned if the checks are unsuccessful.
check_distribution_validity <- function(args) {
  family <- args[["family"]]
  families <- args[["families"]]
  sd <- args[["sd"]]
  if (class(family) !=  "family") {
    cli::cli_abort(c(
      "x" = "{.code {family}} is the incorrect type.",
      "i" = "{.var family} needs to be specified as a family class, e.g.,
             `family = gaussian()`."
    ))
  }
  if (!(family[["family"]] %in% families)) {
    fam_name <- family[["family"]]
    cli::cli_abort(c(
      "x" = "FIMS currently does not offer the family, {.code {fam_name}}, for
             this distribution type.",
      "i" = "The families available for this distribution type are: {.code
             {families}}."
    ))
  }
  if (!is.null(args[["data_type"]])) {
    data_type <- args[["data_type"]]
    data_type_names <- c("index", "agecomp", "lengthcomp")

    if (!(data_type %in% data_type_names)) {
      cli::cli_abort(c(
        "x" = "The data type is incorrect.",
        "i" = "Correct values are: {.code {data_type_names}}."
      ))
    }

    if (grepl("comp", data_type) &&
        (family[["family"]] == "lognormal" ||
         family[["family"]] == "gaussian")) {
      fam_name <- family[["family"]]
      cli::cli_abort(c(
        "x" = "The family, {.code {fam_name}} is not available for the data
               type, {.code {data_type}}.",
        "i" = "The available families for this data type are: `multinomial`."
      ))
    }
    if (data_type == "index" &&
        family[["family"]] == "multinomial") {
      fam_name <- family[["family"]]
      cli::cli_abort(c(
        "x" = "The family, {.code {fam_name}} is not available for the data
               type, {.code {data_type}}.",
        "i" = "The available families for this data type are: `lognormal` or
               `gaussian`."
      ))
    }
  }

  if (!all(sd[["value"]] > 0)) {
    sd_value <- sd[["value"]]
    cli::cli_abort(c(
      "x" = "At least one value for sd, {.code {sd_value}} is out of bounds.",
      "i" = "All standard deviation values need to be positive."
      ))
  }

  if (length(sd[["estimated"]]) > 1) {
    if (length(sd[["value"]]) != length(sd[["estimated"]])) {
      sd_length <- length(sd[["value"]])
      est_length <- length(sd[["estimated"]])
      cli::cli_abort(c(
        "x" = "The length of `sd$value` is {.code {sd_length}} and the length
               of `sd$estimated` is {.code {est_length}}.",
        "i" = "The size of `value` and `estimated` must match."
      ))
    }
  }
}

#' Return name of expected value
#'
#' @inheritParams new_data_distribution
#' @noRd
#' @return
#' A string specifying the name of the expected value.
#'
get_expected_name <- function(family, data_type) {
  expected_name <- dplyr::case_when(
    data_type == "index" && grepl("lognormal|gaussian", family[["family"]]) &&
      family[["link"]] == "log" ~ "log_expected_index",
    data_type == "index" && grepl("lognormal|gaussian", family[["family"]]) &&
      family[["link"]] == "identity" ~ "expected_index",
    grepl("comp", data_type) ~ "proportion_catch_numbers_at_age"
  )
  # Check combination of entries was okay and led to valid name
  if (is.na(expected_name)) {
    cli::cli_abort(c(
      "x" = "Error, expected_name of distribution is {.var {expected_name}}.",
      "i" = "Some combination of the family and data_type are incompatible."
    ))
  }
  return(expected_name)
}

#' Set up a new distribution for a data type
#'
#' @param module An identifier to a C++ fleet module that is linked to the data
#'   of interest.
#' @param family A description of the error distribution and link function to
#'   be used in the model. The argument takes a family class, e.g.,
#'   `stats::gaussian(link = "identity")`.
#' @param sd A list of length two. The first entry, `"value"`, (default = 1)
#'   stores the initial values (scalar or vector) for the relevant standard
#'   deviations. The second entry, `"estimated"` (default = FALSE) is a scalar
#'   or vector of booleans indicating whether or not standard deviation is
#'   estimated. If `"value"` is a vector and `"estimated"` is a scalar, the
#'   `"estimated"` value will be applied to the entire vector, otherwise, the
#'   dimensions of the two must match.
#' @param data_type A single string specifying the type of data that the
#'   distribution will be fit to. Options are listed in the function call,
#'   where the first option listed, i.e., `"index"` is the default.
#' @return
#' Reference Class. Use [show()] to view Rcpp class fields, methods, and
#' documentation.
#' @export
#' @examples
#' \dontrun{
#' nyears <- 30
#' # Create a new fleet module
#' fleet <- methods::new(Fleet)
#' # Create a distribution for the fleet module
#' fleet_distribution <- new_data_distribution(
#'   module = fishing_fleet,
#'   family = lognormal(link = "log"),
#'   sd = list(value = rep(sqrt(log(0.01^2 + 1)), nyears),
#'   estimated = rep(FALSE, nyears),
#'   data_type = "index"
#' )
#' }
new_data_distribution <- function(
  module,
  family,
  sd = list(value = 1, estimated = FALSE),
  data_type = c("index", "agecomp", "lengthcomp")
) {
  data_type <- match.arg(data_type)
  families <- c("lognormal", "gaussian", "multinomial")

  # validity check on user input
  args <- list(
    family = family,
    sd = sd,
    data_type = data_type,
    families = families
  )
  check_distribution_validity(args)

  # assign name of observed data based on data_type
  obs_id_name <- glue::glue("observed_{data_type}_data_id")

  # Set up distribution based on `family` argument`
  if (family[["family"]] == "lognormal") {
    # create new Rcpp module
    new_module <- new(TMBDlnormDistribution)

    # populate logged standard deviation parameter with log of input
    new_module$log_logsd <- new(
      ParameterVector,
      log(sd$value),
      length(sd$value)
    )
    # setup whether or not sd parameter is estimated
    if (length(sd$value) > 1 && length(sd$estimated) == 1) {
      new_module$log_logsd$set_all_estimable(sd$estimated)
    } else {
      for (i in 1:seq_along(sd$estimated)) {
        new_module$log_logsd[i]$estimated <- sd$estimated[i]
      }
    }
  }

  if (family[["family"]] == "gaussian") {
    # create new Rcpp module
    new_module <- new(TMBDnormDistribution)

    # populate logged standard deviation parameter with log of input
    new_module$log_sd <- new(ParameterVector, log(sd$value), length(sd$value))

    # setup whether or not sd parameter is estimated
    if (length(sd$value) > 1 && length(sd$estimated) == 1) {
      new_module$log_sd$set_all_estimable(sd$estimated)
    } else {
      for (i in 1:seq_along(sd$estimated)) {
        new_module$log_sd[i]$estimated <- sd$estimated[i]
      }
    }
  }

  if (family[["family"]] == "multinomial") {
    #create new Rcpp module
    new_module <- new(TMBDmultinomDistribution)
  }

  # setup link to observed data
  if (data_type == "index") {
    new_module$set_observed_data(module$GetObservedIndexDataID())
  }
  if (data_type == "agecomp") {
    new_module$set_observed_data(module$GetObservedAgeCompDataID())
  }

  # set name of expected values
  expected <- get_expected_name(family, data_type)
  # setup link to expected values
  new_module$set_distribution_links("data", module$field(expected)$get_id())

  return(new_module)
}

#' Sets up a new distribution for a process
#' @inheritParams new_data_distribution
#' @param par A string specifying the parameter name the distribution applies
#'   to. Parameters must be members of the specified module. Use
#'   `methods::show(module)` to obtain names of parameters within the module.
#' @param is_random_effect A boolean indicating whether or not the process is
#'   estimated as a random effect.
#' @seealso
#' * [new_data_distribution()]
#' @export
#' @return
#' Reference Class. Use `show()` to view Rcpp class fields, methods, and
#' documentation.
#' @examples
#' \dontrun{
#' # Create a new fleet module
#' recruitment <- methods::new(BevertonHoltRecruitment)
#' # view parameter names of the recruitment module
#' methods::show(BevertonHoltRecruitment)
#' # Create a distribution for the recruitment module
#' recruitment_distribution <- new_process_distribution(
#'   module = recruitment,
#'   par = "log_devs",
#'   family = gaussian(),
#'   sd = list(value = 0.4, estimated = FALSE),
#'   is_random_effect = FALSE
#' )
#' }
new_process_distribution <- function(module,
                                     par,
                                     family,
                                     sd = list(value = 1, estimated = FALSE),
                                     is_random_effect = FALSE) {
  families <- c("lognormal", "gaussian")

  # validity check on user input
  args <- list(family = family, sd = sd, families = families)
  check_distribution_validity(args)

  # Set up distribution based on `family` argument`
  if (family[["family"]] == "lognormal") {
    # create new Rcpp module
    new_module <- new(TMBDlnormDistribution)

    # populate logged standard deviation parameter with log of input
    new_module$log_logsd <- new(
      ParameterVector,
      log(sd$value),
      length(sd$value)
    )
    #setup whether or not sd parameter is estimated
    if (length(sd$value) > 1 && length(sd$estimated) == 1) {
      new_module$log_logsd$set_all_estimable(sd$estimated)
    } else {
      for (i in 1:seq_along(sd$estimated)) {
        new_module$log_logsd[i]$estimated <- sd$estimated[i]
      }
    }
  }

  if (family[["family"]] == "gaussian") {
    # create new Rcpp module
    new_module <- new(TMBDnormDistribution)

    # populate logged standard deviation parameter with log of input
    new_module$log_sd <- new(ParameterVector, log(sd$value), length(sd$value))

    #setup whether or not sd parameter is estimated
    if (length(sd$value) > 1 && length(sd$estimated) == 1) {
      new_module$log_sd$set_all_estimable(sd$estimated)
    } else {
      for (i in 1:seq_along(sd$estimated)) {
        new_module$log_sd[i]$estimated <- sd$estimated[i]
      }
    }
  }

  # indicate whether or not parameter is treated as a random effect in the model
  module$field(par)$set_all_random(is_random_effect)

  n_dim <- length(module$field(par))

  # create new Rcpp modules
  new_module$x <- new(ParameterVector, n_dim)
  new_module$expected_values <- new(ParameterVector, n_dim)

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

#' Distributions not available in {stats}
#'
#' Family objects provide a convenient way to specify the details of the models
#' used by functions such as [stats::glm()]. These functions within this
#' package are not available within {stats} but are designed in a similar
#' manner.
#'
#' @inheritParams lognormal
#' @return
#' An object of class `family` (which has a concise print method). This
#' particular family has a truncated length compared to other distributions in
#' [stats::family()].
#' \item{family}{character: the family name.}
#' \item{link}{character: the link name.}
#'
#'  @seealso
#' * [stats::family()]
#' * [stats::gaussian()]
#' * [stats::glm()]
#' * [stats::power()]
#' * [stats::make.link()]
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
#' @export
multinomial <- function(link = "logit") {
  family_class <- c(
    list(family = "multinomial", link = link),
    stats::make.link(link)
  )
  class(family_class) <- "family"
  return(family_class)
}
