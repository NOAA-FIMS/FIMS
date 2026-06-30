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
  Rcpp::class_<CatchAtAgeInterface>(
      "CatchAtAge",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classCatchAtAgeInterface.html.")
      .constructor()
      .field("total_landings_weight",
             &CatchAtAgeInterface::
                 total_landings_weight)
      .field("total_landings_numbers",
             &CatchAtAgeInterface::
                 total_landings_numbers)
      .field("mortality_F",
             &CatchAtAgeInterface::mortality_F)
      .field("mortality_M",
             &CatchAtAgeInterface::mortality_M)
      .field("mortality_Z",
             &CatchAtAgeInterface::mortality_Z)
      .field("numbers_at_age",
             &CatchAtAgeInterface::numbers_at_age)
      .field("unfished_numbers_at_age",
             &CatchAtAgeInterface::
                 unfished_numbers_at_age)
      .field("biomass",
             &CatchAtAgeInterface::biomass)
      .field("spawning_biomass",
             &CatchAtAgeInterface::
                 spawning_biomass)
      .field("unfished_biomass",
             &CatchAtAgeInterface::unfished_biomass)
      .field("unfished_spawning_biomass",
             &CatchAtAgeInterface::
                 unfished_spawning_biomass)
      .field("proportion_mature_at_age",
             &CatchAtAgeInterface::
                 proportion_mature_at_age)
      .field("expected_recruitment",
             &CatchAtAgeInterface::
                 expected_recruitment)
      .field("sum_selectivity",
             &CatchAtAgeInterface::sum_selectivity);
      .field("landings_numbers_at_age",
             &CatchAtAgeInterface::
                 landings_numbers_at_age)
      .field("landings_weight_at_age",
             &CatchAtAgeInterface::
                 landings_weight_at_age)
      .field("landings_numbers_at_length",
             &CatchAtAgeInterface::
                 landings_numbers_at_length)
      .field("landings_weight",
             &CatchAtAgeInterface::landings_weight)
      .field("landings_numbers",
             &CatchAtAgeInterface::landings_numbers)
      .field("landings_expected",
             &CatchAtAgeInterface::landings_expected)
      .field("log_landings_expected",
             &CatchAtAgeInterface::log_landings_expected)
      .field("agecomp_proportion",
             &CatchAtAgeInterface::agecomp_proportion)
      .field("lengthcomp_proportion",
             &CatchAtAgeInterface::lengthcomp_proportion)
      .field("index_numbers_at_age",
             &CatchAtAgeInterface::index_numbers_at_age)
      .field("index_weight_at_age",
             &CatchAtAgeInterface::index_weight_at_age)
      .field("index_numbers_at_length",
             &CatchAtAgeInterface::
                 index_numbers_at_length)
      .field("index_weight",
             &CatchAtAgeInterface::index_weight)
      .field("index_numbers",
             &CatchAtAgeInterface::index_numbers)
      .field("index_expected",
             &CatchAtAgeInterface::index_expected)
      .field("log_index_expected",
             &CatchAtAgeInterface::log_index_expected)
      .field("catch_index",
             &CatchAtAgeInterface::catch_index)
      .field("agecomp_expected",
             &CatchAtAgeInterface::agecomp_expected)
      .field("lengthcomp_expected",
             &CatchAtAgeInterface::lengthcomp_expected);
      .method("AddPopulation", &CatchAtAgeInterface::AddPopulation)
      .method("get_output", &CatchAtAgeInterface::to_json)
      .method("GetId", &CatchAtAgeInterface::get_id)
      .method("DoReporting", &CatchAtAgeInterface::DoReporting)
      .method("IsReporting", &CatchAtAgeInterface::IsReporting);
}