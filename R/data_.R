#' FIMS input data frame
#'
#' A data frame containing the information needed to run an age-structured stock
#' assessment model in FIMS. This data was generated using
#' the `ASSAMC` package written for the [model comparison project](
#' www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison).
#' The source code to make this dataset is in `data-raw/data_big.R` which is on
#' GitHub but will not be in your local installation because it is in the
#' .Rbuildignore file.
#'
#' @format
#' A data frame with `r NROW(data_big)` observations of `r NCOL(data_big)`
#' variables:
#' \describe{
#'   \item{type}{The type of input the row contains. Allowed types include
#'   `r glue::glue_collapse(fims_input_types, sep = ", ", last = ", and ")`.}
#'   \item{fleet}{A character string providing the name of the information
#'     source that the input was collected from, e.g., `"Trawl fishery"` or
#'     `"age_zero_survey"`. Entries can be `NA` for information that is not
#'     fleet/vessel specific.}
#'   \item{age}{An integer age. Entry can be `NA` if information pertains to
#'     multiple ages, e.g., total catch rather than catch of age-4 fish.
#'     In this dataset, ages start at one, therefore recruitment happens at age
#'     one not age zero.}
#'   \item{length}{A numeric length. Entry can be `NA` if information doesn't
#'     pertain to length. For `length_comp` rows, this identifies the observed
#'     length bin. For `length_bin` rows, this defines length-bin values used
#'     for age-to-length conversion.}
#'   \item{timing}{The timing, i.e., year the data was collected.}
#'   \item{observed}{The observed measurement of interest.}
#'   \item{unit}{A character string specifying the units of `observed`. Allowed
#'     units for each data type are as follows. `mt` is used for `index`,
#'     `catch`, and `weight_at_age` data. `number` or `proportion` are each
#'     viable units for the composition data, where the former is the preferred
#'     unit of measurement.}
#'   \item{uncertainty}{A right-handed formula specifying the distributional
#'     assumptions for the entry in `observed`. See [FIMSFrame()] for more
#'     information about this column.
#' }
#' }
#' @source \url{www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison}
#' @seealso
#' * [FIMSFrame()]
"data_big"
