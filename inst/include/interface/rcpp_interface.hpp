#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP

#include <vector>
#include <map>

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
    bool is_random_effect = false;
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
class fims_rcpp_interface_base {
public:
    static std::vector<fims_rcpp_interface_base*> fims_interface_objects;

    virtual bool add_to_fims_tmb() {
        std::cout << "fims_rcpp_interface_base::add_to_fims_tmb(): Not yet implemented.\n";
        return false;
    }
};
std::vector<fims_rcpp_interface_base*> fims_rcpp_interface_base::fims_interface_objects;

/****************************************************************
 * Recruitment Rcpp interface                                   *
 ***************************************************************/
class recruitment_interface_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, recruitment_interface_base*> recruitment_objects;

    recruitment_interface_base() {

    }

    virtual ~recruitment_interface_base() {
    }

};

uint32_t recruitment_interface_base::id_g = 1;
std::map<uint32_t, recruitment_interface_base* > recruitment_interface_base::recruitment_objects;

/**
 * @brief Interface class for Beverton-Holt recruitment. 
 */
class beverton_holt : public recruitment_interface_base {
public:
    uint32_t id;

    beverton_holt() {
        this->id = recruitment_interface_base::id_g++;
        recruitment_interface_base::recruitment_objects[this->id] = this;
        fims_rcpp_interface_base::fims_interface_objects.push_back(this);
    }

    virtual ~beverton_holt() {
    }

    virtual bool add_to_fims_tmb() {
        std::cout << "beverton_holt::add_to_fims_tmb() : Not yet implemented.\n";
        return false;
    }
};

/****************************************************************
 * Selectivity Rcpp interface                                   *
 ***************************************************************/
class selectivity_interface_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, selectivity_interface_base*> selectivity_objects;

    selectivity_interface_base() {

    }


};

uint32_t selectivity_interface_base::id_g = 1;
std::map<uint32_t, selectivity_interface_base* > selectivity_interface_base::selectivity_objects;

class logistic_selectivity : public selectivity_interface_base {
public:
};

/****************************************************************
 * Growth Rcpp interface                                        *
 ***************************************************************/
class growth_interface_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, growth_interface_base*> growth_objects;

    growth_interface_base() {

    }


};

uint32_t growth_interface_base::id_g = 1;
std::map<uint32_t, growth_interface_base* > growth_interface_base::growth_objects;

/****************************************************************
 * Maturity Rcpp interface                                      *
 ***************************************************************/
class maturity_interface_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, maturity_interface_base*> maturity_objects;

    maturity_interface_base() {

    }


};

uint32_t maturity_interface_base::id_g = 1;
std::map<uint32_t, maturity_interface_base* > maturity_interface_base::maturity_objects;

/****************************************************************
 * Natural Mortality Rcpp interface                             *
 ***************************************************************/
class natural_mortality_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, natural_mortality_base*> natural_mortality_objects;

    natural_mortality_base() {

    }


};

uint32_t natural_mortality_base::id_g = 1;
std::map<uint32_t, natural_mortality_base* > natural_mortality_base::natural_mortality_objects;

/****************************************************************
 * Population Rcpp interface                                    *
 ***************************************************************/
class population : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, population*> population_objects;

    population() {

    }


};

uint32_t population::id_g = 1;
std::map<uint32_t, population* > population::population_objects;

/****************************************************************
 * Data Rcpp interface                                          *
 ***************************************************************/
class data_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, data_base*> data_objects;

    data_base() {

    }


};

uint32_t data_base::id_g = 1;
std::map<uint32_t, data_base* > data_base::data_objects;

/****************************************************************
 * Likelihood Rcpp interface                                    *
 ***************************************************************/
class likelihood_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, likelihood_base*> likelihood_objects;

    likelihood_base() {

    }


};

uint32_t likelihood_base::id_g = 1;
std::map<uint32_t, likelihood_base* > likelihood_base::likelihood_objects;

/****************************************************************
 * Fishing Mortality Rcpp interface                             *
 ***************************************************************/
class fishing_mortality : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, fishing_mortality*> fishing_mortality_objects;

    fishing_mortality() {

    }


};
uint32_t fishing_mortality::id_g = 1;
std::map<uint32_t, fishing_mortality* > fishing_mortality::fishing_mortality_objects;

/****************************************************************
 * Fleet Rcpp interface                                         *
 ***************************************************************/
class fleet : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    static std::map<uint32_t, fleet*> fleet_objects;

    fleet() {

    }


};

uint32_t fleet::id_g = 1;
std::map<uint32_t, fleet* > fleet::fleet_objects;

/**
 *
 */
bool create_tmb_model() {

    for (int i = 0; i < fims_rcpp_interface_base::fims_interface_objects.size(); i++) {
        fims_rcpp_interface_base::fims_interface_objects[i]->add_to_fims_tmb();
    }



    //base model 
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
            fims::Information<TMB_FIMS_REAL_TYPE>::get_instance();
    d0->creat_model();

    //first-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
            fims::Information<TMB_FIMS_FIRST_ORDER>::get_instance();
    d1->creat_model();

    //second-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
            fims::Information<TMB_FIMS_SECOND_ORDER>::get_instance();
    d2->creat_model();

    //third-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
            fims::Information<TMB_FIMS_THIRD_ORDER>::get_instance();
    d3->creat_model();


    return true;
}

RCPP_EXPOSED_CLASS(parameter)
RCPP_MODULE(fims) {
    Rcpp::function("create_tmb_model", &create_tmb_model);

    Rcpp::class_<parameter>("parameter")
            .constructor()
            .constructor<double>()
            .constructor<parameter>()
            .field("value", &parameter::value)
            .field("min", &parameter::min)
            .field("max", &parameter::max)
            .field("is_random_effect", &parameter::is_random_effect)
            .field("estimated", &parameter::estimated);

    Rcpp::class_<beverton_holt>("beverton_holt")
            .constructor();
}


#endif /* RCPP_INTERFACE_HPP */