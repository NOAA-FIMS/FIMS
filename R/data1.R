#' FIMS input data frame
#'
#' A dataset containing information necessary to run an age-structured stock
#' assessment model in FIMS. This data was generated using
#' the `ASSAMC` package written for the [model comparison project](
#' www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison).
#'
#' @format
#' A data frame with `r NROW(data1)` observations of `r NCOL(data1)`
#' variables:
#' \describe{ \item{type}{The type of data the row contains. Allowed types
#'   include `age`, `length`, `index`, `landings`, `age-to-length-conversion`,
#'     and `weight-at-age` data.}
#'   \item{name}{A character string providing the name of the information source
#'     that the data was collected from, e.g., `"Trawl fishery"`.}
#'   \item{age}{An integer age. Entry can be `NA` if information pertains to
#'     multiple ages, e.g., total catch rather than catch of age-4 fish.}
#'   \item{length}{A numeric length. Entry can be `NA` if information doesn't
#'     pertain to length.}
#'   \item{datestart,dateend}{Start and end dates of the data collection period.
#'     Format all dates using `yyyy-mm-dd`, which can accommodate fake years
#'     such as `0001-01-01`.}
#'   \item{value}{The measurement of interest.}
#'   \item{unit}{A character string specifying the units of `value`. Allowed
#'     units for each data type are as follows. `mt` is used for `index`,
#'     `landings`, and `weight-at-age` data. `number` or `proportion` are each
#'     viable units for the composition data, where the former is the preferred
#'     unit of measurement.}
#'   \item{uncertainty}{A real value providing a measurement of uncertainty
#'     for value. For catches and survey indices of abundance this should be
#'     the standard deviation of the logged observations if you are using the
#'     lognormal distribution to fit your data. For composition data it will
#'     be your input sample size.
#' }
#' }
#' @source \url{www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison}
"data1"
