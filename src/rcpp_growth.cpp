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

  Rcpp::class_<VonBertalanffySchnuteGrowthInterface>("VonBertalanffySchnuteGrowth")
      .constructor()
      .field("mean_length_young",
             &VonBertalanffySchnuteGrowthInterface::mean_length_young)
      .field("mean_length_old",
             &VonBertalanffySchnuteGrowthInterface::mean_length_old)
      .field("growth_coefficient",
             &VonBertalanffySchnuteGrowthInterface::growth_coefficient)
      .field("reference_age_for_length_1",
             &VonBertalanffySchnuteGrowthInterface::reference_age_for_length_1)
      .field("reference_age_for_length_2",
             &VonBertalanffySchnuteGrowthInterface::reference_age_for_length_2)
      .field("length_weight_a",
             &VonBertalanffySchnuteGrowthInterface::length_weight_a)
      .field("length_weight_b",
             &VonBertalanffySchnuteGrowthInterface::length_weight_b)
      .field("length_at_age_sd_at_ref_ages",
             &VonBertalanffySchnuteGrowthInterface::length_at_age_sd_at_ref_ages)
      .field("log_sd_length_at_ref_age_1",
             &VonBertalanffySchnuteGrowthInterface::log_sd_length_at_ref_age_1)
      .field("log_sd_length_at_ref_age_2",
             &VonBertalanffySchnuteGrowthInterface::log_sd_length_at_ref_age_2)
      .field("log_sd_growth_coefficient",
             &VonBertalanffySchnuteGrowthInterface::log_sd_growth_coefficient)
      .field("logit_corr_length_at_ref_age_1_length_at_ref_age_2",
             &VonBertalanffySchnuteGrowthInterface::
                 logit_corr_length_at_ref_age_1_length_at_ref_age_2)
      .field("logit_corr_length_at_ref_age_1_growth_coefficient",
             &VonBertalanffySchnuteGrowthInterface::logit_corr_length_at_ref_age_1_growth_coefficient)
      .field("logit_corr_length_at_ref_age_2_growth_coefficient",
             &VonBertalanffySchnuteGrowthInterface::logit_corr_length_at_ref_age_2_growth_coefficient)
      .field("n_ages", &VonBertalanffySchnuteGrowthInterface::n_ages)
      .method("get_id", &VonBertalanffySchnuteGrowthInterface::get_id)
      .method("evaluate", &VonBertalanffySchnuteGrowthInterface::evaluate)
      .method("to_json", &VonBertalanffySchnuteGrowthInterface::to_json)
#ifdef TMB_MODEL
      .method("add_to_fims_tmb",
              &VonBertalanffySchnuteGrowthInterface::add_to_fims_tmb)
#endif
      ;
}
