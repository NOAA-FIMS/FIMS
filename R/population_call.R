#' Create a population backend object through the native `.Call` interface
#'
#' This creates matching backend population objects in the templated native
#' registries and optionally links existing maturity, growth, recruitment,
#' and fleet objects by handle.
#'
#' @param log_m Numeric scalar or vector of yearly log natural mortality.
#' @param log_f_multiplier Numeric scalar or vector of yearly log fishing multipliers.
#' @param log_init_naa Numeric vector of log initial numbers at age.
#' @param log_m_estimation_type Estimation type code(s) for `log_m`.
#' @param log_f_multiplier_estimation_type Estimation type code(s) for `log_f_multiplier`.
#' @param log_init_naa_estimation_type Estimation type code(s) for `log_init_naa`.
#' @param maturity_id Optional integer maturity handle.
#' @param growth_id Optional integer growth handle.
#' @param recruitment_id Optional integer recruitment handle.
#' @param fleet_ids Optional integer vector of fleet handles.
#' @return An integer handle for the created backend object.
#' @export
population_create <- function(log_m,
                              log_f_multiplier,
                              log_init_naa,
                              log_m_estimation_type = "constant",
                              log_f_multiplier_estimation_type = "constant",
                              log_init_naa_estimation_type = "constant",
                              maturity_id = NA_integer_,
                              growth_id = NA_integer_,
                              recruitment_id = NA_integer_,
                              fleet_ids = integer()) {
    .Call(
        "fims_call_create_population",
        as.numeric(log_m),
        as.numeric(log_f_multiplier),
        as.numeric(log_init_naa),
        as.integer(.map_estimation_type_code(log_m_estimation_type)),
        as.integer(.map_estimation_type_code(log_f_multiplier_estimation_type)),
        as.integer(.map_estimation_type_code(log_init_naa_estimation_type)),
        as.integer(maturity_id),
        as.integer(growth_id),
        as.integer(recruitment_id),
        as.integer(fleet_ids),
        PACKAGE = "FIMS"
    )
}

#' Prepare population transformed values through the native `.Call` interface
#'
#' @param log_m Numeric scalar or vector of yearly log natural mortality.
#' @param log_f_multiplier Numeric scalar or vector of yearly log fishing multipliers.
#' @param log_init_naa Numeric vector of log initial numbers at age.
#' @param maturity_id Optional integer maturity handle.
#' @param growth_id Optional integer growth handle.
#' @param recruitment_id Optional integer recruitment handle.
#' @param fleet_ids Optional integer vector of fleet handles.
#' @return A named list with transformed vectors `M` and `f_multiplier`.
#' @export
population_prepare <- function(log_m,
                               log_f_multiplier,
                               log_init_naa,
                               maturity_id = NA_integer_,
                               growth_id = NA_integer_,
                               recruitment_id = NA_integer_,
                               fleet_ids = integer()) {
    population_id <- population_create(
        log_m = log_m,
        log_f_multiplier = log_f_multiplier,
        log_init_naa = log_init_naa,
        maturity_id = maturity_id,
        growth_id = growth_id,
        recruitment_id = recruitment_id,
        fleet_ids = fleet_ids
    )

    .Call(
        "fims_call_population_prepare",
        population_id,
        PACKAGE = "FIMS"
    )
}
