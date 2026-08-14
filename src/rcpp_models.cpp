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

using SharedCatchAtAge = std::shared_ptr<CatchAtAgeInterface>;
using SharedPopulation = std::shared_ptr<PopulationInterface>;
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedCatchAtAge> create_catch_at_age_() {
  auto obj = std::make_shared<CatchAtAgeInterface>();
  return Rcpp::XPtr<SharedCatchAtAge>(new SharedCatchAtAge(obj), true);
}

void add_population_to_catch_at_age_(
    Rcpp::XPtr<SharedCatchAtAge> catch_at_age_xp,
    Rcpp::XPtr<SharedPopulation> population_xp) {
  (*catch_at_age_xp)->population_ids->insert((*population_xp)->get_id());
  (*population_xp)->initialize_catch_at_age = true;
}

std::string get_catch_at_age_output_(Rcpp::XPtr<SharedCatchAtAge> xp) {
  return (*xp)->to_json();
}

uint32_t get_catch_at_age_id_(Rcpp::XPtr<SharedCatchAtAge> xp) {
  return (*xp)->get_id();
}

void set_catch_at_age_reporting_(Rcpp::XPtr<SharedCatchAtAge> xp, bool report) {
  (*xp)->DoReporting(report);
}

bool get_catch_at_age_reporting_(Rcpp::XPtr<SharedCatchAtAge> xp) {
  return (*xp)->IsReporting();
}

Rcpp::XPtr<SharedBase> catch_at_age_to_fims_xptr_(
    Rcpp::XPtr<SharedCatchAtAge> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

/**
 * Function to register fishery model classes with the Rcpp module system.
 *
 */
void register_fishery_models(Rcpp::Module& m) {
  Rcpp::function("create_catch_at_age_", &create_catch_at_age_);
  Rcpp::function("add_population_to_catch_at_age_",
                 &add_population_to_catch_at_age_);
  Rcpp::function("get_catch_at_age_output_", &get_catch_at_age_output_);
  Rcpp::function("get_catch_at_age_id_", &get_catch_at_age_id_);
  Rcpp::function("set_catch_at_age_reporting_", &set_catch_at_age_reporting_);
  Rcpp::function("get_catch_at_age_reporting_", &get_catch_at_age_reporting_);
  Rcpp::function("catch_at_age_to_fims_xptr_", &catch_at_age_to_fims_xptr_);
}
