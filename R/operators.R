# setClass("Rcpp_Parameter", representation(value_m = "numeric",
#                                         min_m = "numeric",
#                                         max_m = "numeric",
#                                         id_m = "integer",
#                                         is_random_effect_m = "logical",
#                                         estimated_m = "logical"))
# Rcpp::setRcppClass(Class = "Parameter",
#                 module = "fims",
#                 fields = list(value_m = "numeric",
#                               min_m = "numeric",
#                               max_m = "numeric",
#                               id_m = "integer",
#                               is_random_effect_m = "logical",
#                               estimated_m = "logical"))

#' Sets methods for operators under the S4 Generic Group, Ops, which includes 
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and 
#' Logic ("&", "|")
#' 
#' @name Ops - Rcpp_Parameter class inputs
#' @param e1 Rcpp_Parameter class
#' @param e2 Rcpp_Parameter class
#' @aliases Ops-Rcpp_Parameter Ops,Rcpp_Parameter-method
#' @docType methods
#' @export
setMethod("Ops", signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"),
    function(e1, e2){
        if(e1$size() != e2$size()){
            stop("Call to operator Ops, vectors not equal length")
        }
        ret = new(Parameter)
        ret$value = methods::callGeneric(e1$value, e2$value)
    }
)

#' Sets methods for operators under the S4 Generic Group, Ops, which includes 
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and 
#' Logic ("&", "|")
#' 
#' @name Ops - Rcpp_Parameter numeric class inputs
#' @param e1 Rcpp_Parameter class
#' @param e2 numeric value
#' @aliases Ops-Rcpp_Parameter-numeric Ops,Rcpp_Parameter-numeric-method
#' @docType methods
#' @export
setMethod("Ops", signature(e1 = "Rcpp_Parameter", e2 = "numeric"),
    function(e1, e2){
        if(e1$size() != length(e2)){
            stop("Call to operator Ops, vectors not equal length")
        }
        ret = new(Parameter)
        ret$value = methods::callGeneric(e1$value, e2$valu)
    }
)

#' Sets methods for operators under the S4 Generic Group, Ops, which includes 
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and 
#' Logic ("&", "|")
#' 
#' @name Ops - numeric Rcpp_Parameter class inputs
#' @param e1 numeric value
#' @param e2 Rcpp_Parameter class
#' @aliases Ops-numeric-Rcpp_Parameter Ops,numeric-Rcpp_Parameter-method
#' @docType methods
#' @export
setMethod("Ops", signature(e1 = "numeric", e2 = "Rcpp_Parameter"),
    function(e1, e2){
        if(length(e1) != e2$size()){
            stop("Call to operator Ops, vectors not equal length")
        }
        ret = new(Parameter)
        ret$value = methods::callGeneric(e1, e2$value)
    }
)

# setClass("Rcpp_ParameterVector", representation(id_g = "integer",
#                                                 storage_m = "list",
#                                                 id_m = "integer"))

# #' Define Rcpp_Parameter class
# #' 
# #' @name ParameterVector
# #' 
# #' @param id_g global unique id
# #' @param storage_m list of Parameter class values
# #' @param id_m local unique id
# Rcpp::setRcppClass(Class = "ParameterVector",
#             module = "fims",
#             fields = list(id_g = "integer",
#                           storage_m = "list",
#                           id_m = "integer"))

#' Sets methods for operators under the S4 Generic Group, Ops, which includes 
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and 
#' Logic ("&", "|")
#' 
#' @name Ops - Rcpp_ParameterVector class inputs
#' @param e1 Rcpp_ParameterVector class
#' @param e2 Rcpp_ParameterVector class
#' @aliases Ops-Rcpp_ParameterVector Ops,Rcpp_ParameterVector-method
#' @docType methods
#' @export
setMethod("Ops", signature(e1 = "Rcpp_ParameterVector", e2 = "Rcpp_ParameterVector"), 
    function(e1, e2) {
        if(e1$size() != e2$size()){
            stop("Call to operator Ops, vectors not equal length")
        }
        ret <-new(ParameterVector, e1$size())
        for(i in 1:e1$size()){
            ret[i]$value = methods::callGeneric(e1[i]$value, e2[i]$value)
        }
        return(ret)
    })

