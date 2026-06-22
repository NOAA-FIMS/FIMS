/**
 * \file rcpp_models.cpp
 * \brief Implementation of Rcpp fishery model interfaces for the FIMS
 * framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_models.hpp"
// static id of the FleetInterfaceBase object
uint32_t FisheryModelInterfaceBase::id_g = 1;

// FleetInterfaceBase to the FleetInterfaceBase objects
std::map<uint32_t, std::shared_ptr<FisheryModelInterfaceBase>>
    FisheryModelInterfaceBase::live_objects;

#include <Rcpp.h>

/**
 * Function to register fishery model classes with the Rcpp module system.
 *
 */
void register_fishery_models(Rcpp::Module& m) {
  Rcpp::class_<CatchAtAgePopulationDerivedQuantitiesInterface>(
      "CatchAtAgePopulationDerivedQuantities",
      "Catch-at-age population derived quantities exposed as RealVector "
      "members.")
      .constructor()
      .method("Initialize",
              &CatchAtAgePopulationDerivedQuantitiesInterface::Initialize)
      .method("Fill", &CatchAtAgePopulationDerivedQuantitiesInterface::Fill)
      .field("total_landings_weight",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 total_landings_weight)
      .field("total_landings_numbers",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 total_landings_numbers)
      .field("mortality_F",
             &CatchAtAgePopulationDerivedQuantitiesInterface::mortality_F)
      .field("mortality_M",
             &CatchAtAgePopulationDerivedQuantitiesInterface::mortality_M)
      .field("mortality_Z",
             &CatchAtAgePopulationDerivedQuantitiesInterface::mortality_Z)
      .field("numbers_at_age",
             &CatchAtAgePopulationDerivedQuantitiesInterface::numbers_at_age)
      .field("unfished_numbers_at_age",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 unfished_numbers_at_age)
      .field("biomass",
             &CatchAtAgePopulationDerivedQuantitiesInterface::biomass)
      .field("spawning_biomass",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 spawning_biomass)
      .field("unfished_biomass",
             &CatchAtAgePopulationDerivedQuantitiesInterface::unfished_biomass)
      .field("unfished_spawning_biomass",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 unfished_spawning_biomass)
      .field("proportion_mature_at_age",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 proportion_mature_at_age)
      .field("expected_recruitment",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 expected_recruitment)
      .field("sum_selectivity",
             &CatchAtAgePopulationDerivedQuantitiesInterface::sum_selectivity);

  Rcpp::class_<CatchAtAgeFleetDerivedQuantitiesInterface>(
      "CatchAtAgeFleetDerivedQuantities",
      "Catch-at-age fleet derived quantities exposed as RealVector members.")
      .constructor()
      .method("Initialize", &CatchAtAgeFleetDerivedQuantitiesInterface::Initialize)
      .method("Fill", &CatchAtAgeFleetDerivedQuantitiesInterface::Fill)
      .field("landings_numbers_at_age",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 landings_numbers_at_age)
      .field("landings_weight_at_age",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 landings_weight_at_age)
      .field("landings_numbers_at_length",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 landings_numbers_at_length)
      .field("landings_weight",
             &CatchAtAgeFleetDerivedQuantitiesInterface::landings_weight)
      .field("landings_numbers",
             &CatchAtAgeFleetDerivedQuantitiesInterface::landings_numbers)
      .field("landings_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::landings_expected)
      .field("log_landings_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::log_landings_expected)
      .field("agecomp_proportion",
             &CatchAtAgeFleetDerivedQuantitiesInterface::agecomp_proportion)
      .field("lengthcomp_proportion",
             &CatchAtAgeFleetDerivedQuantitiesInterface::lengthcomp_proportion)
      .field("index_numbers_at_age",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_numbers_at_age)
      .field("index_weight_at_age",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_weight_at_age)
      .field("index_numbers_at_length",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 index_numbers_at_length)
      .field("index_weight",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_weight)
      .field("index_numbers",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_numbers)
      .field("index_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_expected)
      .field("log_index_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::log_index_expected)
      .field("catch_index",
             &CatchAtAgeFleetDerivedQuantitiesInterface::catch_index)
      .field("expected_catch",
             &CatchAtAgeFleetDerivedQuantitiesInterface::expected_catch)
      .field("expected_index",
             &CatchAtAgeFleetDerivedQuantitiesInterface::expected_index)
      .field("agecomp_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::agecomp_expected)
      .field("lengthcomp_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::lengthcomp_expected);

  Rcpp::class_<CatchAtAgeInterface>(
      "CatchAtAge",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classCatchAtAgeInterface.html.")
      .constructor()
      .method("AddPopulation", &CatchAtAgeInterface::AddPopulation)
      .method("get_output", &CatchAtAgeInterface::to_json)
      .method("GetId", &CatchAtAgeInterface::get_id)
      .method("DoReporting", &CatchAtAgeInterface::DoReporting)
      .method("IsReporting", &CatchAtAgeInterface::IsReporting)
      .method("InitializePopulationDerivedQuantities",
              &CatchAtAgeInterface::InitializePopulationDerivedQuantities)
      .method("InitializeFleetDerivedQuantities",
              &CatchAtAgeInterface::InitializeFleetDerivedQuantities)
      .field("population_derived_quantities",
             &CatchAtAgeInterface::population_derived_quantities)
      .field("fleet_derived_quantities",
             &CatchAtAgeInterface::fleet_derived_quantities);
}
