/**
 * @file fleet.hpp
 * @brief Native .Call entry points for fleet helpers.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_FLEET_HPP
#define FIMS_INTERFACE_CALL_FLEET_HPP

typedef struct SEXPREC *SEXP;

extern "C" SEXP fims_call_create_fleet(SEXP log_fmort_sexp, SEXP log_q_sexp,
                                       SEXP selectivity_id_sexp,
                                       SEXP age_to_length_conversion_sexp,
                                       SEXP log_fmort_estimation_type_sexp,
                                       SEXP log_q_estimation_type_sexp);

extern "C" SEXP fims_call_fleet_prepare(SEXP fleet_id_sexp);

#endif // FIMS_INTERFACE_CALL_FLEET_HPP