#ifndef RCPP_INTERFACE_HPP
#define RCPP_INTERFACE_HPP

#include <Rcpp.h>
#include "interface.hpp"

/**
 * @brief RcppInterface class that defines 
 * the interface between R and C++.
 */
class Parameter{
    public:
    double value;
    double min = std::numeric_limits<double>::min();
    double max = std::numeric_limits<double>::max();
    bool estimated = false;

    Parameter(double value, double min, 
    double max, bool estimated) : 
    value(value), min(min), max(max),
    estimated(estimated){

     }
   
    Parameter(double value){
        this->value = value;
    }

    Parameter(){
        this->value = 0;
    }
};

RCPP_MODULE(rmas) {
    class_<Parameter>("Parameter")
            .constructor()
            .constructor<double>()
            .constructor<Parameter>()
            .field("value", &Parameter::value)
            .field("min", &Parameter::min)
            .field("max", &Parameter::max)
            .field("estimated", &Parameter::estimated)
}


#endif /* RCPP_INTERFACE_HPP */