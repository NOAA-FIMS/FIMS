/**
 * @file init_tmb.hpp
 * @brief An interface to dynamically load the functions.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef SRC_INIT_HPP
#define SRC_INIT_HPP

#include <stdlib.h>
#ifndef R_NO_REMAP
#define R_NO_REMAP
#endif
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include "../call/fleet.hpp"
#include "../call/growth.hpp"
#include "../call/maturity.hpp"
#include "../call/model.hpp"
#include "../call/population.hpp"
#include "../call/recruitment.hpp"
#include "../call/selectivity.hpp"

#ifdef FIMS_ONLOAD_INIT_TMB
/**
 * @brief Function to initialize the TMB C callables.
 *
 * This function is called after the shared library is loaded.
 */
extern "C" SEXP fims_post_load_init_tmb()
{
#ifdef TMB_CCALLABLES
  Rprintf("** Initializing TMB C callables for FIMS...\n");
  TMB_CCALLABLES("FIMS");
#endif
  return R_NilValue;
}

#else

extern "C"
{

  /**
   * @brief Callback definition to load the FIMS module.
   */
  static const R_CallMethodDef CallEntries[] = {
      {"fims_call_create_logistic_selectivity",
       (DL_FUNC)&fims_call_create_logistic_selectivity, 4},
      {"fims_call_logistic_selectivity",
       (DL_FUNC)&fims_call_logistic_selectivity, 2},
      {"fims_call_create_double_logistic_selectivity",
       (DL_FUNC)&fims_call_create_double_logistic_selectivity, 8},
      {"fims_call_double_logistic_selectivity",
       (DL_FUNC)&fims_call_double_logistic_selectivity, 2},
      {"fims_call_information_parameter_counts",
       (DL_FUNC)&fims_call_information_parameter_counts, 0},
      {"fims_call_create_beverton_holt_recruitment",
       (DL_FUNC)&fims_call_create_beverton_holt_recruitment, 6},
      {"fims_call_beverton_holt_evaluate_mean",
       (DL_FUNC)&fims_call_beverton_holt_evaluate_mean, 3},
      {"fims_call_create_logistic_maturity",
       (DL_FUNC)&fims_call_create_logistic_maturity, 4},
      {"fims_call_logistic_maturity",
       (DL_FUNC)&fims_call_logistic_maturity, 2},
      {"fims_call_create_ewaa_growth", (DL_FUNC)&fims_call_create_ewaa_growth,
       4},
      {"fims_call_ewaa_growth_evaluate",
       (DL_FUNC)&fims_call_ewaa_growth_evaluate, 3},
      {"fims_call_create_fleet", (DL_FUNC)&fims_call_create_fleet, 6},
      {"fims_call_fleet_prepare", (DL_FUNC)&fims_call_fleet_prepare, 1},
      {"fims_call_create_population", (DL_FUNC)&fims_call_create_population,
       10},
      {"fims_call_population_prepare", (DL_FUNC)&fims_call_population_prepare,
       1},
      {"fims_call_create_model", (DL_FUNC)&fims_call_create_model, 0},
      {"fims_call_build_default_likelihood",
       (DL_FUNC)&fims_call_build_default_likelihood, 15},
      {"fims_call_add_prior", (DL_FUNC)&fims_call_add_prior, 8},
      {"fims_call_information_model_counts",
       (DL_FUNC)&fims_call_information_model_counts, 0},
      {"fims_call_information_clear", (DL_FUNC)&fims_call_information_clear,
       0},
      {"fims_call_information_get_fixed",
       (DL_FUNC)&fims_call_information_get_fixed, 0},
      {"fims_call_information_get_random",
       (DL_FUNC)&fims_call_information_get_random, 0},
      {"fims_call_information_get_parameter_names",
       (DL_FUNC)&fims_call_information_get_parameter_names, 0},
      TMB_CALLDEFS,
      {NULL, NULL, 0}};

  /**
   * @brief FIMS shared object initializer.
   *
   */
  __attribute__((visibility("default"))) void R_init_FIMS(DllInfo *dll)
  {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
#ifdef TMB_CCALLABLES
    TMB_CCALLABLES("FIMS");
#endif
  }
}

#endif // FIMS_ONLOAD_INIT_TMB

#endif // SRC_INIT_HPP
