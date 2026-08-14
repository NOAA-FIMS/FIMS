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
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedCatchAtAge> create_catch_at_age_() {
  auto obj = std::make_shared<CatchAtAgeInterface>();
  return Rcpp::XPtr<SharedCatchAtAge>(new SharedCatchAtAge(obj), true);
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
  Rcpp::function("catch_at_age_to_fims_xptr_", &catch_at_age_to_fims_xptr_);

  Rcpp::class_<CatchAtAgeInterface>(
      "CatchAtAge",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classCatchAtAgeInterface.html.")
      .constructor()
      .method("AddPopulation", &CatchAtAgeInterface::AddPopulation)
      .method("get_output", &CatchAtAgeInterface::to_json)
      .method("GetId", &CatchAtAgeInterface::get_id)
      .method("DoReporting", &CatchAtAgeInterface::DoReporting)
      .method("IsReporting", &CatchAtAgeInterface::IsReporting);
}
