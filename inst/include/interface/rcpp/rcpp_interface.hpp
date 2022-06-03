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


#include "rcpp_objects/rcpp_recruitment.hpp"
#include "rcpp_objects/rcpp_selectivity.hpp"
#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_maturity.hpp"
#include "rcpp_objects/rcpp_natural_mortality.hpp"
#include "rcpp_objects/rcpp_fishing_mortality.hpp"
#include "rcpp_objects/rcpp_fleet.hpp"
#include "rcpp_objects/rcpp_population.hpp"
#include "rcpp_objects/rcpp_likelihoods.hpp"



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
Rcpp::class_<logistic_selectivity>("logistic_selectivity")
.constructor()
.field("median", &logistic_selectivity::median)
.field("slope", &logistic_selectivity::slope);

}



#endif /* RCPP_INTERFACE_HPP */