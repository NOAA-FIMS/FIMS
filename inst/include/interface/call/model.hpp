/**
 * @file model.hpp
 * @brief Native .Call entry point for backend model construction.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_MODEL_HPP
#define FIMS_INTERFACE_CALL_MODEL_HPP

typedef struct SEXPREC* SEXP;

extern "C" SEXP fims_call_create_model();
extern "C" SEXP fims_call_build_default_likelihood(
    SEXP fishing_fleet_id_sexp, SEXP survey_fleet_id_sexp, SEXP landings_sexp,
    SEXP landings_distribution_sexp, SEXP landings_sd_sexp,
    SEXP landings_age_comp_sexp, SEXP landings_length_comp_sexp,
    SEXP survey_index_sexp, SEXP survey_distribution_sexp, SEXP survey_sd_sexp,
    SEXP survey_age_comp_sexp, SEXP survey_length_comp_sexp,
    SEXP recruitment_log_sd_sexp, SEXP recruitment_log_sd_estimation_type_sexp,
    SEXP n_years_sexp, SEXP n_ages_sexp, SEXP n_lengths_sexp);
extern "C" SEXP fims_call_add_prior(SEXP module_sexp, SEXP object_id_sexp,
                                    SEXP parameter_sexp, SEXP distribution_sexp,
                                    SEXP mean_sexp, SEXP log_sd_sexp,
                                    SEXP mean_estimation_type_sexp,
                                    SEXP log_sd_estimation_type_sexp);
extern "C" SEXP fims_call_information_model_counts();
extern "C" SEXP fims_call_information_clear();
extern "C" SEXP fims_call_information_get_fixed();
extern "C" SEXP fims_call_information_get_random();
extern "C" SEXP fims_call_information_get_parameter_names();
extern "C" SEXP fims_call_information_get_random_effect_names();

#endif  // FIMS_INTERFACE_CALL_MODEL_HPP
