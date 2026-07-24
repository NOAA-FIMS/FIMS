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
#'     multiple ages, e.g., total landings rather than landings of age-4 fish.
#'     In this dataset, ages start at one, therefore recruitment happens at age
#'     one not age zero.}
#'   \item{length}{A numeric length. Entry can be `NA` if information doesn't
#'     pertain to length.}
#'   \item{timing}{The timing, i.e., year the data was collected.}
#'   \item{value}{The measurement of interest.}
#'   \item{unit}{A character string specifying the units of `value`. Allowed
#'     units for each data type are as follows. `mt` is used for `index`,
#'     `landings`, and `weight_at_age` data. `number` or `proportion` are each
#'     viable units for the composition data, where the former is the preferred
#'     unit of measurement.}
#'   \item{uncertainty}{A real value providing a measurement of uncertainty
#'     for value. For landings and indices of abundance this should be
#'     the standard deviation of the logged observations if you are using the
#'     lognormal distribution to fit your data. For composition data it will
#'     be your input sample size.
#' }
#' }
#' @source \url{www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison}
"data_big"

#' FIMS parameters tibble
#' 
#' A tibble containing the parameters needed to run an age-structured stock
#' assessment model in FIMS. This data was generated using the `ASSAMC` package
#' written for the [model comparison project](www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison).
#' The source code to make this dataset is in `data-raw/parameters_.R` which is
#' on GitHub but will not be in your local installation because it is in the .Rbuildignore file.
#' 
#' @format
#' A tibble with `r NROW(parameters_big)` observations of `r NCOL(parameters_big)` variables:
#' \describe{
#'   \item{\code{model_family}:}{The specified model family (e.g.,
#'     "catch_at_age").}
#'   \item{\code{module_name}:}{The name of the FIMS module (e.g.,
#'     "Data", "Selectivity", "Recruitment", "Growth", "Maturity"). These
#'     entries are always written in PascalCase to match the names used in the
#'     C++ code.}
#'   \item{\code{fleet}:}{The name of the fleet the module applies to. This
#'     will be `NA` for non-fleet-specific modules like "Recruitment".}
#'   \item{\code{data}:}{A list-column containing a `tibble` with detailed
#'     parameters. Unnesting this column reveals:
#'     \describe{
#'       \item{\code{module_type}:}{The specific type of the module (e.g.,
#'         "Logistic" for a "Selectivity" module). This column will always be
#'         written in PascalCase to match the names used in the C++ code.}
#'       \item{\code{label}:}{The name of the parameter (e.g.,
#'         "inflection_point").}
#'       \item{\code{age}:}{The age the parameter applies to.}
#'       \item{\code{length}:}{The length bin the parameter applies to.}
#'       \item{\code{time}:}{The time step (i.e., year) the parameter applies
#'         to.}
#'       \item{\code{value}:}{The initial value of the parameter.}
#'       \item{\code{estimation_type}:}{The type of estimation (e.g.,
#'         "constant", "fixed_effects", "random_effects").}
#'       \item{\code{distribution_type}:}{The type of distribution (e.g.,
#'         "Data", "process"), where a process distribution can refer to a
#'         fixed effect or a random effect but it does not fit to data, e.g.,
#'         recruitment deviations.}
#'       \item{\code{distribution}:}{The name of distribution (e.g.,
#'         "Dlnorm", `Dmultinom`). The column will always be written in
#'         PascalCase to match the names used in the C++ code.}
#'     }
#'   }
#' }
#' @source \url{www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison}
"parameters_big"

#' FIMS fit
#' 
#' An object of class `FIMSFit`, where the structure is the same
#' regardless if `optimize = TRUE` or not. Uncertainty information is only
#' included in the `estimates` slot if `get_sd = TRUE`.
#' 
#' @format 
#' An object of class `FIMSFit` with the following slots:
#' `r paste(slotNames(fit_with_optimization_big), collapse = ", ")`
#' @rdname fit_with_optimization_big
"fit_with_optimization_big"

#' @rdname fit_with_optimization_big
"fit_without_optimization_big"

#' FIMS estimates
#'
#' A tibble containing the estimates from a fitted FIMS model. The structure is
#' the same regardless if `optimize = TRUE` or not. Uncertainty information is
#' only included if `get_sd = TRUE`.
#' @format
#' A tibble with `r NROW(estimates_with_optimization_big)`
#' observations of `r NCOL(estimates_with_optimization_big)` variables:
#' `r paste(colnames(estimates_with_optimization_big), collapse = ", ")`
#' @rdname estimates_with_optimization_big
"estimates_with_optimization_big"

#' @rdname estimates_with_optimization_big
"estimates_without_optimization_big"
