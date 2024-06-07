Rcpp::setRcppClass(Class = "Parameter",
            module = "fims",
            fields = list(value_m = "numeric",
                          min_m = "numeric",
                          max_m = "numeric",
                          id_m = "integer",
                          is_random_effect_m = "logical",
                          estimated_m = "logical"),
            methods = list(
                Ops = function(e1 = "self", e2 = "self") methods::callGeneric(e1,e2),
                Ops = function(e1 = "self", e2 = "numeric") methods::callGeneric(e1,e2),
                Ops = function(e1 = "numeric", e2 = "self") methods::callGeneric(e1,e2),
                acos = function(x = "self") new(Parameter,acos(x$value)),
                asin = function(x = "self") new(Parameter,asin(x$value)),
                atan = function(x = "self") new(Parameter,atan(x$value)),
                cos = function(x = "self") new(Parameter,cos(x$value)),
                cosh = function(x = "self") new(Parameter,cosh(x$value)),
                sin = function(x = "self") new(Parameter,sin(x$value)),
                sinh = function(x = "self") new(Parameter,sinh(x$value)),
                tan = function(x = "self") new(Parameter,tan(x$value)),
                tanh = function(x = "self") new(Parameter,tanh(x$value)),
                exp = function(x = "self") new(Parameter,exp(x$value)),
                log10 = function(x = "self") new(Parameter,log10(x$value)),
                sqrt = function(x = "self") new(Parameter,sqrt(x$value)),
                log = function(x = "self") new(Parameter,log(x$value)),
                "^" = function(e1 = "self", e2 = "self") e1$value^e2$value,
                "^" = function(e1 = "numeric", e2 = "self") e1$value^e2$value,
                "^" = function(e1 = "self", e2 = "numeric") e1$value^e2$value,
                "+" = function(e1 = "self", e2 = "self") e1$value+e2$value,
                "+" = function(e1 = "numeric", e2 = "self") e1$value+e2$value,
                "+" = function(e1 = "self", e2 = "numeric") e1$value+e2$value,
                "-" = function(e1 = "self", e2 = "self") e1$value-e2$value,
                "-" = function(e1 = "numeric", e2 = "self") e1$value-e2$value,
                "-" = function(e1 = "self", e2 = "numeric") e1$value-e2$value,
                "*" = function(e1 = "self", e2 = "self") e1$value*e2$value,
                "*" = function(e1 = "numeric", e2 = "self") e1$value*e2$value,
                "*" = function(e1 = "self", e2 = "numeric") e1$value*e2$value,
                "/" = function(e1 = "self", e2 = "self") e1$value/e2$value,
                "/" = function(e1 = "numeric", e2 = "self") e1$value/e2$value,
                "/" = function(e1 = "self", e2 = "numeric") e1$value/e2$value
            ),
            saveAs = "Rcpp_Parameter"
                          )

# -------------------------------------------------------------------------


