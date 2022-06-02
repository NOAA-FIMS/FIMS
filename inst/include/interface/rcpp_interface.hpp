#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP

/*
 * File:   rcpp_interface.hpp
 *
 * Author: Matthew Supernaw
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov
 *
 * Created on May 31, 2022 at 12:04 PM
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 *
 * This software is a "United States Government Work" under the terms of the
 * United States Copyright Act.  It was written as part of the author's official
 * duties as a United States Government employee and thus cannot be copyrighted.
 * This software is freely available to the public for use. The National Oceanic
 * And Atmospheric Administration and the U.S. Government have not placed any
 * restriction on its use or reproduction.  Although all reasonable efforts have
 * been taken to ensure the accuracy and reliability of the software and data,
 * the National Oceanic And Atmospheric Administration and the U.S. Government
 * do not and cannot warrant the performance or results that may be obtained by
 * using this  software or data. The National Oceanic And Atmospheric
 * Administration and the U.S. Government disclaim all warranties, express or
 * implied, including warranties of performance, merchantability or fitness
 * for any particular purpose.
 *
 * Please cite the author(s) in any work or product based on this material.
 *
 */


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
    parameter steep;
    parameter rzero;
    parameter phizero;

    beverton_holt() {
        this->id = recruitment_interface_base::id_g++;
        recruitment_interface_base::recruitment_objects[this->id] = this;
        fims_rcpp_interface_base::fims_interface_objects.push_back(this);
    }

    virtual ~beverton_holt() {
    }

    virtual bool add_to_fims_tmb() {
        //base model 
        std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
                fims::Information<TMB_FIMS_REAL_TYPE>::get_instance();

        std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_REAL_TYPE> > b0 =
                std::make_shared<fims::SRBevertonHolt<TMB_FIMS_REAL_TYPE> >();


        //set relative info
        b0->id = this->id;
        b0->steep = this->steep.value;
        if (this->steep.estimated) {
            if (this->steep.is_random_effect) {
                d0->register_random_effect(b0->steep);
            } else {
                d0->register_parameter(b0->steep);
            }
        }
        b0->rzero = this->rzero.value;
        if (this->rzero.estimated) {
            if (this->rzero.is_random_effect) {
                d0->register_random_effect(b0->rzero);
            } else {
                d0->register_parameter(b0->rzero);
            }
        }
        b0->phizero = this->phizero.value;
        if (this->phizero.estimated) {
            if (this->phizero.is_random_effect) {
                d0->register_random_effect(b0->phizero);
            } else {
                d0->register_parameter(b0->phizero);
            }
        }
        //add to Information
        d0->recruitment_models[b0->id] = b0;


        //first-order derivative
        std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
                fims::Information<TMB_FIMS_FIRST_ORDER>::get_instance();

        std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_FIRST_ORDER> > b1 =
                std::make_shared<fims::SRBevertonHolt<TMB_FIMS_FIRST_ORDER> >();


        //set relative info
        b1->id = this->id;
        b1->steep = this->steep.value;
        if (this->steep.estimated) {
            if (this->steep.is_random_effect) {
                d1->register_random_effect(b1->steep);
            } else {
                d1->register_parameter(b1->steep);
            }
        }
        b1->rzero = this->rzero.value;
        if (this->rzero.estimated) {
            if (this->rzero.is_random_effect) {
                d1->register_random_effect(b1->rzero);
            } else {
                d1->register_parameter(b1->rzero);
            }
        }
        b1->phizero = this->phizero.value;
        if (this->phizero.estimated) {
            if (this->phizero.is_random_effect) {
                d1->register_random_effect(b1->phizero);
            } else {
                d1->register_parameter(b1->phizero);
            }
        }
        //add to Information
        d1->recruitment_models[b1->id] = b1;

        //second-order derivative
        std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
                fims::Information<TMB_FIMS_SECOND_ORDER>::get_instance();

        std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_SECOND_ORDER> > b2 =
                std::make_shared<fims::SRBevertonHolt<TMB_FIMS_SECOND_ORDER> >();


        //set relative info
        b2->id = this->id;
        b2->steep = this->steep.value;
        if (this->steep.estimated) {
            if (this->steep.is_random_effect) {
                d2->register_random_effect(b2->steep);
            } else {
                d2->register_parameter(b2->steep);
            }
        }
        b2->rzero = this->rzero.value;
        if (this->rzero.estimated) {
            if (this->rzero.is_random_effect) {
                d2->register_random_effect(b2->rzero);
            } else {
                d2->register_parameter(b2->rzero);
            }
        }
        b2->phizero = this->phizero.value;
        if (this->phizero.estimated) {
            if (this->phizero.is_random_effect) {
                d2->register_random_effect(b2->phizero);
            } else {
                d2->register_parameter(b2->phizero);
            }
        }
        //add to Information
        d2->recruitment_models[b2->id] = b2;


        //third-order derivative
        std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
                fims::Information<TMB_FIMS_THIRD_ORDER>::get_instance();


        std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_THIRD_ORDER> > b3 =
                std::make_shared<fims::SRBevertonHolt<TMB_FIMS_THIRD_ORDER> >();


        //set relative info
        b3->id = this->id;
        b3->steep = this->steep.value;
        if (this->steep.estimated) {
            if (this->steep.is_random_effect) {
                d3->register_random_effect(b3->steep);
            } else {
                d3->register_parameter(b3->steep);
            }
        }
        b3->rzero = this->rzero.value;
        if (this->rzero.estimated) {
            if (this->rzero.is_random_effect) {
                d3->register_random_effect(b3->rzero);
            } else {
                d3->register_parameter(b3->rzero);
            }
        }
        b3->phizero = this->phizero.value;
        if (this->phizero.estimated) {
            if (this->phizero.is_random_effect) {
                d3->register_random_effect(b3->phizero);
            } else {
                d3->register_parameter(b3->phizero);
            }
        }
        //add to Information
        d3->recruitment_models[b3->id] = b3;

        return true;
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
            .constructor()
            .field("steep", &beverton_holt::steep)
            .field("rzero", &beverton_holt::rzero)
            .field("phizero", &beverton_holt::phizero);

}


#endif /* RCPP_INTERFACE_HPP */