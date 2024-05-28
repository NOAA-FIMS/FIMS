

setMethod("Ops", signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"),
           function(e1, e2) callGeneric(e1, e2))
 
 setMethod("Ops", signature(e1 = "Rcpp_Parameter", e2 = "numeric"),
           function(e1, e2) callGeneric(e1, e2))
 
 setMethod("Ops", signature(e1 = "numeric", e2 = "Rcpp_Parameter"),
           function(e1, e2) callGeneric(e1, e2))

#Parameter
setMethod("acos", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,acos(x$value))})
  setMethod("asin", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,asin(x$value))})
  setMethod("atan", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,atan(x$value))})
  setMethod("cos", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,cos(x$value))})
  setMethod("cosh", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,cosh(x$value))})
  setMethod("sin", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,sin(x$value))})
  setMethod("sinh", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,sinh(x$value))})
  setMethod("tan", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,tan(x$value))})
  setMethod("tanh", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,tanh(x$value))})
  setMethod("exp", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,exp(x$value))})
  setMethod("log10", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter,log10(x$value))})
  setMethod("sqrt", signature(x = "Rcpp_Parameter"), function (x) {new(Parameter, (x$value^0.5))})
  setMethod("log", signature(x = "Rcpp_Parameter"), function (x, base=exp(1)){return(new(Parameter,log(x$value)))})

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
