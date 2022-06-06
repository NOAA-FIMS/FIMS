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
 * beverton_holt <- new(fims$beverton_holt)
 */
class EWAAGrowthInterface : public GrowthInterfaceBase {
 public:
  std::map<double, double> ewaa;

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
    b0->ewaa = this->ewaa;
   
    // add to Information
    d0->Growth_models[b0->id] = b0;

    return true;
  }
};

#endif
