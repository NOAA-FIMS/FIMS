/*
 * File:   rcpp_population.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP

#include "../../../population_dynamics/population/population.hpp"
#include "rcpp_interface_base.hpp"

/****************************************************************
 * Population Rcpp interface                                   *
 ***************************************************************/
/**
 * @brief PopulationInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible Population function
 * */
class PopulationInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g;
  uint32_t id;
  static std::map<uint32_t, PopulationInterfaceBase*> live_objects;

  PopulationInterfaceBase() {
    this->id = PopulationInterfaceBase::id_g++;
    PopulationInterfaceBase::live_objects[this->id] = this;
    PopulationInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual uint32_t get_id() = 0;
};

uint32_t PopulationInterfaceBase::id_g = 1;
std::map<uint32_t, PopulationInterfaceBase*>
    PopulationInterfaceBase::live_objects;

#endif