#ifndef RCPP_RECRUITMENT
#define RCPP_RECRUITMENT

#include "rcpp_interface_base.hpp"

/****************************************************************
 * Recruitment Rcpp interface                                   *
 ***************************************************************/
class recruitment_interface_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    uint32_t id;
    static std::map<uint32_t, recruitment_interface_base*> recruitment_objects;

    recruitment_interface_base() : selectivity_interface_base() {
        this->id = recruitment_interface_base::id_g++;
        recruitment_interface_base::recruitment_objects[this->id] = this;
        fims_rcpp_interface_base::fims_interface_objects.push_back(this);
    }

    virtual ~recruitment_interface_base() {
    }

};

uint32_t recruitment_interface_base::id_g = 1;
std::map<uint32_t, recruitment_interface_base* > recruitment_interface_base::recruitment_objects;

/**
 * @brief Rcpp interface for Beverton-Holt as a S4 object. To instantiate from R:
 * beverton_holt <- new(fims$beverton_holt) 
 */
class beverton_holt : public recruitment_interface_base {
public:
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


#endif
