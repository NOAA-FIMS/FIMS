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
 * ewaa <- new(fims$EWAAgrowth)
 */
class EWAAGrowthInterface : public GrowthInterfaceBase {
 public:
  double weights;
  double ages;

  EWAAGrowthInterface() : GrowthInterfaceBase() {}

  virtual ~EWAAGrowthInterface() {}

  virtual uint32_t get_id() { return this->id; }

  inline std::map<double, double> make_map(double x, double y){
    std::map<double, double> mymap;
    mymap.insert(std::pair<double, double>(x, y));
    return mymap;
  }
  
  std::map<double, double> ewaa = make_map(this->ages, this->weights);

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

        
    // add to Information
    d0->growth_models[b0->id] = b0;

    // // first-order derivative
    // std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
    //     fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    // std::shared_ptr<fims::EWAAgrowth<TMB_FIMS_FIRST_ORDER> > b1 =
    //     std::make_shared<fims::EWAAgrowth<TMB_FIMS_FIRST_ORDER> >();

    // // set relative info
    // b1->id = this->id;
    // b1->ewaa = this->ewaa;
     
    // // add to Information
    // d1->growth_models[b1->id] = b1;

    // // second-order derivative
    // std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
    //     fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    // std::shared_ptr<fims::EWAAgrowth<TMB_FIMS_SECOND_ORDER> > b2 =
    //     std::make_shared<fims::EWAAgrowth<TMB_FIMS_SECOND_ORDER> >();

    // // set relative info
    // b2->id = this->id;
    // b2->ewaa = this->ewaa;
    // // add to Information
    // d2->growth_models[b2->id] = b2;

    // // third-order derivative
    // std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
    //     fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    // std::shared_ptr<fims::EWAAgrowth<TMB_FIMS_THIRD_ORDER> > b3 =
    //     std::make_shared<fims::EWAAgrowth<TMB_FIMS_THIRD_ORDER> >();

    // // set relative info
    // b3->id = this->id;
    // b3->ewaa = this->ewaa;

    // // add to Information
    // d3->growth_models[b3->id] = b3;
    return true;
  }
  };

#endif
