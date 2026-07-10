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
/**
 * Function to register distribution classes with the Rcpp module system.
 *
 */
void register_distributions(Rcpp::Module& m) {
  Rcpp::class_<DistributionsInterfaceBase>(
      "DistributionBase",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDistributionsInterfaceBase.html.")
      .constructor()
      .field("likelihood_type", &DistributionsInterfaceBase::likelihood_type_m)
      .field("distribution_type", &DistributionsInterfaceBase::distribution_type_m)
      .field("dims", &DistributionsInterfaceBase::dims_m)
      .field("observed_values", &DistributionsInterfaceBase::observed_values_m)
      .field("expected_values", &DistributionsInterfaceBase::expected_values_m)
      .field("uncertainty_values", &DistributionsInterfaceBase::uncertainty_values_m)
      .field("observed_key", &DistributionsInterfaceBase::observed_key_m)
      .field("expected_key", &DistributionsInterfaceBase::expected_key_m)
      .field("uncertainty_key", &DistributionsInterfaceBase::uncertainty_key_m)
      .field("observed_subvector", &DistributionsInterfaceBase::observed_subvector_m)
      .field("expected_subvector", &DistributionsInterfaceBase::expected_subvector_m)
      .field("uncertainty_subvector", &DistributionsInterfaceBase::uncertainty_subvector_m)
      .field("lambda_values", &DistributionsInterfaceBase::lambda_values_m)
      ;
      
  Rcpp::class_<DistributionsInterface>(
      "Distribution",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDistributionsInterface.html.")
      .constructor()
      .derives<DistributionsInterfaceBase>("DistributionBase")
      .method("get_id", &DistributionsInterface::get_id)
      .method("evaluate", &DistributionsInterface::evaluate)
      .method("set_values",
              &DistributionsInterface::set_values)
      .method("set_id_links",
              &DistributionsInterface::set_id_links)
      .method("set_subvector_indices",
              &DistributionsInterface::set_subvector_indices)
      ;

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
