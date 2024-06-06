

setMethod("Ops", signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"),
           function(e1, e2) callGeneric(e1, e2))
 
 setMethod("Ops", signature(e1 = "Rcpp_Parameter", e2 = "numeric"),
           function(e1, e2) callGeneric(e1, e2))
 
 setMethod("Ops", signature(e1 = "numeric", e2 = "Rcpp_Parameter"),
           function(e1, e2) callGeneric(e1, e2))

#Parameter
 
#' Computes the trigonometric arc-cosine function
#' @name acos
#' @param x input value to be evaluated
#' @docType methods
  setMethod("acos", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,acos(x$value))})

#' Computes the trigonometric arc-sine function
#' @name asin
#' @param x input value to be evaluated
#' @docType methods
  setMethod("asin", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,asin(x$value))})

#' Computes the trigonometric arc-tangent function
#' @name atan
#' @param x input value to be evaluated
#' @docType methods
  setMethod("atan", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,atan(x$value))})

#' Computes the trigonometric cosine function
#' @name cos
#' @param x input value to be evaluated
#' @docType methods
  setMethod("cos", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,cos(x$value))})

#' Computes the hyperbolic arc-cosine function
#' @name cosh
#' @param x input value to be evaluated
#' @docType methods
  setMethod("cosh", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,cosh(x$value))})

#' Computes the trigonometric sine function
#' @name sin
#' @param x input value to be evaluated
#' @docType methods
  setMethod("sin", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,sin(x$value))})

#' Computes the hyperbolic sine function
#' @name sinh
#' @param x input value to be evaluated
#' @docType methods
  setMethod("sinh", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,sinh(x$value))})

#' Computes the trigonometric tangent function
#' @name tan
#' @param x input value to be evaluated
#' @docType methods
  setMethod("tan", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,tan(x$value))})

#' Computes the hyperbolic tangent function
#' @name tanh
#' @param x input value to be evaluated
#' @docType methods
  setMethod("tanh", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,tanh(x$value))})

#' Computes the exponential function
#' @name exp
#' @param x input value to be evaluated
#' @docType methods
  setMethod("exp", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,exp(x$value))})

#' Computes the base 10 logarithm
#' @name log10
#' @param x input value to be evaluated
#' @docType methods
  setMethod("log10", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,log10(x$value))})

#' Computes the (principle) square root
#' @name sqrt
#' @param x input value to be evaluated
#' @docType methods
  setMethod("sqrt", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter, (x$value^0.5))})

#' Computes logarithms, by default natural logarithms
#' @name log
#' @param x input value to be evaluated
#' @docType methods
  setMethod("log", signature(x = "Rcpp_Parameter"), function (x, base=exp(1)){return(new(Parameter,log(x$value)))})


setMethod("acos", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-acos(x[i]$value)
}
return(xx)
})

setMethod("asin", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-asin(x[i]$value)
}
return(xx)
})

setMethod("atan", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-atan(x[i]$value)
}
return(xx)
})

setMethod("cos", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-cos(x[i]$value)
}
return(xx)
})

setMethod("cosh", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-cosh(x[i]$value)
}
return(xx)
})

setMethod("sin", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-sin(x[i]$value)
}
return(xx)
})

setMethod("sinh", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-sinh(x[i]$value)
}
return(xx)
})

setMethod("tan", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-tan(x[i]$value)
}
return(xx)
})

setMethod("tanh", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-tanh(x[i]$value)
}
return(xx)
})

setMethod("exp", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-exp(x[i]$value)
}
return(xx)
})

setMethod("log10", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-log10(x[i]$value)
}
return(xx)
})

setMethod("sqrt", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-sqrt(x[i]$value)
}
return(xx)
})

