/**
 * \file rcpp_maturity.cpp
 * \brief Implementation of Rcpp maturity interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_maturity.hpp"
// static id of the MaturityInterfaceBase object
uint32_t MaturityInterfaceBase::id_g = 1;
// local id of the MaturityInterfaceBase object map relating the ID of the
// MaturityInterfaceBase to the MaturityInterfaceBase objects
std::map<uint32_t, std::shared_ptr<MaturityInterfaceBase>>
    MaturityInterfaceBase::live_objects;

#include <Rcpp.h>

using SharedLogisticMaturity = std::shared_ptr<LogisticMaturityInterface>;
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedLogisticMaturity> create_logistic_maturity_() {
  auto obj = std::make_shared<LogisticMaturityInterface>();
  return Rcpp::XPtr<SharedLogisticMaturity>(new SharedLogisticMaturity(obj),
                                            true);
}

void set_logistic_maturity_inflection_point_(
    Rcpp::XPtr<SharedLogisticMaturity> xp, Rcpp::NumericVector values,
    Rcpp::CharacterVector estimation_status) {
  (*xp)->inflection_point.resize(values.size());
  (*xp)->inflection_point.set_values(values);
  (*xp)->inflection_point.set_estimation_status(estimation_status);
}

void set_logistic_maturity_slope_(Rcpp::XPtr<SharedLogisticMaturity> xp,
                                  Rcpp::NumericVector values,
                                  Rcpp::CharacterVector estimation_status) {
  (*xp)->slope.resize(values.size());
  (*xp)->slope.set_values(values);
  (*xp)->slope.set_estimation_status(estimation_status);
}

uint32_t get_logistic_maturity_id_(Rcpp::XPtr<SharedLogisticMaturity> xp) {
  return (*xp)->get_id();
}

Rcpp::XPtr<SharedBase> logistic_maturity_to_fims_xptr_(
    Rcpp::XPtr<SharedLogisticMaturity> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

/**
 * Function to register maturity classes with the Rcpp module system.
 *
 */
void register_maturity(Rcpp::Module& m) {
  Rcpp::function("create_logistic_maturity_", &create_logistic_maturity_);
  Rcpp::function("set_logistic_maturity_inflection_point_",
                 &set_logistic_maturity_inflection_point_);
  Rcpp::function("set_logistic_maturity_slope_", &set_logistic_maturity_slope_);
  Rcpp::function("get_logistic_maturity_id_", &get_logistic_maturity_id_);
  Rcpp::function("logistic_maturity_to_fims_xptr_",
                 &logistic_maturity_to_fims_xptr_);
}
