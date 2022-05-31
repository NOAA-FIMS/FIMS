#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP
#include "interface.hpp"
#include "../common/model.hpp"



#define RCPP_NO_SUGAR
#include <Rcpp.h>


/**
 * @brief RcppInterface class that defines 
 * the interface between R and C++.
 */
class parameter{
    public:
    double value;
    double min = std::numeric_limits<double>::min();
    double max = std::numeric_limits<double>::max();
    bool estimated = false;

    parameter(double value, double min, 
    double max, bool estimated) : 
    value(value), min(min), max(max),
    estimated(estimated){

     }
   
    parameter(double value){
        this->value = value;
    }

    parameter(){
        this->value = 0;
    }
};

//Recruitment Rcpp interface

//Selectivity Rcpp interface


bool create_model(){


return true;
}




RCPP_EXPOSED_CLASS(parameter)
RCPP_MODULE(fims) {
    Rcpp::function("create_model", &create_model);

    Rcpp::class_<parameter>("parameter")
            .constructor()
            .constructor<double>()
            .constructor<parameter>()
            .field("value", &parameter::value)
            .field("min", &parameter::min)
            .field("max", &parameter::max)
            .field("estimated", &parameter::estimated);
}


#endif /* RCPP_INTERFACE_HPP */