setMethod("log", signature(x = "Rcpp_ParameterVector"), function (x) {
 xx<-new(ParameterVector, x$size())
for(i in 1:x$size()){
     xx[i]$value<-log(x[i]$value)
}
return(xx)
})



  setMethod("^", signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"), function (e1, e2){
    (e1$value^e2$value)})
  setMethod("^", signature(e1 = "Rcpp_Parameter", e2 = "numeric"), function (e1, e2){
    ((e1$value^ e2))})
  setMethod("^", signature(e1 = "numeric", e2 = "Rcpp_Parameter"), function (e1, e2){
    (e1^ e2$value)})
  
  #+
  setMethod("+", signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"), function (e1, e2){
    return(new(Parameter,e1$value + e2$value))})
  setMethod("+", signature(e1 = "Rcpp_Parameter", e2 = "numeric"), function (e1, e2){
    return(new(Parameter,e1$value + e2))})
  setMethod("+", signature(e1 = "numeric", e2 = "Rcpp_Parameter"), function (e1, e2){
    return(new(Parameter,e1 + e2$value))})
  #-
  setMethod("-", signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"), function (e1, e2){
    return(new(Parameter,e1$value - e2$value))})
  setMethod("-", signature(e1 = "Rcpp_Parameter", e2 = "numeric"), function (e1, e2){
    return (new(Parameter,e1 - e2$value))})
  setMethod("-", signature(e1 = "numeric", e2 = "Rcpp_Parameter"), function (e1, e2){
    return(new(Parameter,e1 - e2$value))})
  
  #*
  setMethod("*", signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"), function (e1, e2){
    return(new(Parameter,e1$value * e2$value))})
  setMethod("*", signature(e1 = "Rcpp_Parameter", e2 = "numeric"), function (e1, e2){
    return(new(Parameter,e1$value * e2))})
  setMethod("*", signature(e1 = "numeric", e2 = "Rcpp_Parameter"), function (e1, e2){
    return(new(Parameter,e1 * e2$value))})
  
  #/
  setMethod("/", signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"), function (e1, e2){
    return(new(Parameter,e1$value / e2$value))})
  setMethod("/", signature(e1 = "Rcpp_Parameter", e2 = "numeric"), function (e1, e2){
    return(new(Parameter,e1$value / e2))})
  setMethod("/", signature(e1 = "numeric", e2 = "Rcpp_Parameter"), function (e1, e2){
    return(new(Parameter,e1 / e2$value))})


# -------------------------------------------------------------------------

#setMethod("<-", c(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"), function (e1, e2){
 # (e1$value<- e2$value)})

#setMethod("=", c(e1 = "Rcpp_Parameter", e2 = "numeric"), function (e1, e2){
 # (e1$value<- e2)})



setMethod("+", signature(e1 = "Rcpp_ParameterVector", e2 = "Rcpp_ParameterVector"), function (e1, e2){
    
    if(e1$size() != e2$size()){
        stop("Call to operator \"+\", vectors not equal length")
    }
    ret<-new(ParameterVector, e1$size())
    for(i in 1:e1$size()){
        ret[i]$value <-e1[i]$value +e2[i]$value
    }
    return(ret)
     })


setMethod("+", signature(e1 = "Rcpp_ParameterVector", e2 = "numeric"), function (e1, e2){
    
    if(e1$size() != length(e2)){
        
        if(length(e2) == 1){
            ret<-new(ParameterVector, e1$size())
            for(i in 1:e1$size()){
                ret[i]$value <-e1[i]$value +e2
            }
            return(ret)
        }
        stop("Call to operator \"+\", vectors not equal length")
    }
    ret<-new(ParameterVector, e1$size())
    for(i in 1:e1$size()){
        ret[i]$value <-e1[i]$value +e2[i]
    }
    return(ret)
     })

setMethod("+", signature(e1 = "numeric", e2 = "Rcpp_ParameterVector"), function (e1, e2){
    
    if(length(e1) != e2$size()){
        if(length(e1) == 1){
            ret<-new(ParameterVector, e2$size())
            for(i in 1:e2$size()){
                ret[i]$value <-e1+e2[i]$value
            }
            return(ret)
        }
        stop("Call to operator \"+\", vectors not equal length")
    }
    ret<-new(ParameterVector, e2$size())
    for(i in 1:e2$size()){
        ret[i]$value <-e1[i]+e2[i]$value
    }
    return(ret)
     })

setMethod("-", signature(e1 = "Rcpp_ParameterVector", e2 = "Rcpp_ParameterVector"), function (e1, e2){
    
    if(e1$size() != e2$size()){
        stop("Call to operator \"-\", vectors not equal length")
    }
    ret<-new(ParameterVector, e1$size())
    for(i in 1:e1$size()){
        ret[i]$value <-e1[i]$value - e2[i]$value
    }
    return(ret)
     })


