#' fit: example function demonstrating TMB specific Roxygen comments
#'
#' @importFrom TMB MakeADFun
#' @useDynLib FIMS
#' @return TMB object
#' @export
#'
#' @examples mkObj()
mkObj <- function() {
  obj <- MakeADFun(data = list(), parameters = list(p = 1), DLL = "FIMS")
  return(obj)
}
