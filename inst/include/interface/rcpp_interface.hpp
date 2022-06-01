#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP
#include "interface.hpp"
#include "../common/model.hpp"
#include "../common/information.hpp"

#define RCPP_NO_SUGAR
#include <Rcpp.h>

/**
 * @brief RcppInterface class that defines 
 * the interface between R and C++ for parameter types.
 */
class parameter {
public:
    double value;
    double min = std::numeric_limits<double>::min();
    double max = std::numeric_limits<double>::max();
    bool estimated = false;

    parameter(double value, double min,
            double max, bool estimated) :
    value(value), min(min), max(max),
    estimated(estimated) {

    }

    parameter(double value) {
        this->value = value;
    }

    parameter() {
        this->value = 0;
    }
};

/**
 *@brief Base class for all interface objects
 */
class fims_rcpp_interface_base{
public:
   
    virtual bool add_to_fims(){
        std::cout <<"Not yet implemented!";
        return false;
    }
};

//Recruitment Rcpp interface

class recruitment_interface_base : public fims_rcpp_interface_base{
public:
    static uint32_t id_g;
    static std::map<uint32_t, recruitment_interface_base*> recruitment_objects;

    recruitment_interface_base() {

    }


};

uint32_t recruitment_interface_base::id_g = 1;
std::map<uint32_t, std::shared_ptr<recruitment_interface_base> recruitment_interface_base::recruitment_objects;


/**
 * @brief Interface class for Beverton-Holt recruitment. 
 */
class beverton_holt : public recruitment_interface_base {
public:
    uint32_t id;

    beverton_holt() {
        this->id = recruitment_interface_base::id_g++;
        recruitment_interface_base[this->id] = this;
    }
    
    virtual bool add_to_fims(){
        
    }
};


//Selectivity Rcpp interface

class logistic_selectivity {
public:
};

//Growth Rcpp interface

bool create_model() {

    std::shared_ptr<fims::information<FIMS_REAL_TYPE> > a =
            fims::information<FIMS_REAL_TYPE>::get_instance();

    std::shared_ptr<fims::information<FIMS_FIRST_ORDER> > b =
            fims::information<FIMS_FIRST_ORDER>::get_instance();

    std::shared_ptr<fims::information<FIMS_SECOND_ORDER> > c =
            fims::information<FIMS_SECOND_ORDER>::get_instance();

    std::shared_ptr<fims::information<FIMS_THIRD_ORDER> > d =
            fims::information<FIMS_THIRD_ORDER>::get_instance();

    std::cout << a->parameters.size() << std::endl;
    std::cout << b->parameters.size() << std::endl;
    std::cout << c->parameters.size() << std::endl;
    std::cout << d->parameters.size() << std::endl;

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