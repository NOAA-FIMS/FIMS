#' {FIMS} input data frame for milestone 1
#'
#' A dataset containing information necessary to run an age-structured stock
#' assessment model in {FIMS} for milestone 1. This data was generated using
#' the `ASSAMC` package written for the
#' [model comparison project](www.github.com/Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison).
#'
#' @format A data frame with 1140 observations of  8 variables:
#' \describe{
#'   \item{type}{The type of data the row contains.}
#'   \item{name}{A character string providing the name of the information source
#'     that the data was collected from, e.g., `"Trawl fishery"`}
#'   \item{age}{An integer age. Entry can be `NA` if information pertains to
#'     multiple ages, e.g., total catch rather than catch of age-4 fish.}
#'   \item{datestart,dateend}{Start and end dates of the data collection period.
#'     Format all dates using `yyyy-mm-dd`, which can accommodate fake years
#'     such as `0001-01-01`.}
#'   \item{value}{The measurement of interest.}
#'   \item{unit}{A character string specifying the units of `value`.}
#'   \item{uncertainty}{A real value providing a measurement of uncertainty
#'     for value.}
#' }
#' @source \url{www.github.com/Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison}
"data_mile1"
