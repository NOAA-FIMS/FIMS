#' Distribution parameter lookup table
#'
#' A lookup table that maps each supported distribution family to the field
#' name used to link observed data, the random-value generator, and the
#' remaining parameter names for that family.
#'
#' @format A data frame with columns `family`, `sample_function`, `central_parameter`, and
#'   `other_parameters`. `sample_function` is a character column with function
#'   names.
#' @keywords datasets
"fims_distributions"