Rcpp::setRcppClass(Class = "ParameterVector",
            module = "fims",
            fields = list(id_g = "integer",
                          storage_m = "list",
                          id_m = "integer"),
            methods = list(
                acos =  function (x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-acos(x[i]$value)
                    }
                    return(xx)
                    },
                asin = function(x = "self") {
                        xx<-new(ParameterVector, x$size())
                        for(i in 1:x$size()){
                            xx[i]$value<-asin(x[i]$value)
                        }
                        return(xx)
                    },
                atan = function(x = "self") {
                        xx<-new(ParameterVector, x$size())
                        for(i in 1:x$size()){
                            xx[i]$value<-atan(x[i]$value)
                        }
                        return(xx)
                        },
                cos = function(x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-cos(x[i]$value)
                    }
                    return(xx)
                    },
                cosh = function(x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-cosh(x[i]$value)
                    }
                    return(xx)
                    },
                sin = function(x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-sin(x[i]$value)
                    }
                    return(xx)
                    },
                sinh = function(x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-sinh(x[i]$value)
                    }
                    return(xx)
                    },
                tan = function(x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-tan(x[i]$value)
                    }
                    return(xx)
                    },
                tanh = function(x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-tanh(x[i]$value)
                    }
                    return(xx)
                    },
                exp = function(x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-exp(x[i]$value)
                    }
                    return(xx)
                    },
                log10 = function(x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-log10(x[i]$value)
                    }
                    return(xx)
                    },
                sqrt = function(x = "self") function (x) {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-sqrt(x[i]$value)
                    }
                    return(xx)
                    },
                log = function(x = "self") {
                    xx<-new(ParameterVector, x$size())
                    for(i in 1:x$size()){
                        xx[i]$value<-log(x[i]$value)
                    }
                    return(xx)
                    },
                "^" = function(e1 = "self", e2 = "self") {
                    if(e1$size() != e2$size()){
                        stop("Call to operator \"^\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e1$size())
                    for(i in 1:e1$size()){
                        ret[i]$value <-e1[i]$value^e2[i]$value
                    }
                    return(ret)
                    },
                "^" = function(e1 = "self", e2 = "numeric") {
                    if(e1$size() != length(e2)){
                        
                        if(length(e2) == 1){
                            ret<-new(ParameterVector, e1$size())
                            for(i in 1:e1$size()){
                                ret[i]$value <-e1[i]$value^e2
                            }
                            return(ret)
                        }
                        stop("Call to operator \"^\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e1$size())
                    for(i in 1:e1$size()){
                        ret[i]$value <-e1[i]$value^e2[i]
                    }
                    return(ret)
                                },
                "^" = function(e1 = "numeric", e2 = "self") {
                    if(length(e1) != e2$size()){
                        if(length(e1) == 1){
                            ret<-new(ParameterVector, e2$size())
                            for(i in 1:e2$size()){
                                ret[i]$value <- e1^e2[i]$value
                            }
                            return(ret)
                        }
                        stop("Call to operator \"^\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e2$size())
                    for(i in 1:e2$size()){
                        ret[i]$value <-e1[i]^e2[i]$value
                    }
                    return(ret)
                    },
                "+" = function(e1 = "self", e2 = "self") {
                    if(e1$size() != e2$size()){
                        stop("Call to operator \"+\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e1$size())
                    for(i in 1:e1$size()){
                        ret[i]$value <-e1[i]$value +e2[i]$value
                    }
                    return(ret)
                    },
                "+" = function(e1 = "self", e2 = "numeric") {
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
                                },
                "+" = function(e1 = "numeric", e2 = "self") {
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
                    },
                "-" = function(e1 = "self", e2 = "self") {
                    if(e1$size() != e2$size()){
                        stop("Call to operator \"-\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e1$size())
                    for(i in 1:e1$size()){
                        ret[i]$value <-e1[i]$value - e2[i]$value
                    }
                    return(ret)
                    },
                "-" = function(e1 = "self", e2 = "numeric") {
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
                                },
                "-" = function(e1 = "numeric", e2 = "self") {
                    if(length(e1) != e2$size()){
                        if(length(e1) == 1){
                            ret<-new(ParameterVector, e2$size())
                            for(i in 1:e2$size()){
                                ret[i]$value <-e1 - e2[i]$value
                            }
                            return(ret)
                        }
                        stop("Call to operator \"-\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e2$size())
                    for(i in 1:e2$size()){
                        ret[i]$value <-e1[i] - e2[i]$value
                    }
                    return(ret)
                    },
                "*" = function(e1 = "self", e2 = "self") {
                    if(e1$size() != e2$size()){
                        stop("Call to operator \"*\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e1$size())
                    for(i in 1:e1$size()){
                        ret[i]$value <-e1[i]$value *e2[i]$value
                    }
                    return(ret)
                    },
                "*" = function(e1 = "self", e2 = "numeric") {
                    if(e1$size() != length(e2)){
                        
                        if(length(e2) == 1){
                            ret<-new(ParameterVector, e1$size())
                            for(i in 1:e1$size()){
                                ret[i]$value <-e1[i]$value *e2
                            }
                            return(ret)
                        }
                        stop("Call to operator \"*\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e1$size())
                    for(i in 1:e1$size()){
                        ret[i]$value <-e1[i]$value *e2[i]
                    }
                    return(ret)
                                },
                "*" = function(e1 = "numeric", e2 = "self") {
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
                    },
                "/" = function(e1 = "self", e2 = "self") {
                    if(e1$size() != e2$size()){
                        stop("Call to operator \"/\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e1$size())
                    for(i in 1:e1$size()){
                        ret[i]$value <-e1[i]$value /e2[i]$value
                    }
                    return(ret)
                    },
                "/" = function(e1 = "self", e2 = "numeric") {
                    if(e1$size() != length(e2)){
                        
                        if(length(e2) == 1){
                            ret<-new(ParameterVector, e1$size())
                            for(i in 1:e1$size()){
                                ret[i]$value <-e1[i]$value /e2
                            }
                            return(ret)
                        }
                        stop("Call to operator \"/\", vectors not equal length")
                    }
                    ret<-new(ParameterVector, e1$size())
                    for(i in 1:e1$size()){
                        ret[i]$value <-e1[i]$value /e2[i]
                    }
                    return(ret)
                                },
                "/" = function(e1 = "numeric", e2 = "self") {
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
                    }),
            saveAs = "Rcpp_ParameterVector"
                          )

  

#setMethod("<-", c(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"), function (e1, e2){
 # (e1$value<- e2$value)})

#setMethod("=", c(e1 = "Rcpp_Parameter", e2 = "numeric"), function (e1, e2){
 # (e1$value<- e2)})

