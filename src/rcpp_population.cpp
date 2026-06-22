/**
 * \file rcpp_population.cpp
 * \brief Implementation of Rcpp population interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp"

// static id of the PopulationInterfaceBase object
uint32_t PopulationInterfaceBase::id_g = 1;
// local id of the PopulationInterfaceBase object map relating the ID of the
// PopulationInterfaceBase to the PopulationInterfaceBase objects
std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>
    PopulationInterfaceBase::live_objects;

#include <Rcpp.h>
/**
 * Function to register population classes with the Rcpp module system.
 *
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
      .field("total_landings_weight", &PopulationInterface::total_landings_weight)
      .field("total_landings_numbers",
             &PopulationInterface::total_landings_numbers)
      .field("mortality_F", &PopulationInterface::mortality_F)
      .field("mortality_M", &PopulationInterface::mortality_M)
      .field("mortality_Z", &PopulationInterface::mortality_Z)
      .field("numbers_at_age", &PopulationInterface::numbers_at_age)
      .field("unfished_numbers_at_age",
             &PopulationInterface::unfished_numbers_at_age)
      .field("biomass", &PopulationInterface::biomass)
      .field("spawning_biomass", &PopulationInterface::spawning_biomass)
      .field("unfished_biomass", &PopulationInterface::unfished_biomass)
      .field("unfished_spawning_biomass",
             &PopulationInterface::unfished_spawning_biomass)
      .field("proportion_mature_at_age",
             &PopulationInterface::proportion_mature_at_age)
      .field("expected_recruitment", &PopulationInterface::expected_recruitment)
      .field("sum_selectivity", &PopulationInterface::sum_selectivity)
      .method("SetMaturityID", &PopulationInterface::SetMaturityID)
      .method("SetGrowthID", &PopulationInterface::SetGrowthID)
      .method("SetRecruitmentID", &PopulationInterface::SetRecruitmentID)
      .method("AddFleet", &PopulationInterface::AddFleet)
      .method("SetName", &PopulationInterface::SetName)
      .method("GetName", &PopulationInterface::GetName);
}