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

#include "../inst/include/interface/rcpp/rcpp_include.hpp"

/**
 * Function to register growth classes with the Rcpp module system.
 *
 */
void register_growth(Rcpp::Module& m) {
  Rcpp::class_<EWAAGrowthInterface>(
      "EWAAGrowth",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classEWAAGrowthInterface.html.")
      .constructor()
      .field("ages", &EWAAGrowthInterface::ages, "Ages for each age class.")
      .field("weights", &EWAAGrowthInterface::weights,
             "Weights for each age class.")
      .field("n_years", &EWAAGrowthInterface::n_years, "Number of years.")
      .method("get_id", &EWAAGrowthInterface::get_id)
      .method("evaluate", &EWAAGrowthInterface::evaluate);

  Rcpp::class_<VonBertalanffySchnuteGrowthInterface>(
      "VonBertalanffySchnuteGrowth")
      .constructor()
      .field("mean_length_young",
             &VonBertalanffySchnuteGrowthInterface::mean_length_young)
      .field("mean_length_old",
             &VonBertalanffySchnuteGrowthInterface::mean_length_old)
      .field("growth_coefficient",
             &VonBertalanffySchnuteGrowthInterface::growth_coefficient)
      .field(
          "reference_age_for_length_young",
          &VonBertalanffySchnuteGrowthInterface::reference_age_for_length_young)
      .field(
          "reference_age_for_length_old",
          &VonBertalanffySchnuteGrowthInterface::reference_age_for_length_old)
      .field("length_weight_a",
             &VonBertalanffySchnuteGrowthInterface::length_weight_a)
      .field("length_weight_b",
             &VonBertalanffySchnuteGrowthInterface::length_weight_b)
      .field("length_at_age_sd_at_reference_ages",
             &VonBertalanffySchnuteGrowthInterface::
                 length_at_age_sd_at_reference_ages)
      .field("n_ages", &VonBertalanffySchnuteGrowthInterface::n_ages)
      .method("get_id", &VonBertalanffySchnuteGrowthInterface::get_id)
      .method("evaluate", &VonBertalanffySchnuteGrowthInterface::evaluate)
      .method("to_json", &VonBertalanffySchnuteGrowthInterface::to_json)
#ifdef TMB_MODEL
      .method("add_to_fims_tmb",
              &VonBertalanffySchnuteGrowthInterface::add_to_fims_tmb)
#endif
      ;

  Rcpp::class_<VonBertalanffyTraditionalGrowthInterface>(
      "VonBertalanffyTraditionalGrowth")
      .constructor()
      .field("asymptotic_length",
             &VonBertalanffyTraditionalGrowthInterface::asymptotic_length)
      .field("growth_coefficient",
             &VonBertalanffyTraditionalGrowthInterface::growth_coefficient)
      .field("age_at_zero_length",
             &VonBertalanffyTraditionalGrowthInterface::age_at_zero_length)
      .field("reference_age_for_length_young",
             &VonBertalanffyTraditionalGrowthInterface::
                 reference_age_for_length_young)
      .field("reference_age_for_length_old",
             &VonBertalanffyTraditionalGrowthInterface::
                 reference_age_for_length_old)
      .field("length_weight_a",
             &VonBertalanffyTraditionalGrowthInterface::length_weight_a)
      .field("length_weight_b",
             &VonBertalanffyTraditionalGrowthInterface::length_weight_b)
      .field("length_at_age_sd_at_reference_ages",
             &VonBertalanffyTraditionalGrowthInterface::
                 length_at_age_sd_at_reference_ages)
      .field("log_sd_asymptotic_length",
             &VonBertalanffyTraditionalGrowthInterface::
                 log_sd_asymptotic_length)
      .field("log_sd_growth_coefficient",
             &VonBertalanffyTraditionalGrowthInterface::
                 log_sd_growth_coefficient)
      .field("log_sd_age_at_zero_length",
             &VonBertalanffyTraditionalGrowthInterface::
                 log_sd_age_at_zero_length)
      .field("asymptotic_length_growth_coefficient_logit_corr",
             &VonBertalanffyTraditionalGrowthInterface::
                 asymptotic_length_growth_coefficient_logit_corr)
      .field("asymptotic_length_age_at_zero_length_logit_corr",
             &VonBertalanffyTraditionalGrowthInterface::
                 asymptotic_length_age_at_zero_length_logit_corr)
      .field("growth_coefficient_age_at_zero_length_logit_corr",
             &VonBertalanffyTraditionalGrowthInterface::
                 growth_coefficient_age_at_zero_length_logit_corr)
      .field("n_ages", &VonBertalanffyTraditionalGrowthInterface::n_ages)
      .method("get_id", &VonBertalanffyTraditionalGrowthInterface::get_id)
      .method("evaluate", &VonBertalanffyTraditionalGrowthInterface::evaluate)
      .method("to_json", &VonBertalanffyTraditionalGrowthInterface::to_json)
#ifdef TMB_MODEL
      .method("add_to_fims_tmb",
              &VonBertalanffyTraditionalGrowthInterface::add_to_fims_tmb)
#endif
      ;
}