#' Sets methods for operators under the S4 Generic Group, Ops, which includes 
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and 
#' Logic ("&", "|")
#' 
#' @name Ops - Rcpp_ParameterVector numeric class inputs
#' @param e1 Rcpp_ParameterVector class
#' @param e2 numeric vector or value
#' @aliases Ops-Rcpp_ParameterVector-numeric Ops,Rcpp_ParameterVector-numeric-method
#' @docType methods
#' @export
setMethod("Ops", signature(e1 = "Rcpp_ParameterVector", e2 = "numeric"),
    function(e1, e2) {
        if(e1$size() != length(e2)){
            if(length(e2) == 1){
                ret<-new(ParameterVector, e1$size())
                for(i in 1:e1$size()){
                    ret[i]$value <- methods::callGeneric(e1[i]$value, e2)
                }
                return(ret)
            }
            stop("Call to Ops, vectors not equal length")
        }
        ret<-new(ParameterVector, e1$size())
        for(i in 1:e1$size()){
            ret[i]$value <- methods::callGeneric(e1[i]$value, e2[i])
        }
        return(ret)
    })

#' Sets methods for operators under the S4 Generic Group, Ops, which includes 
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and 
#' Logic ("&", "|")
#' 
#' @name Ops - numeric Rcpp_ParameterVector class inputs
#' @param e1 numeric vector or value
#' @param e2 Rcpp_ParameterVector class
#' @aliases Ops-numeric-Rcpp_ParameterVector Ops,numeric-Rcpp_ParameterVector-method
#' @docType methods
#' @export
setMethod("Ops", signature(e1 = "numeric", e2 = "Rcpp_ParameterVector"),
    function(e1, e2) {
        if(length(e1) != e2$size()){
            if(length(e1) == 1){
                ret<-new(ParameterVector, e2$size())
                for(i in 1:e2$size()){
                    ret[i]$value <- methods::callGeneric(e1, e2[i]$value)
                }
                return(ret)
            }
            stop("Call to operator \"*\", vectors not equal length")
        }
        ret<-new(ParameterVector, e2$size())
        for(i in 1:e2$size()){
            ret[i]$value <- methods::callGeneric(e1[i], e2[i]$value)
        }
        return(ret)
    })    

#' Sets methods for math functions including trigonometry functions, "abs", "sign", 
#' "sqrt", "ceiling", "floor", "trunc", "cummax", "cumprod", "cumsum", "log", "log10",
#' "log2", "log1p", "exp", "expm1", "gamma", "lgamma", "digamma", "trigamma"
#' 
#' @name Math
#' @param x Rcpp_ParameterVector class
#' @aliases Math-Rcpp_ParameterVector Math,Rcpp_ParameterVector-Math-method
#' @docType methods
setMethod("Math", signature(x = "Rcpp_ParameterVector"),
    function(x) {
            xx <- new(ParameterVector, x$size())
            for(i in 1:x$size()){
                xx[i]$value <- methods::callGeneric(x[i]$value)
            }
            return(xx)
        }
    )

#' Set methods for summary functions including "max", "min", "range", "prod", "sum", "any", "all"
#' 
#' @name Summary
#' @param x Rcpp_ParameterVector class
#' @aliases Summary-Rcpp_ParameterVector Summary,Rcpp_ParameterVector-Summary-method
#' @docType methods
setMethod("Summary", signature(x = "Rcpp_ParameterVector"),
    function(x) {
            xx <- new(ParameterVector, x$size())
            for(i in 1:x$size()){
                xx[i]$value <- methods::callGeneric(x[i]$value)
            }
            return(xx)
        }
    )


       
