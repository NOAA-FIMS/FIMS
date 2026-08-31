.native_integer_vector <- function(value,
                                   argument,
                                   minimum = 0L,
                                   allow_na = FALSE,
                                   scalar = FALSE) {
  valid_type <- is.numeric(value)
  valid_length <- !scalar || length(value) == 1L
  missing <- is.na(value)
  valid_missing <- allow_na || !any(missing)
  present <- value[!missing]
  valid_values <- all(is.finite(present)) &&
    all(present %% 1 == 0) &&
    all(present >= minimum) &&
    all(present <= .Machine[["integer.max"]])

  if (!valid_type || !valid_length || !valid_missing || !valid_values) {
    qualifier <- if (scalar) "one whole number" else "whole numbers"
    if (allow_na) {
      qualifier <- paste0(qualifier, " or NA")
    }
    cli::cli_abort(
      "{.arg {argument}} must contain {qualifier} greater than or equal to {minimum}."
    )
  }

  as.integer(value)
}
