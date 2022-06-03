/*
 * File:   rcpp_recruitment.hpp
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
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_RECRUITMENT_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_RECRUITMENT_HPP


#include "rcpp_interface_base.hpp"
#include "../../../population_dynamics/recruitment/recruitment.hpp"

/****************************************************************
 * Recruitment Rcpp interface                                   *
 ***************************************************************/
class RecruitmentInterfaceBase : public FIMSRcppInterfaceBase {
public:
    static uint32_t id_g;
    uint32_t id;
    static std::map<uint32_t, RecruitmentInterfaceBase*> live_objects;

    RecruitmentInterfaceBase() {
        this->id = RecruitmentInterfaceBase::id_g++;
        RecruitmentInterfaceBase::live_objects[this->id] = this;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
    }

    virtual ~RecruitmentInterfaceBase() {
    }

    virtual uint32_t get_id() = 0;
};

uint32_t RecruitmentInterfaceBase::id_g = 1;
std::map<uint32_t, RecruitmentInterfaceBase* > RecruitmentInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Beverton-Holt as an S4 object. To instantiate 
 * from R:
 * beverton_holt <- new(fims$beverton_holt) 
 */
class BevertonHoltRecruitment : public RecruitmentInterfaceBase {
public:
    Parameter steep;
    Parameter rzero;
    Parameter phizero;

    BevertonHoltRecruitment() : RecruitmentInterfaceBase() {

    }

    virtual ~BevertonHoltRecruitment() {
    }

    virtual uint32_t get_id() {
        return this->id;
    }

    virtual bool add_to_fims_tmb() {
        //base model 
        std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
                fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

        std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_REAL_TYPE> > b0 =
                std::make_shared<fims::SRBevertonHolt<TMB_FIMS_REAL_TYPE> >();


        //set relative info
        b0->id = this->id;
        b0->steep = this->steep.value;
        if (this->steep.estimated) {
            if (this->steep.is_random_effect) {
                d0->RegisterRandomEffect(b0->steep);
            } else {
                d0->RegisterRarameter(b0->steep);
            }
        }
        b0->rzero = this->rzero.value;
        if (this->rzero.estimated) {
            if (this->rzero.is_random_effect) {
                d0->RegisterRandomEffect(b0->rzero);
            } else {
                d0->RegisterRarameter(b0->rzero);
            }
        }
        b0->phizero = this->phizero.value;
        if (this->phizero.estimated) {
            if (this->phizero.is_random_effect) {
                d0->RegisterRandomEffect(b0->phizero);
            } else {
                d0->RegisterRarameter(b0->phizero);
            }
        }
        //add to Information
        d0->recruitment_models[b0->id] = b0;


        //first-order derivative
        std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
                fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

        std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_FIRST_ORDER> > b1 =
                std::make_shared<fims::SRBevertonHolt<TMB_FIMS_FIRST_ORDER> >();


        //set relative info
        b1->id = this->id;
        b1->steep = this->steep.value;
        if (this->steep.estimated) {
            if (this->steep.is_random_effect) {
                d1->RegisterRandomEffect(b1->steep);
            } else {
                d1->RegisterRarameter(b1->steep);
            }
        }
        b1->rzero = this->rzero.value;
        if (this->rzero.estimated) {
            if (this->rzero.is_random_effect) {
                d1->RegisterRandomEffect(b1->rzero);
            } else {
                d1->RegisterRarameter(b1->rzero);
            }
        }
        b1->phizero = this->phizero.value;
        if (this->phizero.estimated) {
            if (this->phizero.is_random_effect) {
                d1->RegisterRandomEffect(b1->phizero);
            } else {
                d1->RegisterRarameter(b1->phizero);
            }
        }
        //add to Information
        d1->recruitment_models[b1->id] = b1;

        //second-order derivative
        std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
                fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

        std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_SECOND_ORDER> > b2 =
                std::make_shared<fims::SRBevertonHolt<TMB_FIMS_SECOND_ORDER> >();


        //set relative info
        b2->id = this->id;
        b2->steep = this->steep.value;
        if (this->steep.estimated) {
            if (this->steep.is_random_effect) {
                d2->RegisterRandomEffect(b2->steep);
            } else {
                d2->RegisterRarameter(b2->steep);
            }
        }
        b2->rzero = this->rzero.value;
        if (this->rzero.estimated) {
            if (this->rzero.is_random_effect) {
                d2->RegisterRandomEffect(b2->rzero);
            } else {
                d2->RegisterRarameter(b2->rzero);
            }
        }
        b2->phizero = this->phizero.value;
        if (this->phizero.estimated) {
            if (this->phizero.is_random_effect) {
                d2->RegisterRandomEffect(b2->phizero);
            } else {
                d2->RegisterRarameter(b2->phizero);
            }
        }
        //add to Information
        d2->recruitment_models[b2->id] = b2;


        //third-order derivative
        std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
                fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();


        std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_THIRD_ORDER> > b3 =
                std::make_shared<fims::SRBevertonHolt<TMB_FIMS_THIRD_ORDER> >();


        //set relative info
        b3->id = this->id;
        b3->steep = this->steep.value;
        if (this->steep.estimated) {
            if (this->steep.is_random_effect) {
                d3->RegisterRandomEffect(b3->steep);
            } else {
                d3->RegisterRarameter(b3->steep);
            }
        }
        b3->rzero = this->rzero.value;
        if (this->rzero.estimated) {
            if (this->rzero.is_random_effect) {
                d3->RegisterRandomEffect(b3->rzero);
            } else {
                d3->RegisterRarameter(b3->rzero);
            }
        }
        b3->phizero = this->phizero.value;
        if (this->phizero.estimated) {
            if (this->phizero.is_random_effect) {
                d3->RegisterRandomEffect(b3->phizero);
            } else {
                d3->RegisterRarameter(b3->phizero);
            }
        }
        //add to Information
        d3->recruitment_models[b3->id] = b3;

        return true;
    }
};


#endif
