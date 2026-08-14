/**
 * \file rcpp_selectivity.cpp
 * \brief Implementation of Rcpp selectivity interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp"

// static id of the SelectivityInterfaceBase object
uint32_t SelectivityInterfaceBase::id_g = 1;
// local id of the SelectivityInterfaceBase object map relating the ID of the
// SelectivityInterfaceBase to the SelectivityInterfaceBase objects
std::map<uint32_t, std::shared_ptr<SelectivityInterfaceBase>>
    SelectivityInterfaceBase::live_objects;
#include <Rcpp.h>

using SharedLogisticSelectivity = std::shared_ptr<LogisticSelectivityInterface>;
using SharedDoubleLogisticSelectivity =
    std::shared_ptr<DoubleLogisticSelectivityInterface>;
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedLogisticSelectivity> create_logistic_selectivity_() {
  auto obj = std::make_shared<LogisticSelectivityInterface>();
  return Rcpp::XPtr<SharedLogisticSelectivity>(
      new SharedLogisticSelectivity(obj), true);
}

Rcpp::XPtr<SharedDoubleLogisticSelectivity>
create_double_logistic_selectivity_() {
  auto obj = std::make_shared<DoubleLogisticSelectivityInterface>();
  return Rcpp::XPtr<SharedDoubleLogisticSelectivity>(
      new SharedDoubleLogisticSelectivity(obj), true);
}

void set_logistic_selectivity_inflection_point_(
    Rcpp::XPtr<SharedLogisticSelectivity> xp, Rcpp::NumericVector values,
    Rcpp::CharacterVector estimation_status) {
  (*xp)->inflection_point.resize(values.size());
  (*xp)->inflection_point.set_values(values);
  (*xp)->inflection_point.set_estimation_status(estimation_status);
}

void set_logistic_selectivity_slope_(Rcpp::XPtr<SharedLogisticSelectivity> xp,
                                     Rcpp::NumericVector values,
                                     Rcpp::CharacterVector estimation_status) {
  (*xp)->slope.resize(values.size());
  (*xp)->slope.set_values(values);
  (*xp)->slope.set_estimation_status(estimation_status);
}

void set_double_logistic_selectivity_inflection_point_asc_(
    Rcpp::XPtr<SharedDoubleLogisticSelectivity> xp, Rcpp::NumericVector values,
    Rcpp::CharacterVector estimation_status) {
  (*xp)->inflection_point_asc.resize(values.size());
  (*xp)->inflection_point_asc.set_values(values);
  (*xp)->inflection_point_asc.set_estimation_status(estimation_status);
}

void set_double_logistic_selectivity_slope_asc_(
    Rcpp::XPtr<SharedDoubleLogisticSelectivity> xp, Rcpp::NumericVector values,
    Rcpp::CharacterVector estimation_status) {
  (*xp)->slope_asc.resize(values.size());
  (*xp)->slope_asc.set_values(values);
  (*xp)->slope_asc.set_estimation_status(estimation_status);
}

void set_double_logistic_selectivity_inflection_point_desc_(
    Rcpp::XPtr<SharedDoubleLogisticSelectivity> xp, Rcpp::NumericVector values,
    Rcpp::CharacterVector estimation_status) {
  (*xp)->inflection_point_desc.resize(values.size());
  (*xp)->inflection_point_desc.set_values(values);
  (*xp)->inflection_point_desc.set_estimation_status(estimation_status);
}

void set_double_logistic_selectivity_slope_desc_(
    Rcpp::XPtr<SharedDoubleLogisticSelectivity> xp, Rcpp::NumericVector values,
    Rcpp::CharacterVector estimation_status) {
  (*xp)->slope_desc.resize(values.size());
  (*xp)->slope_desc.set_values(values);
  (*xp)->slope_desc.set_estimation_status(estimation_status);
}

uint32_t get_logistic_selectivity_id_(
    Rcpp::XPtr<SharedLogisticSelectivity> xp) {
  return (*xp)->get_id();
}

uint32_t get_double_logistic_selectivity_id_(
    Rcpp::XPtr<SharedDoubleLogisticSelectivity> xp) {
  return (*xp)->get_id();
}

Rcpp::XPtr<SharedBase> logistic_selectivity_to_fims_xptr_(
    Rcpp::XPtr<SharedLogisticSelectivity> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

Rcpp::XPtr<SharedBase> double_logistic_selectivity_to_fims_xptr_(
    Rcpp::XPtr<SharedDoubleLogisticSelectivity> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

/**
 * Function to register selectivity classes with the Rcpp module system.
 *
 */
void register_selectivity(Rcpp::Module& m) {
  Rcpp::function("create_logistic_selectivity_", &create_logistic_selectivity_);
  Rcpp::function("set_logistic_selectivity_inflection_point_",
                 &set_logistic_selectivity_inflection_point_);
  Rcpp::function("set_logistic_selectivity_slope_",
                 &set_logistic_selectivity_slope_);
  Rcpp::function("get_logistic_selectivity_id_", &get_logistic_selectivity_id_);
  Rcpp::function("logistic_selectivity_to_fims_xptr_",
                 &logistic_selectivity_to_fims_xptr_);
  Rcpp::function("create_double_logistic_selectivity_",
                 &create_double_logistic_selectivity_);
  Rcpp::function("set_double_logistic_selectivity_inflection_point_asc_",
                 &set_double_logistic_selectivity_inflection_point_asc_);
  Rcpp::function("set_double_logistic_selectivity_slope_asc_",
                 &set_double_logistic_selectivity_slope_asc_);
  Rcpp::function("set_double_logistic_selectivity_inflection_point_desc_",
                 &set_double_logistic_selectivity_inflection_point_desc_);
  Rcpp::function("set_double_logistic_selectivity_slope_desc_",
                 &set_double_logistic_selectivity_slope_desc_);
  Rcpp::function("get_double_logistic_selectivity_id_",
                 &get_double_logistic_selectivity_id_);
  Rcpp::function("double_logistic_selectivity_to_fims_xptr_",
                 &double_logistic_selectivity_to_fims_xptr_);
}
