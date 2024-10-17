#' Sets methods for operators under the S4 Generic Group, Ops, which includes
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and
#' Logic ("&", "|")
#' @importFrom methods callGeneric
#' @param e1 Rcpp_Parameter class
#' @param e2 Rcpp_Parameter class
#' @keywords set_methods
#' @export
#' @rdname Ops
setMethod("Ops", signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"),
    function(e1, e2){
        ret = new(Parameter)
        ret$value = callGeneric(e1$value, e2$value)
    }
)

#' Sets methods for operators under the S4 Generic Group, Ops, which includes
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and
#' Logic ("&", "|")
#'
#' @param e1 Rcpp_Parameter class
#' @param e2 numeric value
#' @keywords set_methods
#' @rdname Ops
setMethod("Ops", signature(e1 = "Rcpp_Parameter", e2 = "numeric"),
    function(e1, e2){
        if(length(e2) != 1){
            stop("Call to operator Ops, value not scalar")
        }
        ret = new(Parameter)
        ret$value = methods::callGeneric(e1$value, e2)
    }
)

#' Sets methods for operators under the S4 Generic Group, Ops, which includes
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and
#' Logic ("&", "|")
#'
#' @param e1 numeric value
#' @param e2 Rcpp_Parameter class
#' @keywords set_methods
#' @rdname Ops
setMethod("Ops", signature(e1 = "numeric", e2 = "Rcpp_Parameter"),
    function(e1, e2){
        if(length(e1) != 1){
            stop("Call to operator Ops, value not scalar")
        }
        ret = new(Parameter)
        ret$value = methods::callGeneric(e1, e2$value)
    }
)



#' Sets methods for operators under the S4 Generic Group, Ops, which includes
#' Arith ("+", "-", "*", "^", %%, %/%, "/"),
#' Compare ("==", ">", "<", "!=", "<=", ">="), and
#' Logic ("&", "|")
#'
#' @param e1 Rcpp_ParameterVector class
#' @param e2 Rcpp_ParameterVector class
#' @keywords set_methods
#' @rdname Ops
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
#' @param e1 Rcpp_ParameterVector class
#' @param e2 numeric vector or value
#' @keywords set_methods
#' @rdname Ops
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
#' @param e1 numeric vector or value
#' @param e2 Rcpp_ParameterVector class
#' @keywords set_methods
#' @rdname Ops
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
            stop("Call to operator, vectors not equal length")
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
#' @param x numeric vector
#' @keywords set_methods
#' @export
#' @rdname Math
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
#' @param x numeric vector
#' @export
#' @keywords set_methods
#' @rdname Summary
setMethod("Summary", signature(x = "Rcpp_ParameterVector"),
    function(x) {
            xx <- new(ParameterVector, x$size())
            for(i in 1:x$size()){
                xx[i]$value <- methods::callGeneric(x[i]$value)
            }
            return(xx)
        }
    )



