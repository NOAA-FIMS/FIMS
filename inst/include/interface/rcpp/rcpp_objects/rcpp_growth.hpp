/*
 * File:   rcpp_growth.hpp
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
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_GROWTH_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_GROWTH_HPP

#include "../../../population_dynamics/growth/growth.hpp"
#include "rcpp_interface_base.hpp"

/****************************************************************
 * Growth Rcpp interface                                   *
 ***************************************************************/
class GrowthInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g;
  uint32_t id;
  static std::map<uint32_t, GrowthInterfaceBase*> live_objects;

  GrowthInterfaceBase() {
    this->id = GrowthInterfaceBase::id_g++;
    GrowthInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~GrowthInterfaceBase() {}

  virtual uint32_t get_id() = 0;
};

uint32_t GrowthInterfaceBase::id_g = 1;
std::map<uint32_t, GrowthInterfaceBase*>
    GrowthInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for EWAAgrowth as an S4 object. To instantiate
 * from R:
 * ewaa <- new(fims$EWAAGrowthInterface)
 */
class EWAAGrowthInterface : public GrowthInterfaceBase {
 public:
  Parameter ewaa;

  EWAAGrowthInterface() : GrowthInterfaceBase() {}

  virtual ~EWAAGrowthInterface() {}

  virtual uint32_t get_id() { return this->id; }

  virtual bool add_to_fims_tmb() {
    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::EWAAgrowth<TMB_FIMS_REAL_TYPE> > b0 =
        std::make_shared<fims::EWAAgrowth<TMB_FIMS_REAL_TYPE> >();

    // set relative info
    b0->id = this->id;
    b0->ewaa = this->ewaa.value;
    if (this->ewaa.estimated) {
      if (this->ewaa.is_random_effect) {
        d0->RegisterRandomEffect(b0->ewaa);
      } else {
        d0->RegisterRarameter(b0->ewaa);
      }
    }
   
    // add to Information
    d0->Growth_models[b0->id] = b0;

    // first-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::EWAAgrowth<TMB_FIMS_FIRST_ORDER> > b1 =
        std::make_shared<fims::EWAAgrowth<TMB_FIMS_FIRST_ORDER> >();

    // set relative info
    b1->id = this->id;
    b1->ewaa = this->ewaa.value;
    if (this->ewaa.estimated) {
      if (this->ewaa.is_random_effect) {
        d1->RegisterRandomEffect(b1->ewaa);
      } else {
        d1->RegisterRarameter(b1->ewaa);
      }
    }

    // add to Information
    d1->Growth_models[b1->id] = b1;

    // second-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_SECOND_ORDER> > b2 =
        std::make_shared<fims::SRBevertonHolt<TMB_FIMS_SECOND_ORDER> >();

    // set relative info
    b2->id = this->id;
    b2->ewaa = this->ewaa.value;
    if (this->ewaa.estimated) {
      if (this->ewaa.is_random_effect) {
        d2->RegisterRandomEffect(b2->ewaa);
      } else {
        d2->RegisterRarameter(b2->ewaa);
      }
    }

    // add to Information
    d2->Growth_models[b2->id] = b2;

    // third-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_THIRD_ORDER> > b3 =
        std::make_shared<fims::SRBevertonHolt<TMB_FIMS_THIRD_ORDER> >();

    // set relative info
    b3->id = this->id;
    b3->ewaa = this->ewaa.value;
    if (this->ewaa.estimated) {
      if (this->ewaa.is_random_effect) {
        d3->RegisterRandomEffect(b3->ewaa);
      } else {
        d3->RegisterRarameter(b3->ewaa);
      }
    }
    
    // add to Information
    d3->Growth_models[b3->id] = b3;

    return true;
  }
};

#endif
