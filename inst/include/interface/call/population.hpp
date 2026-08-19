/**
 * @file population.hpp
 * @brief Native .Call entry points for population helpers.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_POPULATION_HPP
#define FIMS_INTERFACE_CALL_POPULATION_HPP

typedef struct SEXPREC *SEXP;

extern "C" SEXP fims_call_create_population(SEXP log_m_sexp,
                                            SEXP log_f_multiplier_sexp,
                                            SEXP log_init_naa_sexp,
                                            SEXP log_m_estimation_type_sexp,
                                            SEXP log_f_multiplier_estimation_type_sexp,
                                            SEXP log_init_naa_estimation_type_sexp,
                                            SEXP maturity_id_sexp,
                                            SEXP growth_id_sexp,
                                            SEXP recruitment_id_sexp,
                                            SEXP fleet_ids_sexp);

extern "C" SEXP fims_call_population_prepare(SEXP population_id_sexp);

#endif // FIMS_INTERFACE_CALL_POPULATION_HPP