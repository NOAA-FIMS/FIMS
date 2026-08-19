/**
 * @file maturity.hpp
 * @brief Native .Call entry points for maturity helpers.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_MATURITY_HPP
#define FIMS_INTERFACE_CALL_MATURITY_HPP

typedef struct SEXPREC *SEXP;

extern "C" SEXP fims_call_create_logistic_maturity(SEXP inflection_point_sexp,
                                                   SEXP slope_sexp,
                                                   SEXP inflection_point_estimation_type_sexp,
                                                   SEXP slope_estimation_type_sexp);

extern "C" SEXP fims_call_logistic_maturity(SEXP x_sexp, SEXP maturity_id_sexp);

#endif // FIMS_INTERFACE_CALL_MATURITY_HPP