/**
 * \file rcpp_population.cpp
 * \brief Implementation of Rcpp population interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp"
#include "../inst/include/interface/rcpp/fims_modules.hpp"

// static id of the PopulationInterfaceBase object
uint32_t PopulationInterfaceBase::id_g = 1;
// local id of the PopulationInterfaceBase object map relating the ID of the
// PopulationInterfaceBase to the PopulationInterfaceBase objects
std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>
    PopulationInterfaceBase::live_objects;

    #include <Rcpp.h>
 /**
 * Function to register population classes with the Rcpp module system.
 * @param m The Rcpp module to register the classes with.
 */   
 void register_population(Rcpp::Module& m) {
    Rcpp::class_<PopulationInterface>(
      "Population",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classPopulationInterface.html.")
      .constructor()
      .method("get_id", &PopulationInterface::get_id)
      .field("n_ages", &PopulationInterface::n_ages)
      .field("n_fleets", &PopulationInterface::n_fleets)
      .field("n_years", &PopulationInterface::n_years)
      .field("n_lengths", &PopulationInterface::n_lengths)
      .field("log_M", &PopulationInterface::log_M)
      .field("log_f_multiplier", &PopulationInterface::log_f_multiplier)
      .field("spawning_biomass_ratio",
             &PopulationInterface::spawning_biomass_ratio)
      .field("log_init_naa", &PopulationInterface::log_init_naa)
      .field("ages", &PopulationInterface::ages)
      .method("SetMaturityID", &PopulationInterface::SetMaturityID)
      .method("SetGrowthID", &PopulationInterface::SetGrowthID)
      .method("SetRecruitmentID", &PopulationInterface::SetRecruitmentID)
      .method("AddFleet", &PopulationInterface::AddFleet)
      .method("SetName", &PopulationInterface::SetName)
      .method("GetName", &PopulationInterface::GetName);
 }