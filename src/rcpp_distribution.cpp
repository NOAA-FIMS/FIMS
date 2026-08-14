/**
 * \file rcpp_distribution.cpp
 * \brief Implementation of Rcpp distribution interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_distribution.hpp"

// static id of the DistributionsInterfaceBase object
uint32_t DistributionsInterfaceBase::id_g = 1;
// local id of the DistributionsInterfaceBase object map relating the ID of the
// DistributionsInterfaceBase to the DistributionsInterfaceBase objects
std::map<uint32_t, std::shared_ptr<DistributionsInterfaceBase>>
    DistributionsInterfaceBase::live_objects;

#include <Rcpp.h>

using SharedDnormDistribution = std::shared_ptr<DnormDistributionsInterface>;
using SharedDlnormDistribution = std::shared_ptr<DlnormDistributionsInterface>;
using SharedDmultinomDistribution =
    std::shared_ptr<DmultinomDistributionsInterface>;
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedDnormDistribution> create_dnorm_distribution_() {
  auto obj = std::make_shared<DnormDistributionsInterface>();
  return Rcpp::XPtr<SharedDnormDistribution>(new SharedDnormDistribution(obj),
                                             true);
}

Rcpp::XPtr<SharedDlnormDistribution> create_dlnorm_distribution_() {
  auto obj = std::make_shared<DlnormDistributionsInterface>();
  return Rcpp::XPtr<SharedDlnormDistribution>(new SharedDlnormDistribution(obj),
                                              true);
}

Rcpp::XPtr<SharedDmultinomDistribution> create_dmultinom_distribution_() {
  auto obj = std::make_shared<DmultinomDistributionsInterface>();
  return Rcpp::XPtr<SharedDmultinomDistribution>(
      new SharedDmultinomDistribution(obj), true);
}

void set_dnorm_observed_values_(Rcpp::XPtr<SharedDnormDistribution> xp,
                                Rcpp::NumericVector values,
                                Rcpp::CharacterVector estimation_status) {
  (*xp)->observed_values.resize(values.size());
  (*xp)->observed_values.set_values(values);
  (*xp)->observed_values.set_estimation_status(estimation_status);
}

void set_dnorm_expected_values_(Rcpp::XPtr<SharedDnormDistribution> xp,
                                Rcpp::NumericVector values,
                                Rcpp::CharacterVector estimation_status) {
  (*xp)->expected_values.resize(values.size());
  (*xp)->expected_values.set_values(values);
  (*xp)->expected_values.set_estimation_status(estimation_status);
}

void set_dnorm_expected_mean_(Rcpp::XPtr<SharedDnormDistribution> xp,
                              Rcpp::NumericVector values,
                              Rcpp::CharacterVector estimation_status) {
  (*xp)->expected_mean.resize(values.size());
  (*xp)->expected_mean.set_values(values);
  (*xp)->expected_mean.set_estimation_status(estimation_status);
}

void set_dnorm_log_sd_(Rcpp::XPtr<SharedDnormDistribution> xp,
                       Rcpp::NumericVector values,
                       Rcpp::CharacterVector estimation_status) {
  (*xp)->log_sd.resize(values.size());
  (*xp)->log_sd.set_values(values);
  (*xp)->log_sd.set_estimation_status(estimation_status);
}

void set_dnorm_observed_data_id_(Rcpp::XPtr<SharedDnormDistribution> xp,
                                 int observed_data_id) {
  (*xp)->set_observed_data(observed_data_id);
}

void set_dnorm_distribution_links_(Rcpp::XPtr<SharedDnormDistribution> xp,
                                   std::string input_type,
                                   Rcpp::IntegerVector ids) {
  (*xp)->set_distribution_links(input_type, ids);
}

void set_dnorm_distribution_mean_(Rcpp::XPtr<SharedDnormDistribution> xp,
                                  double value) {
  (*xp)->set_distribution_mean(value);
}

uint32_t get_dnorm_distribution_id_(Rcpp::XPtr<SharedDnormDistribution> xp) {
  return (*xp)->get_id();
}

void set_dlnorm_observed_values_(Rcpp::XPtr<SharedDlnormDistribution> xp,
                                 Rcpp::NumericVector values,
                                 Rcpp::CharacterVector estimation_status) {
  (*xp)->observed_values.resize(values.size());
  (*xp)->observed_values.set_values(values);
  (*xp)->observed_values.set_estimation_status(estimation_status);
}

void set_dlnorm_expected_values_(Rcpp::XPtr<SharedDlnormDistribution> xp,
                                 Rcpp::NumericVector values,
                                 Rcpp::CharacterVector estimation_status) {
  (*xp)->expected_values.resize(values.size());
  (*xp)->expected_values.set_values(values);
  (*xp)->expected_values.set_estimation_status(estimation_status);
}

void set_dlnorm_log_sd_(Rcpp::XPtr<SharedDlnormDistribution> xp,
                        Rcpp::NumericVector values,
                        Rcpp::CharacterVector estimation_status) {
  (*xp)->log_sd.resize(values.size());
  (*xp)->log_sd.set_values(values);
  (*xp)->log_sd.set_estimation_status(estimation_status);
}

void set_dlnorm_observed_data_id_(Rcpp::XPtr<SharedDlnormDistribution> xp,
                                  int observed_data_id) {
  (*xp)->set_observed_data(observed_data_id);
}

void set_dlnorm_distribution_links_(Rcpp::XPtr<SharedDlnormDistribution> xp,
                                    std::string input_type,
                                    Rcpp::IntegerVector ids) {
  (*xp)->set_distribution_links(input_type, ids);
}

uint32_t get_dlnorm_distribution_id_(Rcpp::XPtr<SharedDlnormDistribution> xp) {
  return (*xp)->get_id();
}

void set_dmultinom_observed_values_(Rcpp::XPtr<SharedDmultinomDistribution> xp,
                                    Rcpp::NumericVector values,
                                    Rcpp::CharacterVector estimation_status) {
  (*xp)->observed_values.resize(values.size());
  (*xp)->observed_values.set_values(values);
  (*xp)->observed_values.set_estimation_status(estimation_status);
}

void set_dmultinom_expected_values_(Rcpp::XPtr<SharedDmultinomDistribution> xp,
                                    Rcpp::NumericVector values,
                                    Rcpp::CharacterVector estimation_status) {
  (*xp)->expected_values.resize(values.size());
  (*xp)->expected_values.set_values(values);
  (*xp)->expected_values.set_estimation_status(estimation_status);
}

void set_dmultinom_dims_(Rcpp::XPtr<SharedDmultinomDistribution> xp,
                         Rcpp::NumericVector values) {
  (*xp)->dims.set_values(values);
}

void set_dmultinom_observed_data_id_(Rcpp::XPtr<SharedDmultinomDistribution> xp,
                                     int observed_data_id) {
  (*xp)->set_observed_data(observed_data_id);
}

void set_dmultinom_distribution_links_(
    Rcpp::XPtr<SharedDmultinomDistribution> xp, std::string input_type,
    Rcpp::IntegerVector ids) {
  (*xp)->set_distribution_links(input_type, ids);
}

void set_dmultinom_note_(Rcpp::XPtr<SharedDmultinomDistribution> xp,
                         std::string note) {
  (*xp)->set_note(note);
}

uint32_t get_dmultinom_distribution_id_(
    Rcpp::XPtr<SharedDmultinomDistribution> xp) {
  return (*xp)->get_id();
}

Rcpp::XPtr<SharedBase> dnorm_distribution_to_fims_xptr_(
    Rcpp::XPtr<SharedDnormDistribution> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

Rcpp::XPtr<SharedBase> dlnorm_distribution_to_fims_xptr_(
    Rcpp::XPtr<SharedDlnormDistribution> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

Rcpp::XPtr<SharedBase> dmultinom_distribution_to_fims_xptr_(
    Rcpp::XPtr<SharedDmultinomDistribution> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}
/**
 * Function to register distribution classes with the Rcpp module system.
 *
 */
