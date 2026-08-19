/**
 * @file growth.hpp
 * @brief Native .Call entry points for growth helpers.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_GROWTH_HPP
#define FIMS_INTERFACE_CALL_GROWTH_HPP

typedef struct SEXPREC *SEXP;

extern "C" SEXP fims_call_create_ewaa_growth(SEXP ages_sexp, SEXP weights_sexp,
                                             SEXP n_years_sexp,
                                             SEXP weights_estimation_type_sexp);

extern "C" SEXP fims_call_ewaa_growth_evaluate(SEXP year_sexp, SEXP age_sexp,
                                               SEXP growth_id_sexp);

#endif // FIMS_INTERFACE_CALL_GROWTH_HPP