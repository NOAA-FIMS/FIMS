/**
 * \file rcpp_fleet.cpp
 * \brief Implementation of Rcpp fleet interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_fleet.hpp"

#include <Rcpp.h>
// static id of the FleetInterfaceBase object
uint32_t FleetInterfaceBase::id_g = 1;
// local id of the FleetInterfaceBase object map relating the ID of the
// FleetInterfaceBase to the FleetInterfaceBase objects
std::map<uint32_t, std::shared_ptr<FleetInterfaceBase>>
    FleetInterfaceBase::live_objects;

/**
 * Function to register fleet classes with the Rcpp module system.
 *
 */
void register_fleet(Rcpp::Module& m) {
  Rcpp::class_<FleetInterface>(
      "Fleet",
      "See https://noaa-fims.github.io/FIMS/doxygen/classFleetInterface.html.")
      .constructor()
      .field("log_q", &FleetInterface::log_q)
      .field("log_Fmort", &FleetInterface::log_Fmort)
      .field("n_ages", &FleetInterface::n_ages)
      .field("n_years", &FleetInterface::n_years)
      .field("n_lengths", &FleetInterface::n_lengths)
      .field("observed_landings_units",
             &FleetInterface::observed_landings_units)
      .field("observed_index_units", &FleetInterface::observed_index_units)
      .field("log_index_expected", &FleetInterface::log_index_expected)
      .field("age_to_length_conversion",
             &FleetInterface::age_to_length_conversion)
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
             &CatchAtAgeInterface::lengthcomp_expected)
      .method("get_id", &FleetInterface::get_id)
      .method("SetName", &FleetInterface::SetName)
      .method("GetName", &FleetInterface::GetName)
      .method("SetObservedAgeCompDataID",
              &FleetInterface::SetObservedAgeCompDataID)
      .method("GetObservedAgeCompDataID",
              &FleetInterface::GetObservedAgeCompDataID)
      .method("SetObservedLengthCompDataID",
              &FleetInterface::SetObservedLengthCompDataID)
      .method("GetObservedLengthCompDataID",
              &FleetInterface::GetObservedLengthCompDataID)
      .method("SetObservedIndexDataID", &FleetInterface::SetObservedIndexDataID)
      .method("GetObservedIndexDataID", &FleetInterface::GetObservedIndexDataID)
      .method("SetObservedLandingsDataID",
              &FleetInterface::SetObservedLandingsDataID)
      .method("GetObservedLandingsDataID",
              &FleetInterface::GetObservedLandingsDataID)
      .method("SetSelectivityID", &FleetInterface::SetSelectivityID);
}