Rcpp::loadModule(module = "fims", what = TRUE)

.onUnload <- function(libpath) {
  library.dynam.unload("FIMS", libpath)
}
