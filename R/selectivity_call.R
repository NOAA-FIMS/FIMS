#' Create a logistic selectivity backend object through the native `.Call` interface
#'
#' This creates matching backend logistic selectivity objects in the templated
#' native registries and returns a shared handle for later evaluation.
#'
#' @param inflection_point Numeric scalar or vector matching `x`.
#' @param slope Positive numeric scalar or vector matching `x`. Values are
#' stored internally as log-slopes before evaluation.
#' @param inflection_point_estimation_type Estimation type code(s) for `inflection_point`.
#' Accepts `"constant"`, `"fixed_effects"`, `"random_effects"`, or integer codes 0, 1, 2.
#' @param slope_estimation_type Estimation type code(s) for `slope`.
#' Accepts `"constant"`, `"fixed_effects"`, `"random_effects"`, or integer codes 0, 1, 2.
#' @return An integer handle for the created backend object.
#' @export
selectivity_logistic_create <- function(inflection_point,
                                        slope,
                                        inflection_point_estimation_type = "constant",
                                        slope_estimation_type = "constant") {
  .Call(
    "fims_call_create_logistic_selectivity",
    as.numeric(inflection_point),
    as.numeric(slope),
    as.integer(.map_estimation_type_code(inflection_point_estimation_type)),
    as.integer(.map_estimation_type_code(slope_estimation_type)),
    PACKAGE = "FIMS"
  )
}

#' Evaluate logistic selectivity through the native `.Call` interface
#'
#' @param x Numeric vector of input values.
#' @param inflection_point Numeric scalar or vector used when creating the backend object.
#' @param slope Positive numeric scalar or vector used when creating the backend object.
#' Values are stored internally as log-slopes before evaluation.
#' @return A numeric vector of logistic selectivity values.
#' @export
selectivity_logistic <- function(x, inflection_point, slope) {
  valid_lengths <- c(1L, length(x))
  if (!(length(inflection_point) %in% valid_lengths) ||
    !(length(slope) %in% valid_lengths)) {
    stop(
      "Selectivity parameters must have length 1 or match the length of `x`.",
      call. = FALSE
    )
  }
  selectivity_id <- selectivity_logistic_create(inflection_point, slope)

  .Call(
    "fims_call_logistic_selectivity",
    as.numeric(x),
    selectivity_id,
    PACKAGE = "FIMS"
  )
}

#' Create a double logistic selectivity backend object through the native `.Call` interface
#'
#' This creates matching backend double logistic selectivity objects in the
#' templated native registries and returns a shared handle for later evaluation.
#'
#' @param inflection_point_asc Numeric scalar or vector used when creating the backend object.
#' @param slope_asc Positive numeric scalar or vector used when creating the backend object.
#' Values are stored internally as log-slopes before evaluation.
#' @param inflection_point_desc Numeric scalar or vector used when creating the backend object.
#' @param slope_desc Positive numeric scalar or vector used when creating the backend object.
#' Values are stored internally as log-slopes before evaluation.
#' @param inflection_point_asc_estimation_type Estimation type code(s) for `inflection_point_asc`.
#' Accepts `"constant"`, `"fixed_effects"`, `"random_effects"`, or integer codes 0, 1, 2.
#' @param slope_asc_estimation_type Estimation type code(s) for `slope_asc`.
#' Accepts `"constant"`, `"fixed_effects"`, `"random_effects"`, or integer codes 0, 1, 2.
#' @param inflection_point_desc_estimation_type Estimation type code(s) for `inflection_point_desc`.
#' Accepts `"constant"`, `"fixed_effects"`, `"random_effects"`, or integer codes 0, 1, 2.
#' @param slope_desc_estimation_type Estimation type code(s) for `slope_desc`.
#' Accepts `"constant"`, `"fixed_effects"`, `"random_effects"`, or integer codes 0, 1, 2.
#' @return An integer handle for the created backend object.
#' @export
selectivity_double_logistic_create <- function(inflection_point_asc,
                                               slope_asc,
                                               inflection_point_desc,
                                               slope_desc,
                                               inflection_point_asc_estimation_type = "constant",
                                               slope_asc_estimation_type = "constant",
                                               inflection_point_desc_estimation_type = "constant",
                                               slope_desc_estimation_type = "constant") {
  .Call(
    "fims_call_create_double_logistic_selectivity",
    as.numeric(inflection_point_asc),
    as.numeric(slope_asc),
    as.numeric(inflection_point_desc),
    as.numeric(slope_desc),
    as.integer(.map_estimation_type_code(inflection_point_asc_estimation_type)),
    as.integer(.map_estimation_type_code(slope_asc_estimation_type)),
    as.integer(.map_estimation_type_code(inflection_point_desc_estimation_type)),
    as.integer(.map_estimation_type_code(slope_desc_estimation_type)),
    PACKAGE = "FIMS"
  )
}

#' Evaluate double logistic selectivity through the native `.Call` interface
#'
#' @param x Numeric vector of input values.
#' @param inflection_point_asc Numeric scalar or vector used when creating the backend object.
#' @param slope_asc Positive numeric scalar or vector used when creating the backend object.
#' Values are stored internally as log-slopes before evaluation.
#' @param inflection_point_desc Numeric scalar or vector used when creating the backend object.
#' @param slope_desc Positive numeric scalar or vector used when creating the backend object.
#' Values are stored internally as log-slopes before evaluation.
#' @return A numeric vector of double logistic selectivity values.
#' @export
selectivity_double_logistic <- function(x,
                                        inflection_point_asc,
                                        slope_asc,
                                        inflection_point_desc,
                                        slope_desc) {
  parameter_lengths <- lengths(list(
    inflection_point_asc,
    slope_asc,
    inflection_point_desc,
    slope_desc
  ))
  if (any(!(parameter_lengths %in% c(1L, length(x))))) {
    stop(
      "Selectivity parameters must have length 1 or match the length of `x`.",
      call. = FALSE
    )
  }
  selectivity_id <- selectivity_double_logistic_create(
    inflection_point_asc,
    slope_asc,
    inflection_point_desc,
    slope_desc
  )

  .Call(
    "fims_call_double_logistic_selectivity",
    as.numeric(x),
    selectivity_id,
    PACKAGE = "FIMS"
  )
}

.map_estimation_type_code <- function(x) {
  if (is.character(x)) {
    lookup <- c(
      constant = 0L,
      fixed_effects = 1L,
      random_effects = 2L
    )
    if (!all(x %in% names(lookup))) {
      stop("Estimation type values must be one of: constant, fixed_effects, random_effects.")
    }
    return(unname(lookup[x]))
  }

  if (is.numeric(x) || is.integer(x)) {
    if (any(!is.finite(x)) || any(x %% 1 != 0) ||
      !all(x %in% c(0L, 1L, 2L))) {
      stop("Numeric estimation type codes must be one of: 0 (constant), 1 (fixed_effects), 2 (random_effects).")
    }
    return(as.integer(x))
  }

  stop("Estimation type must be character or integer-like.")
}

#' Get Information parameter and random-effect registration counts
#'
#' Returns the current sizes of fixed/random parameter vectors and names
#' registered in `Information<double>`.
#'
#' @return Named integer vector with registration counts.
#' @export
native_information_parameter_counts <- function() {
  .Call("fims_call_information_parameter_counts", PACKAGE = "FIMS")
}

#' Get Information model assembly counts
#'
#' Returns current counts of model-graph containers that determine whether
#' likelihood components are connected.
#'
#' @return Named integer vector with model assembly counts.
#' @export
native_information_model_counts <- function() {
  .Call("fims_call_information_model_counts", PACKAGE = "FIMS")
}
