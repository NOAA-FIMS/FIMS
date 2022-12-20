#' fit: example function demonstrating TMB specific Roxygen comments
#'
#' @importFrom TMB MakeADFun
#' @useDynLib FIMS, .registration = TRUE
#' @importFrom Rcpp sourceCpp
#' @return TMB object
#' @export
#'
#' @examples \dontrun{
#' mkObj()
#' }
mkObj <- function() {
  obj <- MakeADFun(data = list(), parameters = list(p = 1), DLL = "FIMS")
  return(obj)
}
