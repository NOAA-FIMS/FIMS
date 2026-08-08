#' Create an EWAA growth backend object through the native `.Call` interface
#'
#' This creates matching backend EWAA growth objects in the templated native
#' registries and returns a shared handle for later evaluation.
#'
#' @param ages Numeric vector of ages.
#' @param weights Numeric vector of weights at age.
#' @param n_years Integer number of modeled years.
#' @param weights_estimation_type Estimation type code(s) for `weights`.
#' @return An integer handle for the created backend object.
#' @export
growth_ewaa_create <- function(ages,
                               weights,
                               n_years,
                               weights_estimation_type = "constant") {
    .Call(
        "fims_call_create_ewaa_growth",
        as.numeric(ages),
        as.numeric(weights),
        as.integer(n_years),
        as.integer(.map_estimation_type_code(weights_estimation_type)),
        PACKAGE = "FIMS"
    )
}

#' Evaluate EWAA growth through the native `.Call` interface
#'
#' @param year Integer scalar or vector of years.
#' @param age Numeric scalar or vector of ages.
#' @param ages Numeric vector of ages used to create the backend object.
#' @param weights Numeric vector of weights at age used to create the backend object.
#' @param n_years Integer number of modeled years used to create the backend object.
#' @return A numeric vector of evaluated weights-at-age.
#' @export
growth_ewaa <- function(year, age, ages, weights, n_years) {
    growth_id <- growth_ewaa_create(
        ages = ages, weights = weights,
        n_years = n_years
    )

    .Call(
        "fims_call_ewaa_growth_evaluate",
        as.integer(year),
        as.numeric(age),
        growth_id,
        PACKAGE = "FIMS"
    )
}
