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
  return Rcpp::XPtr<SharedDnormDistribution>(
      new SharedDnormDistribution(obj), true);
}

Rcpp::XPtr<SharedDlnormDistribution> create_dlnorm_distribution_() {
  auto obj = std::make_shared<DlnormDistributionsInterface>();
  return Rcpp::XPtr<SharedDlnormDistribution>(
      new SharedDlnormDistribution(obj), true);
}

Rcpp::XPtr<SharedDmultinomDistribution> create_dmultinom_distribution_() {
  auto obj = std::make_shared<DmultinomDistributionsInterface>();
  return Rcpp::XPtr<SharedDmultinomDistribution>(
      new SharedDmultinomDistribution(obj), true);
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
  Rcpp::function("dnorm_distribution_to_fims_xptr_",
                 &dnorm_distribution_to_fims_xptr_);
  Rcpp::function("create_dlnorm_distribution_", &create_dlnorm_distribution_);
  Rcpp::function("dlnorm_distribution_to_fims_xptr_",
                 &dlnorm_distribution_to_fims_xptr_);
  Rcpp::function("create_dmultinom_distribution_",
                 &create_dmultinom_distribution_);
  Rcpp::function("dmultinom_distribution_to_fims_xptr_",
                 &dmultinom_distribution_to_fims_xptr_);

  Rcpp::class_<DnormDistributionsInterface>(
      "DnormDistribution",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDnormDistributionsInterface.html.")
      .constructor()
      .method("get_id", &DnormDistributionsInterface::get_id)
      .method("evaluate", &DnormDistributionsInterface::evaluate)
      .method("set_observed_data",
              &DnormDistributionsInterface::set_observed_data)
      .method("set_distribution_mean",
              &DnormDistributionsInterface::set_distribution_mean)
      .method("set_distribution_links",
              &DnormDistributionsInterface::set_distribution_links)
      .field("observed_values", &DnormDistributionsInterface::observed_values)
      .field("expected_values", &DnormDistributionsInterface::expected_values)
      .field("expected_mean", &DnormDistributionsInterface::expected_mean)
      .field("log_sd", &DnormDistributionsInterface::log_sd);

  Rcpp::class_<DlnormDistributionsInterface>(
      "DlnormDistribution",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDlnormDistributionsInterface.html.")
      .constructor()
      .method("get_id", &DlnormDistributionsInterface::get_id)
      .method("evaluate", &DlnormDistributionsInterface::evaluate)
      .method("set_observed_data",
              &DlnormDistributionsInterface::set_observed_data)
      .method("set_distribution_links",
              &DlnormDistributionsInterface::set_distribution_links)
      .field("observed_values", &DlnormDistributionsInterface::observed_values)
      .field("expected_values", &DlnormDistributionsInterface::expected_values)
      .field("log_sd", &DlnormDistributionsInterface::log_sd);

  Rcpp::class_<DmultinomDistributionsInterface>(
      "DmultinomDistribution",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDmultinomDistributionsInterface.html.")
      .constructor()
      .method("get_id", &DmultinomDistributionsInterface::get_id)
      .method("evaluate", &DmultinomDistributionsInterface::evaluate)
      .method("set_observed_data",
              &DmultinomDistributionsInterface::set_observed_data)
      .method("set_distribution_links",
              &DmultinomDistributionsInterface::set_distribution_links)
      .method("set_note", &DmultinomDistributionsInterface::set_note)
      .field("observed_values",
             &DmultinomDistributionsInterface::observed_values)
      .field("expected_values",
             &DmultinomDistributionsInterface::expected_values)
      .field("dims", &DmultinomDistributionsInterface::dims);
}
