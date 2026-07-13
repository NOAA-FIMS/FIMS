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
      .field("lengths", &FleetInterface::lengths)
      .field("observed_catch_units",
             &FleetInterface::observed_catch_units)
      .field("observed_index_units", &FleetInterface::observed_index_units)
      .field("log_index_expected", &FleetInterface::log_index_expected)
      .field("age_to_length_conversion",
             &FleetInterface::age_to_length_conversion)
      .field("catch_numbers_at_age",
             &FleetInterface::
                 catch_numbers_at_age)
      .field("catch_weight_at_age",
             &FleetInterface::
                 catch_weight_at_age)
      .field("catch_numbers_at_length",
             &FleetInterface::
                 catch_numbers_at_length)
      .field("catch_weight",
             &FleetInterface::catch_weight)
      .field("catch_numbers",
             &FleetInterface::catch_numbers)
      .field("catch_expected",
             &FleetInterface::catch_expected)
      .field("log_catch_expected",
             &FleetInterface::log_catch_expected)
      .field("agecomp_proportion",
             &FleetInterface::agecomp_proportion)
      .field("lengthcomp_proportion",
             &FleetInterface::lengthcomp_proportion)
      .field("index_numbers_at_age",
             &FleetInterface::index_numbers_at_age)
      .field("index_weight_at_age",
             &FleetInterface::index_weight_at_age)
      .field("index_numbers_at_length",
             &FleetInterface::
                 index_numbers_at_length)
      .field("index_weight",
             &FleetInterface::index_weight)
      .field("index_numbers",
             &FleetInterface::index_numbers)
      .field("index_expected",
             &FleetInterface::index_expected)
      .field("agecomp_expected",
             &FleetInterface::agecomp_expected)
      .field("lengthcomp_expected",
             &FleetInterface::lengthcomp_expected)
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
      .method("SetObservedCatchDataID",
              &FleetInterface::SetObservedCatchDataID)
      .method("GetObservedCatchDataID",
              &FleetInterface::GetObservedCatchDataID)
      .method("SetSelectivityID", &FleetInterface::SetSelectivityID);
}