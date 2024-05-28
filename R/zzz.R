Rcpp::loadModule(module = "fims", what = TRUE)

.onUnload <- function(libpath) {
  library.dynam.unload("FIMS", libpath)
}

setMethod("[<-", signature(x = "Rcpp_ParameterVector", i = "numeric"),
          function(x, i) {
            (x$at(i))
             return(x)
          })

setMethod("[", signature(x = "Rcpp_ParameterVector", i = "numeric"),
          function(x, i) {
            return(x$at(i))
          })

# setMethod("lapply", signature(X = "Rcpp_ParameterVector", FUN = "sum"),
#           function(X,FUN) {
#             lapply(X$data,FUN)
#           })

setMethod("length", signature(x = "Rcpp_ParameterVector"),
          function(x) {
            return(x$size())
          })

setMethod("sum", signature(x = "Rcpp_ParameterVector"),
          function(x) {
            ret<-new(Parameter)
            tmp<-0.0
            for(i in 1:x$size()){
                tmp = tmp + x[i]$value
            }
            ret$value<-tmp
            return(ret)
          })

setMethod("dim", signature(x = "Rcpp_ParameterVector"),
          function(x) {
            return(x$size())
          })
