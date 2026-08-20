#' Clear native FIMS model state
#'
#' Clears registered native model objects, parameters, data, distributions,
#' and backend ID counters. Call this before constructing an independent model.
#'
#' @return `NULL`, invisibly.
#' @export
native_clear <- function() {
  invisible(.Call("fims_call_information_clear", PACKAGE = "FIMS"))
}

#' Assemble the registered native FIMS model
#'
#' @return `TRUE` when model assembly succeeds.
#' @export
native_create_model <- function() {
  .Call("fims_call_create_model", PACKAGE = "FIMS")
}

#' Build the native default likelihood graph
#'
#' @param fishing_fleet_id Integer handle for the fishing fleet.
#' @param survey_fleet_id Integer handle for the survey fleet.
#' @param landings Numeric landings observations.
#' @param landings_cv Numeric landings coefficient of variation.
#' @param landings_age_comp Numeric flattened landings age compositions.
#' @param landings_length_comp Numeric flattened landings length compositions.
#' @param survey_index Numeric survey-index observations.
#' @param survey_cv Numeric survey-index coefficient of variation.
#' @param survey_age_comp Numeric flattened survey age compositions.
#' @param survey_length_comp Numeric flattened survey length compositions.
#' @param recruitment_log_sd Numeric recruitment standard deviation on the
#'   scale expected by the native builder.
#' @param recruitment_log_sd_estimation_type Estimation type for recruitment
#'   `log_sd`.
#' @param n_years,n_ages,n_lengths Integer model dimensions.
#' @return `TRUE` when likelihood construction succeeds.
#' @export
native_build_default_likelihood <- function(
  fishing_fleet_id,
  survey_fleet_id,
  landings,
  landings_cv,
  landings_age_comp,
  landings_length_comp,
  survey_index,
  survey_cv,
  survey_age_comp,
  survey_length_comp,
  recruitment_log_sd,
  recruitment_log_sd_estimation_type = "constant",
  n_years,
  n_ages,
  n_lengths
) {
  .Call(
    "fims_call_build_default_likelihood",
    as.integer(fishing_fleet_id),
    as.integer(survey_fleet_id),
    as.numeric(landings),
    as.numeric(landings_cv),
    as.numeric(landings_age_comp),
    as.numeric(landings_length_comp),
    as.numeric(survey_index),
    as.numeric(survey_cv),
    as.numeric(survey_age_comp),
    as.numeric(survey_length_comp),
    as.numeric(recruitment_log_sd),
    as.integer(.map_estimation_type_code(recruitment_log_sd_estimation_type)),
    as.integer(n_years),
    as.integer(n_ages),
    as.integer(n_lengths),
    PACKAGE = "FIMS"
  )
}

#' Add a prior to a native FIMS parameter
#'
#' Adds a normal or lognormal prior to an entire native parameter vector. A
#' scalar `mean` or `log_sd` is recycled across the vector; alternatively each
#' may have the same length as the target parameter. Prior hyperparameters can
#' be constants or fixed effects.
#'
#' @param module Native module name: `"Fleet"`, `"Population"`,
#'   `"Recruitment"`, `"Maturity"`, or `"Selectivity"`.
#' @param object_id Integer handle returned when the native module was created.
#' @param parameter Name of the parameter vector within the module.
#' @param distribution Either `"normal"` (or `"Dnorm"`) or `"lognormal"`
#'   (or `"Dlnorm"`).
#' @param mean Numeric prior mean. For a lognormal prior this is the mean on the
#'   log scale.
#' @param log_sd Numeric natural logarithm of the prior standard deviation.
#' @param mean_estimation_type,log_sd_estimation_type Whether each prior
#'   hyperparameter is `"constant"` or `"fixed_effects"`.
#' @return `TRUE` invisibly when the prior is registered.
#' @export
native_add_prior <- function(
  module,
  object_id,
  parameter,
  distribution = c("normal", "lognormal", "Dnorm", "Dlnorm"),
  mean = 0,
  log_sd = 0,
  mean_estimation_type = "constant",
  log_sd_estimation_type = "constant"
) {
  distribution <- match.arg(distribution)
  distribution <- switch(distribution,
    Dnorm = "normal",
    Dlnorm = "lognormal",
    distribution
  )
  if (!length(mean) || !all(is.finite(mean))) {
    stop("`mean` must contain finite numeric values.", call. = FALSE)
  }
  if (!length(log_sd) || !all(is.finite(log_sd))) {
    stop("`log_sd` must contain finite numeric values.", call. = FALSE)
  }
  result <- .Call(
    "fims_call_add_prior",
    as.character(module),
    as.integer(object_id),
    as.character(parameter),
    distribution,
    as.numeric(mean),
    as.numeric(log_sd),
    as.integer(.map_estimation_type_code(mean_estimation_type)),
    as.integer(.map_estimation_type_code(log_sd_estimation_type)),
    PACKAGE = "FIMS"
  )
  invisible(result)
}

#' @rdname native_add_prior
#' @export
add_prior <- native_add_prior

#' Read registered native FIMS parameters
#'
#' @return A numeric vector in native registration order.
#' @name native_parameters
NULL

#' @rdname native_parameters
#' @export
native_get_fixed <- function() {
  .Call("fims_call_information_get_fixed", PACKAGE = "FIMS")
}

#' @rdname native_parameters
#' @export
native_get_random <- function() {
  .Call("fims_call_information_get_random", PACKAGE = "FIMS")
}

#' Read registered native FIMS parameter names
#'
#' @return A character vector in native registration order.
#' @export
native_get_parameter_names <- function() {
  .Call("fims_call_information_get_parameter_names", PACKAGE = "FIMS")
}

#' @rdname native_clear
#' @export
clear <- native_clear

#' @rdname native_create_model
#' @export
CreateTMBModel <- native_create_model

#' @rdname native_parameters
#' @export
get_fixed <- native_get_fixed

#' @rdname native_parameters
#' @export
get_random <- native_get_random

#' @rdname native_get_parameter_names
#' @param pars Optional numeric parameter vector to name.
#' @export
get_parameter_names <- function(pars = NULL) {
  parameter_names <- native_get_parameter_names()
  if (is.null(pars)) {
    return(parameter_names)
  }

  if (length(parameter_names) != length(pars)) {
    return(pars)
  }
  names(pars) <- parameter_names
  pars
}

#' @rdname native_get_parameter_names
#' @export
get_random_names <- function(pars = NULL) {
  if (is.null(pars)) {
    return(character())
  }

  names(pars) <- paste0("random_effect_", seq_along(pars))
  pars
}
