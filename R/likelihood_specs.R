#' Likelihood specification constructors
#'
#' These constructors describe distributions and model roles without creating
#' or linking Rcpp objects. They are intended for the higher-level R builder
#' interface, where users describe the model and FIMS performs the low-level
#' object wiring internally.
#'
#' @param mean Numeric expected value for a normal distribution.
#' @param sd Positive numeric standard deviation.
#' @param distribution A FIMS distribution specification.
#' @param value Numeric value or vector for a fixed effect or prior target.
#' @param fleet Character name of the fleet associated with an observation.
#' @param data Observed data values. Vectors, matrices, and data frames are
#'   accepted and stored without modification.
#'
#' @return
#' Distribution constructors return objects with class
#' `fims_distribution_spec`. Role constructors return objects with class
#' `fims_role_spec`. Observation constructors return objects with class
#' `fims_observation_spec`.
#'
#' @name likelihood_specs
#' @keywords likelihood
NULL

#' @rdname likelihood_specs
#' @export
normal <- function(mean = 0, sd = 1) {
  check_numeric_spec(mean, "mean")
  check_positive_numeric_spec(sd, "sd")
  new_fims_distribution_spec(
    family = "normal",
    backend = "DnormDistribution",
    mean = mean,
    sd = sd
  )
}

#' @rdname likelihood_specs
#' @export
prior <- function(value, distribution) {
  check_numeric_spec(value, "value")
  check_distribution_spec(distribution)
  new_fims_role_spec(
    role = "prior",
    value = value,
    distribution = distribution
  )
}

#' @rdname likelihood_specs
#' @export
random_effect <- function(distribution) {
  check_distribution_spec(distribution)
  new_fims_role_spec(
    role = "random_effect",
    distribution = distribution
  )
}

#' @rdname likelihood_specs
#' @export
fixed_effect <- function(value) {
  check_numeric_spec(value, "value")
  new_fims_role_spec(
    role = "fixed_effect",
    value = value
  )
}

#' @rdname likelihood_specs
#' @export
observe_landings <- function(fleet, data, distribution = lognormal()) {
  new_fims_observation_spec(
    data_type = "landings",
    fleet = fleet,
    data = data,
    distribution = distribution,
    allowed_families = c("normal", "lognormal")
  )
}

#' @rdname likelihood_specs
#' @export
observe_index <- function(fleet, data, distribution = lognormal()) {
  new_fims_observation_spec(
    data_type = "index",
    fleet = fleet,
    data = data,
    distribution = distribution,
    allowed_families = c("normal", "lognormal")
  )
}

#' @rdname likelihood_specs
#' @export
observe_age_comp <- function(fleet, data, distribution = multinomial()) {
  new_fims_observation_spec(
    data_type = "age_comp",
    fleet = fleet,
    data = data,
    distribution = distribution,
    allowed_families = "multinomial"
  )
}

#' @rdname likelihood_specs
#' @export
observe_length_comp <- function(fleet, data, distribution = multinomial()) {
  new_fims_observation_spec(
    data_type = "length_comp",
    fleet = fleet,
    data = data,
    distribution = distribution,
    allowed_families = "multinomial"
  )
}

new_fims_distribution_spec <- function(family, backend, ...) {
  spec <- list(
    family = family,
    backend = backend,
    args = list(...)
  )
  class(spec) <- c("fims_distribution_spec", "fims_spec")
  spec
}

new_fims_role_spec <- function(role, ...) {
  spec <- c(list(role = role), list(...))
  class(spec) <- c("fims_role_spec", "fims_spec")
  spec
}

new_fims_observation_spec <- function(
    data_type, fleet, data, distribution, allowed_families) {
  check_name_spec(fleet, "fleet")
  check_data_spec(data)
  check_distribution_spec(distribution)
  check_distribution_family(distribution, allowed_families, data_type)

  spec <- list(
    data_type = data_type,
    fleet = fleet,
    data = data,
    distribution = distribution
  )
  class(spec) <- c("fims_observation_spec", "fims_spec")
  spec
}

as_fims_distribution_spec <- function(family, backend, family_class, ...) {
  args <- list(link = family_class[["link"]], ...)
  args <- args[!vapply(args, is.null, logical(1))]
  family_class[["fims_spec"]] <- do.call(
    new_fims_distribution_spec,
    c(list(family = family, backend = backend), args)
  )
  class(family_class) <- unique(c("fims_distribution_spec", class(family_class)))
  family_class
}

check_distribution_spec <- function(distribution) {
  if (!inherits(distribution, "fims_distribution_spec")) {
    stop(
      "`distribution` must be a FIMS distribution specification.",
      call. = FALSE
    )
  }
  invisible(distribution)
}

check_distribution_family <- function(distribution, allowed_families, data_type) {
  family <- distribution[["family"]]
  if (!family %in% allowed_families) {
    stop(
      "`distribution` family `", family, "` is not supported for `",
      data_type, "` observations.",
      call. = FALSE
    )
  }
  invisible(distribution)
}

check_name_spec <- function(value, name) {
  if (!is.character(value) || length(value) != 1L || is.na(value) ||
      value == "") {
    stop("`", name, "` must be a non-empty character scalar.", call. = FALSE)
  }
  invisible(value)
}

check_data_spec <- function(data) {
  if (is.null(data)) {
    stop("`data` must not be NULL.", call. = FALSE)
  }
  invisible(data)
}

check_numeric_spec <- function(value, name) {
  if (!is.numeric(value) || length(value) < 1L || any(is.na(value))) {
    stop(
      "`", name, "` must be a numeric value or vector with no missing values.",
      call. = FALSE
    )
  }
  invisible(value)
}

check_positive_numeric_spec <- function(value, name) {
  check_numeric_spec(value, name)
  if (any(value <= 0)) {
    stop("`", name, "` must be greater than 0.", call. = FALSE)
  }
  invisible(value)
}
