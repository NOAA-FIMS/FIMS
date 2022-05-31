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

    Parameter(double value, double min, double max){
        this->value = value;
        this->min = min;
        this->max = max;
    }

    Parameter(double value){
        this->value = value;
    }

    Parameter(){
        this->value = 0;
    }
};




#endif /* RCPP_INTERFACE_HPP */