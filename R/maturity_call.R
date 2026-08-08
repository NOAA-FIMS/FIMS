#' Create a logistic maturity backend object through the native `.Call` interface
#'
#' This creates matching backend logistic maturity objects in the templated
#' native registries and returns a shared handle for later evaluation.
#'
#' @param inflection_point Numeric scalar or vector used when creating the backend object.
#' @param slope Numeric scalar or vector used when creating the backend object.
#' @param inflection_point_estimation_type Estimation type code(s) for `inflection_point`.
#' @param slope_estimation_type Estimation type code(s) for `slope`.
#' @return An integer handle for the created backend object.
#' @export
maturity_logistic_create <- function(inflection_point,
                                     slope,
                                     inflection_point_estimation_type = "constant",
                                     slope_estimation_type = "constant") {
    .Call(
        "fims_call_create_logistic_maturity",
        as.numeric(inflection_point),
        as.numeric(slope),
        as.integer(.map_estimation_type_code(inflection_point_estimation_type)),
        as.integer(.map_estimation_type_code(slope_estimation_type)),
        PACKAGE = "FIMS"
    )
}

#' Evaluate logistic maturity through the native `.Call` interface
#'
#' @param x Numeric vector of input values.
#' @param inflection_point Numeric scalar or vector used when creating the backend object.
#' @param slope Numeric scalar or vector used when creating the backend object.
#' @return A numeric vector of logistic maturity values.
#' @export
maturity_logistic <- function(x, inflection_point, slope) {
    maturity_id <- maturity_logistic_create(inflection_point, slope)

    .Call(
        "fims_call_logistic_maturity",
        as.numeric(x),
        maturity_id,
        PACKAGE = "FIMS"
    )
}
