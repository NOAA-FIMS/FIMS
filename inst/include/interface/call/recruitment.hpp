/**
 * @file recruitment.hpp
 * @brief Native .Call entry points for recruitment helpers.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_RECRUITMENT_HPP
#define FIMS_INTERFACE_CALL_RECRUITMENT_HPP

typedef struct SEXPREC *SEXP;

extern "C" SEXP fims_call_create_beverton_holt_recruitment(
    SEXP logit_steep_sexp,
    SEXP log_rzero_sexp,
    SEXP log_devs_sexp,
    SEXP logit_steep_estimation_type_sexp,
    SEXP log_rzero_estimation_type_sexp,
    SEXP log_devs_estimation_type_sexp);

extern "C" SEXP fims_call_beverton_holt_evaluate_mean(
    SEXP spawners_sexp, SEXP phi_0_sexp, SEXP recruitment_id_sexp);

#endif // FIMS_INTERFACE_CALL_RECRUITMENT_HPP