void register_distributions(Rcpp::Module& m) {
  Rcpp::function("create_dnorm_distribution_", &create_dnorm_distribution_);
  Rcpp::function("set_dnorm_observed_values_", &set_dnorm_observed_values_);
  Rcpp::function("set_dnorm_expected_values_", &set_dnorm_expected_values_);
  Rcpp::function("set_dnorm_expected_mean_", &set_dnorm_expected_mean_);
  Rcpp::function("set_dnorm_log_sd_", &set_dnorm_log_sd_);
  Rcpp::function("set_dnorm_observed_data_id_", &set_dnorm_observed_data_id_);
  Rcpp::function("set_dnorm_distribution_links_",
                 &set_dnorm_distribution_links_);
  Rcpp::function("set_dnorm_distribution_mean_", &set_dnorm_distribution_mean_);
  Rcpp::function("get_dnorm_distribution_id_", &get_dnorm_distribution_id_);
  Rcpp::function("dnorm_distribution_to_fims_xptr_",
                 &dnorm_distribution_to_fims_xptr_);
  Rcpp::function("create_dlnorm_distribution_", &create_dlnorm_distribution_);
  Rcpp::function("set_dlnorm_observed_values_", &set_dlnorm_observed_values_);
  Rcpp::function("set_dlnorm_expected_values_", &set_dlnorm_expected_values_);
  Rcpp::function("set_dlnorm_log_sd_", &set_dlnorm_log_sd_);
  Rcpp::function("set_dlnorm_observed_data_id_", &set_dlnorm_observed_data_id_);
  Rcpp::function("set_dlnorm_distribution_links_",
                 &set_dlnorm_distribution_links_);
  Rcpp::function("get_dlnorm_distribution_id_", &get_dlnorm_distribution_id_);
  Rcpp::function("dlnorm_distribution_to_fims_xptr_",
                 &dlnorm_distribution_to_fims_xptr_);
  Rcpp::function("create_dmultinom_distribution_",
                 &create_dmultinom_distribution_);
  Rcpp::function("set_dmultinom_observed_values_",
                 &set_dmultinom_observed_values_);
  Rcpp::function("set_dmultinom_expected_values_",
                 &set_dmultinom_expected_values_);
  Rcpp::function("set_dmultinom_dims_", &set_dmultinom_dims_);
  Rcpp::function("set_dmultinom_observed_data_id_",
                 &set_dmultinom_observed_data_id_);
  Rcpp::function("set_dmultinom_distribution_links_",
                 &set_dmultinom_distribution_links_);
  Rcpp::function("set_dmultinom_note_", &set_dmultinom_note_);
  Rcpp::function("get_dmultinom_distribution_id_",
                 &get_dmultinom_distribution_id_);
  Rcpp::function("dmultinom_distribution_to_fims_xptr_",
                 &dmultinom_distribution_to_fims_xptr_);
}
