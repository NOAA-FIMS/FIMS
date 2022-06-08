/*
 * File:   rcpp_growth.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file 
 * for reuse information.
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
  double ages;
  double weights;
  
  std::map<double, double> ewaa = make_map(ages, weights);

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
    d0->growth_models[b0->id] = b0;

    return true;
  }
};

#endif
