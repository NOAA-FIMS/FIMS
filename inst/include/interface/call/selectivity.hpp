/**
 * @file selectivity.hpp
 * @brief Native .Call entry points for selectivity helpers.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_SELECTIVITY_HPP
#define FIMS_INTERFACE_CALL_SELECTIVITY_HPP

typedef struct SEXPREC *SEXP;

extern "C" SEXP fims_call_create_logistic_selectivity(SEXP inflection_point_sexp,
                                                      SEXP slope_sexp,
                                                      SEXP inflection_point_estimation_type_sexp,
                                                      SEXP slope_estimation_type_sexp);

extern "C" SEXP fims_call_logistic_selectivity(SEXP x_sexp,
                                               SEXP selectivity_id_sexp);

extern "C" SEXP fims_call_create_double_logistic_selectivity(
    SEXP inflection_point_asc_sexp, SEXP slope_asc_sexp,
    SEXP inflection_point_desc_sexp, SEXP slope_desc_sexp,
    SEXP inflection_point_asc_estimation_type_sexp,
    SEXP slope_asc_estimation_type_sexp,
    SEXP inflection_point_desc_estimation_type_sexp,
    SEXP slope_desc_estimation_type_sexp);

extern "C" SEXP fims_call_double_logistic_selectivity(
    SEXP x_sexp, SEXP selectivity_id_sexp);

extern "C" SEXP fims_call_information_parameter_counts();

#endif // FIMS_INTERFACE_CALL_SELECTIVITY_HPP