setMethod("-", signature(e1 = "Rcpp_ParameterVector", e2 = "numeric"), function (e1, e2){
    
    if(e1$size() != length(e2)){
        
        if(length(e2) == 1){
            ret<-new(ParameterVector, e1$size())
            for(i in 1:e1$size()){
                ret[i]$value <-e1[i]$value - e2
            }
            return(ret)
        }
        stop("Call to operator \"-\", vectors not equal length")
    }
    ret<-new(ParameterVector, e1$size())
    for(i in 1:e1$size()){
        ret[i]$value <-e1[i]$value - e2[i]
    }
    return(ret)
     })

setMethod("-", signature(e1 = "numeric", e2 = "Rcpp_ParameterVector"), function (e1, e2){
    
    if(length(e1) != e2$size()){
        if(length(e1) == 1){
            ret<-new(ParameterVector, e2$size())
            for(i in 1:e2$size()){
                ret[i]$value <-e1-e2[i]$value
            }
            return(ret)
        }
        stop("Call to operator \"-\", vectors not equal length")
    }
    ret<-new(ParameterVector, e2$size())
    for(i in 1:e2$size()){
        ret[i]$value <-e1[i]-e2[i]$value
    }
    return(ret)
     })

setMethod("*", signature(e1 = "Rcpp_ParameterVector", e2 = "Rcpp_ParameterVector"), function (e1, e2){
    
    if(e1$size() != e2$size()){
        stop("Call to operator \"*\", vectors not equal length")
    }
    ret<-new(ParameterVector, e1$size())
    for(i in 1:e1$size()){
        ret[i]$value <-e1[i]$value * e2[i]$value
    }
    return(ret)
     })


setMethod("*", signature(e1 = "Rcpp_ParameterVector", e2 = "numeric"), function (e1, e2){
    
    if(e1$size() != length(e2)){
        
        if(length(e2) == 1){
            ret<-new(ParameterVector, e1$size())
            for(i in 1:e1$size()){
                ret[i]$value <-e1[i]$value * e2
            }
            return(ret)
        }
        stop("Call to operator \"*\", vectors not equal length")
    }
    ret<-new(ParameterVector, e1$size())
    for(i in 1:e1$size()){
        ret[i]$value <-e1[i]$value * e2[i]
    }
    return(ret)
     })

setMethod("*", signature(e1 = "numeric", e2 = "Rcpp_ParameterVector"), function (e1, e2){
    
    if(length(e1) != e2$size()){
        if(length(e1) == 1){
            ret<-new(ParameterVector, e2$size())
            for(i in 1:e2$size()){
                ret[i]$value <-e1*e2[i]$value
            }
            return(ret)
        }
        stop("Call to operator \"*\", vectors not equal length")
    }
    ret<-new(ParameterVector, e2$size())
    for(i in 1:e2$size()){
        ret[i]$value <-e1[i]*e2[i]$value
    }
    return(ret)
     })

setMethod("/", signature(e1 = "Rcpp_ParameterVector", e2 = "Rcpp_ParameterVector"), function (e1, e2){
    
    if(e1$size() != e2$size()){
        stop("Call to operator \"/\", vectors not equal length")
    }
    ret<-new(ParameterVector, e1$size())
    for(i in 1:e1$size()){
        ret[i]$value <-e1[i]$value / e2[i]$value
    }
    return(ret)
     })


setMethod("/", signature(e1 = "Rcpp_ParameterVector", e2 = "numeric"), function (e1, e2){
    
    if(e1$size() != length(e2)){
        
        if(length(e2) == 1){
            ret<-new(ParameterVector, e1$size())
            for(i in 1:e1$size()){
                ret[i]$value <-e1[i]$value / e2
            }
            return(ret)
        }
        stop("Call to operator \"/\", vectors not equal length")
    }
    ret<-new(ParameterVector, e1$size())
    for(i in 1:e1$size()){
        ret[i]$value <-e1[i]$value / e2[i]
    }
    return(ret)
     })

setMethod("/", signature(e1 = "numeric", e2 = "Rcpp_ParameterVector"), function (e1, e2){
    
    if(length(e1) != e2$size()){
        if(length(e1) == 1){
            ret<-new(ParameterVector, e2$size())
            for(i in 1:e2$size()){
                ret[i]$value <-e1/e2[i]$value
            }
            return(ret)
        }
        stop("Call to operator \"/\", vectors not equal length")
    }
    ret<-new(ParameterVector, e2$size())
    for(i in 1:e2$size()){
        ret[i]$value <-e1[i]/e2[i]$value
    }
    return(ret)
     })
