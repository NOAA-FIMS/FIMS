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

uint32_t GMRFDistributionsInterface::id_g = 1;
std::map<uint32_t, std::shared_ptr<GMRFDistributionsInterface>>
    GMRFDistributionsInterface::live_objects;

#include <Rcpp.h>
/**
 * Function to register distribution classes with the Rcpp module system.
 *
 */
void register_distributions(Rcpp::Module& m) {
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
              &set_distribution_links_adapter<DnormDistributionsInterface>)
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
              &set_distribution_links_adapter<DlnormDistributionsInterface>)
      .field("observed_values", &DlnormDistributionsInterface::observed_values)
      .field("expected_values", &DlnormDistributionsInterface::expected_values)
      .field("log_sd", &DlnormDistributionsInterface::log_sd);

  Rcpp::class_<GMRFDistributionsInterface>(
      "GMRFDistributionsInterface",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classGMRFDistributionsInterface.html.")
      .constructor()
      .method("get_id", &GMRFDistributionsInterface::get_id)
      .method("evaluate", &GMRFDistributionsInterface::evaluate)
      .method("set_distribution_links",
              &set_distribution_links_adapter<GMRFDistributionsInterface>)
      .method("set_precision_builder_id",
              &GMRFDistributionsInterface::set_precision_builder_id)
      .field("lpdf_vec", &GMRFDistributionsInterface::lpdf_vec);

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
              &set_distribution_links_adapter<DmultinomDistributionsInterface>)
      .method("set_note", &DmultinomDistributionsInterface::set_note)
      .field("observed_values",
             &DmultinomDistributionsInterface::observed_values)
      .field("expected_values",
             &DmultinomDistributionsInterface::expected_values)
      .field("dims", &DmultinomDistributionsInterface::dims);
}
