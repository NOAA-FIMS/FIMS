#' Create a Beverton-Holt recruitment backend object through the native `.Call` interface
#'
#' This creates matching backend Beverton-Holt recruitment objects in the
#' templated native registries and returns a shared handle for later
#' evaluation.
#'
#' @param logit_steep Numeric scalar or vector used when creating the backend object.
#' @param log_rzero Numeric scalar or vector used when creating the backend object.
#' @param log_devs Optional numeric vector of log recruitment deviations.
#' @param logit_steep_estimation_type Estimation type code(s) for `logit_steep`.
#' @param log_rzero_estimation_type Estimation type code(s) for `log_rzero`.
#' @param log_devs_estimation_type Estimation type code(s) for `log_devs`.
#' @return An integer handle for the created backend object.
#' @export
recruitment_beverton_holt_create <- function(logit_steep,
                                             log_rzero,
                                             log_devs = numeric(),
                                             logit_steep_estimation_type = "constant",
                                             log_rzero_estimation_type = "constant",
                                             log_devs_estimation_type = "constant") {
    .Call(
        "fims_call_create_beverton_holt_recruitment",
        as.numeric(logit_steep),
        as.numeric(log_rzero),
        as.numeric(log_devs),
        as.integer(.map_estimation_type_code(logit_steep_estimation_type)),
        as.integer(.map_estimation_type_code(log_rzero_estimation_type)),
        as.integer(.map_estimation_type_code(log_devs_estimation_type)),
        PACKAGE = "FIMS"
    )
}

#' Evaluate Beverton-Holt recruitment mean through the native `.Call` interface
#'
#' @param spawners Numeric scalar or vector of spawner values.
#' @param phi_0 Numeric scalar or vector of unfished spawners-per-recruit values.
#' @param logit_steep Numeric scalar or vector used when creating the backend object.
#' @param log_rzero Numeric scalar or vector used when creating the backend object.
#' @return A numeric vector of expected recruitment values.
#' @export
recruitment_beverton_holt <- function(spawners,
                                      phi_0,
                                      logit_steep,
                                      log_rzero) {
    recruitment_id <- recruitment_beverton_holt_create(logit_steep, log_rzero)

    .Call(
        "fims_call_beverton_holt_evaluate_mean",
        as.numeric(spawners),
        as.numeric(phi_0),
        recruitment_id,
        PACKAGE = "FIMS"
    )
}
