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

  Rcpp::class_<VonBSchnuteGrowthInterface>("VonBSchnuteGrowth")
      .constructor()
      .field("mean_length_young",
             &VonBSchnuteGrowthInterface::mean_length_young)
      .field("mean_length_old",
             &VonBSchnuteGrowthInterface::mean_length_old)
      .field("von_bertalanffy_coefficient_K",
             &VonBSchnuteGrowthInterface::von_bertalanffy_coefficient_K)
      .field("reference_age_for_length_1",
             &VonBSchnuteGrowthInterface::reference_age_for_length_1)
      .field("reference_age_for_length_2",
             &VonBSchnuteGrowthInterface::reference_age_for_length_2)
      .field("length_weight_a",
             &VonBSchnuteGrowthInterface::length_weight_a)
      .field("length_weight_b",
             &VonBSchnuteGrowthInterface::length_weight_b)
      .field("length_at_age_sd_at_ref_ages",
             &VonBSchnuteGrowthInterface::length_at_age_sd_at_ref_ages)
      .field("log_sd_length_at_ref_age_1",
             &VonBSchnuteGrowthInterface::log_sd_length_at_ref_age_1)
      .field("log_sd_length_at_ref_age_2",
             &VonBSchnuteGrowthInterface::log_sd_length_at_ref_age_2)
      .field("log_sd_growth_coefficient_K",
             &VonBSchnuteGrowthInterface::log_sd_growth_coefficient_K)
      .field("logit_corr_length_at_ref_age_1_length_at_ref_age_2",
             &VonBSchnuteGrowthInterface::
                 logit_corr_length_at_ref_age_1_length_at_ref_age_2)
      .field("logit_corr_length_at_ref_age_1_k",
             &VonBSchnuteGrowthInterface::logit_corr_length_at_ref_age_1_k)
      .field("logit_corr_length_at_ref_age_2_k",
             &VonBSchnuteGrowthInterface::logit_corr_length_at_ref_age_2_k)
      .field("n_ages", &VonBSchnuteGrowthInterface::n_ages)
      .method("get_id", &VonBSchnuteGrowthInterface::get_id)
      .method("evaluate", &VonBSchnuteGrowthInterface::evaluate)
      .method("to_json", &VonBSchnuteGrowthInterface::to_json)
#ifdef TMB_MODEL
      .method("add_to_fims_tmb",
              &VonBSchnuteGrowthInterface::add_to_fims_tmb)
#endif
      ;
}
