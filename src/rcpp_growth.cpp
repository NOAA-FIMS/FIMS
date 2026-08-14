/**
 * \file rcpp_growth.cpp
 * \brief Implementation of Rcpp growth interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_growth.hpp"
// static id of the GrowthInterfaceBase object
uint32_t GrowthInterfaceBase::id_g = 1;
// local id of the GrowthInterfaceBase object map relating the ID of the
// GrowthInterfaceBase to the GrowthInterfaceBase objects
std::map<uint32_t, std::shared_ptr<GrowthInterfaceBase>>
    GrowthInterfaceBase::live_objects;

#include <Rcpp.h>

using SharedEWAAGrowth = std::shared_ptr<EWAAGrowthInterface>;
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedEWAAGrowth> create_ewaa_growth_() {
  auto obj = std::make_shared<EWAAGrowthInterface>();
  return Rcpp::XPtr<SharedEWAAGrowth>(new SharedEWAAGrowth(obj), true);
}

void set_ewaa_growth_data_(Rcpp::XPtr<SharedEWAAGrowth> xp,
                           Rcpp::NumericVector ages,
                           Rcpp::NumericVector weights, int n_years) {
  (*xp)->ages.set_values(ages);
  (*xp)->weights.set_values(weights);
  (*xp)->n_years = n_years;
  (*xp)->initialized = false;
}

uint32_t get_ewaa_growth_id_(Rcpp::XPtr<SharedEWAAGrowth> xp) {
  return (*xp)->get_id();
}

Rcpp::XPtr<SharedBase> ewaa_growth_to_fims_xptr_(
    Rcpp::XPtr<SharedEWAAGrowth> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

/**
 * Function to register growth classes with the Rcpp module system.
 *
 */
void register_growth(Rcpp::Module& m) {
  Rcpp::function("create_ewaa_growth_", &create_ewaa_growth_);
  Rcpp::function("set_ewaa_growth_data_", &set_ewaa_growth_data_);
  Rcpp::function("get_ewaa_growth_id_", &get_ewaa_growth_id_);
  Rcpp::function("ewaa_growth_to_fims_xptr_", &ewaa_growth_to_fims_xptr_);
}
