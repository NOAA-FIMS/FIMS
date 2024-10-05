## usethis namespace: start
#' @useDynLib FIMS, .registration = TRUE
#' @importFrom Rcpp sourceCpp
#' @importFrom TMB MakeADFun
#' @import stats
#' @import methods
#' @importFrom ggplot2 .data
#' @export CreateTMBModel
#' @export get_fixed
#' @export get_random
#' @export clear
#' @export clear_logs
#' @export Parameter
#' @export BevertonHoltRecruitment
#' @export Fleet
#' @export AgeComp
#' @export Index
#' @export Population
#' @export TMBDnormDistribution
#' @export LogisticMaturity
#' @export LogisticSelectivity
#' @export DoubleLogisticSelectivity
#' @export EWAAgrowth
#' @export TMBDlnormDistribution
#' @export TMBDmultinomDistribution
## usethis namespace: end
NULL

# Resolve note from R CMD check on unused imports
# See https://github.com/r-lib/devtools/blob/main/R/devtools-package.R and
# https://r-pkgs.org/dependencies-in-practice.html#how-to-not-use-a-package-in-imports
ignore_unused_imports <- function() {
  scales::date_format
}
