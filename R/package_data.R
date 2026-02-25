#' FIMS input vector
#'
#' A character vector containing the allowed data type names used to classify
#' records in the FIMS input data. These values can be used when constructing or
#' validating the `type` column in a FIMS input.
#' @format
#' A vector of `r length(fims_data_types)` strings:
#' \describe{
#'     \item{landings}{The data type used for observed landings.}
#'     \item{index}{The data type used for relative abundance index observations,
#'        such as survey indices or CPUE.}
#'     \item{age_comp}{The data type used for age composition observations.}
#'     \item{length_comp}{The data type used for length composition
#'        observations.}
#'     \item{weight_at_age}{The data type used for weight_at_age observations.}
#' }
"fims_data_types"
