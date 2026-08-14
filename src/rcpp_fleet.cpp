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

using SharedFleet = std::shared_ptr<FleetInterface>;
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedFleet> create_fleet_() {
  auto obj = std::make_shared<FleetInterface>();
  return Rcpp::XPtr<SharedFleet>(new SharedFleet(obj), true);
}

void set_fleet_dimensions_(Rcpp::XPtr<SharedFleet> xp, int n_years, int n_ages,
                           int n_lengths) {
  (*xp)->n_years = n_years;
  (*xp)->n_ages = n_ages;
  (*xp)->n_lengths = n_lengths;
}

void set_fleet_name_(Rcpp::XPtr<SharedFleet> xp, std::string name) {
  (*xp)->SetName(name);
}

void set_fleet_units_(Rcpp::XPtr<SharedFleet> xp,
                      std::string observed_catch_units,
                      std::string observed_index_units) {
  (*xp)->observed_catch_units = observed_catch_units;
  (*xp)->observed_index_units = observed_index_units;
}

void set_fleet_data_ids_(Rcpp::XPtr<SharedFleet> xp,
                         int observed_age_comp_data_id = -999,
                         int observed_length_comp_data_id = -999,
                         int observed_index_data_id = -999,
                         int observed_catch_data_id = -999) {
  (*xp)->SetObservedAgeCompDataID(observed_age_comp_data_id);
  (*xp)->SetObservedLengthCompDataID(observed_length_comp_data_id);
  (*xp)->SetObservedIndexDataID(observed_index_data_id);
  (*xp)->SetObservedCatchDataID(observed_catch_data_id);
}

void set_fleet_selectivity_id_(Rcpp::XPtr<SharedFleet> xp, int selectivity_id) {
  (*xp)->SetSelectivityID(selectivity_id);
}

void set_fleet_log_q_(Rcpp::XPtr<SharedFleet> xp, Rcpp::NumericVector values,
                      Rcpp::CharacterVector estimation_status) {
  (*xp)->log_q.resize(values.size());
  (*xp)->log_q.set_values(values);
  (*xp)->log_q.set_estimation_status(estimation_status);
}

void set_fleet_log_Fmort_(Rcpp::XPtr<SharedFleet> xp,
                          Rcpp::NumericVector values,
                          Rcpp::CharacterVector estimation_status) {
  (*xp)->log_Fmort.resize(values.size());
  (*xp)->log_Fmort.set_values(values);
  (*xp)->log_Fmort.set_estimation_status(estimation_status);
}

void set_fleet_age_to_length_conversion_(
    Rcpp::XPtr<SharedFleet> xp, Rcpp::NumericVector values,
    Rcpp::CharacterVector estimation_status) {
  (*xp)->age_to_length_conversion.resize(values.size());
  (*xp)->age_to_length_conversion.set_values(values);
  (*xp)->age_to_length_conversion.set_estimation_status(estimation_status);
}

uint32_t get_fleet_id_(Rcpp::XPtr<SharedFleet> xp) { return (*xp)->get_id(); }

Rcpp::XPtr<SharedBase> fleet_to_fims_xptr_(Rcpp::XPtr<SharedFleet> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

/**
 * Function to register fleet classes with the Rcpp module system.
 *
 */
void register_fleet(Rcpp::Module& m) {
  Rcpp::function("create_fleet_", &create_fleet_);
  Rcpp::function("set_fleet_dimensions_", &set_fleet_dimensions_);
  Rcpp::function("set_fleet_name_", &set_fleet_name_);
  Rcpp::function("set_fleet_units_", &set_fleet_units_);
  Rcpp::function("set_fleet_data_ids_", &set_fleet_data_ids_);
  Rcpp::function("set_fleet_selectivity_id_", &set_fleet_selectivity_id_);
  Rcpp::function("set_fleet_log_q_", &set_fleet_log_q_);
  Rcpp::function("set_fleet_log_Fmort_", &set_fleet_log_Fmort_);
  Rcpp::function("set_fleet_age_to_length_conversion_",
                 &set_fleet_age_to_length_conversion_);
  Rcpp::function("get_fleet_id_", &get_fleet_id_);
  Rcpp::function("fleet_to_fims_xptr_", &fleet_to_fims_xptr_);
}
