#' Create a fleet backend object through the native `.Call` interface
#'
#' This creates matching backend fleet objects in the templated native
#' registries and returns a shared handle for later evaluation.
#'
#' @param log_fmort Numeric scalar or vector of yearly log fishing mortality.
#' @param log_q Numeric scalar or vector of yearly log catchability.
#' @param selectivity_id Optional integer selectivity handle to link to the fleet.
#' @param age_to_length_conversion Optional numeric vector containing the
#'   flattened age-to-length conversion matrix.
#' @param log_fmort_estimation_type Estimation type code(s) for `log_fmort`.
#' @param log_q_estimation_type Estimation type code(s) for `log_q`.
#' @return An integer handle for the created backend object.
#' @export
fleet_create <- function(log_fmort,
                         log_q,
                         selectivity_id = NA_integer_,
                         age_to_length_conversion = numeric(),
                         log_fmort_estimation_type = "constant",
                         log_q_estimation_type = "constant") {
  selectivity_id <- .native_integer_vector(
    selectivity_id, "selectivity_id",
    allow_na = TRUE, scalar = TRUE
  )
  .Call(
    "fims_call_create_fleet",
    as.numeric(log_fmort),
    as.numeric(log_q),
    selectivity_id,
    as.numeric(age_to_length_conversion),
    as.integer(.map_estimation_type_code(log_fmort_estimation_type)),
    as.integer(.map_estimation_type_code(log_q_estimation_type)),
    PACKAGE = "FIMS"
  )
}

#' Prepare fleet transformed values through the native `.Call` interface
#'
#' @param log_fmort Numeric scalar or vector of yearly log fishing mortality.
#' @param log_q Numeric scalar or vector of yearly log catchability.
#' @param selectivity_id Optional integer selectivity handle to link to the fleet.
#' @param age_to_length_conversion Optional numeric vector containing the
#'   flattened age-to-length conversion matrix.
#' @return A named list with transformed vectors `Fmort` and `q`.
#' @export
fleet_prepare <- function(log_fmort,
                          log_q,
                          selectivity_id = NA_integer_,
                          age_to_length_conversion = numeric()) {
  fleet_id <- fleet_create(
    log_fmort = log_fmort,
    log_q = log_q,
    selectivity_id = selectivity_id,
    age_to_length_conversion = age_to_length_conversion
  )

  result <- .Call(
    "fims_call_fleet_prepare",
    fleet_id,
    PACKAGE = "FIMS"
  )

  # The backend stores scalar catchability once. Expand it to the number of
  # modeled years at the R boundary rather than reading beyond native storage.
  if (length(log_q) == 1L && length(log_fmort) > 1L) {
    result[["q"]] <- rep(exp(log_q), length(log_fmort))
  }
  result
}
