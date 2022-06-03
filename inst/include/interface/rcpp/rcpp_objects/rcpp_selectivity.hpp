#ifndef RCPP_SELECTIVITY_HPP
#define RCPP_SELECTIVITY_HPP

#include "rcpp_interface_base.hpp"
#include "../../../population_dynamics/selectivity/selectivity.hpp"

/****************************************************************
 * Selectivity Rcpp interface                                   *
 ***************************************************************/
class selectivity_interface_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    uint32_t id;
    static std::map<uint32_t, selectivity_interface_base*> selectivity_objects;

    selectivity_interface_base() {
        this->id = selectivity_interface_base::id_g++;
        selectivity_interface_base::selectivity_objects[this->id] = this;
        fims_rcpp_interface_base::fims_interface_objects.push_back(this);
    }


};

uint32_t selectivity_interface_base::id_g = 1;
std::map<uint32_t, selectivity_interface_base* > selectivity_interface_base::selectivity_objects;


/**
 * @brief Rcpp interface for logistic selectivity as a S4 object. To instantiate from R:
 * logistic_selectivity <- new(fims$logistic_selectivity) 
 */
class logistic_selectivity : public selectivity_interface_base {
public:
    parameter median;
    parameter slope;

    logistic_selectivity() : selectivity_interface_base() {
    }

    virtual ~logistic_selectivity() {
    }

    virtual bool add_to_fims_tmb() {

        std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
                fims::Information<TMB_FIMS_REAL_TYPE>::get_instance();

        std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_REAL_TYPE> > ls0 =
                std::make_shared<fims::LogisticSelectivity<TMB_FIMS_REAL_TYPE> >();


        //set relative info
        ls0->id = this->id;
        ls0->median = this->median.value;
        if (this->median.estimated) {
            if (this->median.is_random_effect) {
                d0->register_random_effect(ls0->median);
            } else {
                d0->register_parameter(ls0->median);
            }
        }
        ls0->slope = this->slope.value;
        if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
                d0->register_random_effect(ls0->slope);
            } else {
                d0->register_parameter(ls0->slope);
            }
        }

        //add to Information
        d0->selectivity_models[ls0->id] = ls0;

        std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
                fims::Information<TMB_FIMS_FIRST_ORDER>::get_instance();

        std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_FIRST_ORDER> > ls1 =
                std::make_shared<fims::LogisticSelectivity<TMB_FIMS_FIRST_ORDER> >();


        //set relative info
        ls1->id = this->id;
        ls1->median = this->median.value;
        if (this->median.estimated) {
            if (this->median.is_random_effect) {
                d1->register_random_effect(ls1->median);
            } else {
                d1->register_parameter(ls1->median);
            }
        }
        ls1->slope = this->slope.value;
        if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
                d1->register_random_effect(ls1->slope);
            } else {
                d1->register_parameter(ls1->slope);
            }
        }

        //add to Information
        d1->selectivity_models[ls1->id] = ls1;

        std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
                fims::Information<TMB_FIMS_SECOND_ORDER>::get_instance();

        std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_SECOND_ORDER> > ls2 =
                std::make_shared<fims::LogisticSelectivity<TMB_FIMS_SECOND_ORDER> >();


        //set relative info
        ls2->id = this->id;
        ls2->median = this->median.value;
        if (this->median.estimated) {
            if (this->median.is_random_effect) {
                d2->register_random_effect(ls2->median);
            } else {
                d2->register_parameter(ls2->median);
            }
        }
        ls2->slope = this->slope.value;
        if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
                d2->register_random_effect(ls2->slope);
            } else {
                d2->register_parameter(ls2->slope);
            }
        }

        //add to Information
        d2->selectivity_models[ls2->id] = ls2;


        std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
                fims::Information<TMB_FIMS_THIRD_ORDER>::get_instance();

        std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_THIRD_ORDER> > ls3 =
                std::make_shared<fims::LogisticSelectivity<TMB_FIMS_THIRD_ORDER> >();


        //set relative info
        ls3->id = this->id;
        ls3->median = this->median.value;
        if (this->median.estimated) {
            if (this->median.is_random_effect) {
                d3->register_random_effect(ls3->median);
            } else {
                d3->register_parameter(ls3->median);
            }
        }
        ls3->slope = this->slope.value;
        if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
                d3->register_random_effect(ls3->slope);
            } else {
                d3->register_parameter(ls3->slope);
            }
        }

        //add to Information
        d3->selectivity_models[ls3->id] = ls3;


        return true;
    }

};


#endif