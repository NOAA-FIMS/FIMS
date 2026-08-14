/**
 * \file rcpp_data.cpp
 * \brief Implementation of Rcpp data interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_data.hpp"
// static id of the DataInterfaceBase object
uint32_t DataInterfaceBase::id_g = 1;
// local id of the DataInterfaceBase object map relating the ID of the
// DataInterfaceBase to the DataInterfaceBase objects
std::map<uint32_t, std::shared_ptr<DataInterfaceBase>>
    DataInterfaceBase::live_objects;

using SharedAgeComp = std::shared_ptr<AgeCompDataInterface>;
using SharedLengthComp = std::shared_ptr<LengthCompDataInterface>;
using SharedCatch = std::shared_ptr<CatchDataInterface>;
using SharedIndex = std::shared_ptr<IndexDataInterface>;
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedAgeComp> create_age_comp_(int ymax = 0, int amax = 0) {
  auto obj = std::make_shared<AgeCompDataInterface>(ymax, amax);
  return Rcpp::XPtr<SharedAgeComp>(new SharedAgeComp(obj), true);
}

Rcpp::XPtr<SharedLengthComp> create_length_comp_(int ymax = 0, int lmax = 0) {
  auto obj = std::make_shared<LengthCompDataInterface>(ymax, lmax);
  return Rcpp::XPtr<SharedLengthComp>(new SharedLengthComp(obj), true);
}

Rcpp::XPtr<SharedCatch> create_catch_(int ymax = 0) {
  auto obj = std::make_shared<CatchDataInterface>(ymax);
  return Rcpp::XPtr<SharedCatch>(new SharedCatch(obj), true);
}

Rcpp::XPtr<SharedIndex> create_index_(int ymax = 0) {
  auto obj = std::make_shared<IndexDataInterface>(ymax);
  return Rcpp::XPtr<SharedIndex>(new SharedIndex(obj), true);
}

void set_age_comp_data_(Rcpp::XPtr<SharedAgeComp> xp,
                        Rcpp::NumericVector values) {
  (*xp)->age_comp_data.set_values(values);
}

void set_age_comp_uncertainty_(Rcpp::XPtr<SharedAgeComp> xp,
                               Rcpp::NumericVector values) {
  (*xp)->uncertainty.set_values(values);
}

void set_length_comp_data_(Rcpp::XPtr<SharedLengthComp> xp,
                           Rcpp::NumericVector values) {
  (*xp)->length_comp_data.set_values(values);
}

void set_length_comp_uncertainty_(Rcpp::XPtr<SharedLengthComp> xp,
                                  Rcpp::NumericVector values) {
  (*xp)->uncertainty.set_values(values);
}

void set_catch_data_(Rcpp::XPtr<SharedCatch> xp, Rcpp::NumericVector values) {
  (*xp)->catch_data.set_values(values);
}

void set_catch_uncertainty_(Rcpp::XPtr<SharedCatch> xp,
                            Rcpp::NumericVector values) {
  (*xp)->uncertainty.set_values(values);
}

void set_index_data_(Rcpp::XPtr<SharedIndex> xp, Rcpp::NumericVector values) {
  (*xp)->index_data.set_values(values);
}

void set_index_uncertainty_(Rcpp::XPtr<SharedIndex> xp,
                            Rcpp::NumericVector values) {
  (*xp)->uncertainty.set_values(values);
}

uint32_t get_age_comp_id_(Rcpp::XPtr<SharedAgeComp> xp) {
  return (*xp)->get_id();
}

uint32_t get_length_comp_id_(Rcpp::XPtr<SharedLengthComp> xp) {
  return (*xp)->get_id();
}

uint32_t get_catch_id_(Rcpp::XPtr<SharedCatch> xp) { return (*xp)->get_id(); }

uint32_t get_index_id_(Rcpp::XPtr<SharedIndex> xp) { return (*xp)->get_id(); }

Rcpp::XPtr<SharedBase> age_comp_to_fims_xptr_(Rcpp::XPtr<SharedAgeComp> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

Rcpp::XPtr<SharedBase> length_comp_to_fims_xptr_(
    Rcpp::XPtr<SharedLengthComp> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

Rcpp::XPtr<SharedBase> catch_to_fims_xptr_(Rcpp::XPtr<SharedCatch> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

Rcpp::XPtr<SharedBase> index_to_fims_xptr_(Rcpp::XPtr<SharedIndex> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

/**
 * Function to register data classes with the Rcpp module system.
 */
void register_data(Rcpp::Module& m) {
  Rcpp::function("create_age_comp_", &create_age_comp_);
  Rcpp::function("set_age_comp_data_", &set_age_comp_data_);
  Rcpp::function("set_age_comp_uncertainty_", &set_age_comp_uncertainty_);
  Rcpp::function("get_age_comp_id_", &get_age_comp_id_);
  Rcpp::function("age_comp_to_fims_xptr_", &age_comp_to_fims_xptr_);
  Rcpp::function("create_length_comp_", &create_length_comp_);
  Rcpp::function("set_length_comp_data_", &set_length_comp_data_);
  Rcpp::function("set_length_comp_uncertainty_", &set_length_comp_uncertainty_);
  Rcpp::function("get_length_comp_id_", &get_length_comp_id_);
  Rcpp::function("length_comp_to_fims_xptr_", &length_comp_to_fims_xptr_);
  Rcpp::function("create_catch_", &create_catch_);
  Rcpp::function("set_catch_data_", &set_catch_data_);
  Rcpp::function("set_catch_uncertainty_", &set_catch_uncertainty_);
  Rcpp::function("get_catch_id_", &get_catch_id_);
  Rcpp::function("catch_to_fims_xptr_", &catch_to_fims_xptr_);
  Rcpp::function("create_index_", &create_index_);
  Rcpp::function("set_index_data_", &set_index_data_);
  Rcpp::function("set_index_uncertainty_", &set_index_uncertainty_);
  Rcpp::function("get_index_id_", &get_index_id_);
  Rcpp::function("index_to_fims_xptr_", &index_to_fims_xptr_);
}
