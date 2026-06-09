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
#include <Rcpp.h>
#include <R_ext/Rdynload.h>

#ifdef FIMS_ONLOAD_INIT_TMB
/**
 * @brief Function to register functions with the Rcpp module system.
 *
 * This function is called when the Rcpp module is loaded, and it registers
 * all of the TMB C callables.
 */
extern "C" SEXP fims_post_load_init_tmb() {
#ifdef TMB_CCALLABLES
  Rprintf("** Initializing TMB C callables for FIMS...\n");
  TMB_CCALLABLES("FIMS");
#endif
  return R_NilValue;
}

#else

extern "C" {

/**
 * @brief TODO: Handles the initialization of the fims rcpp module.
 *
 * @return SEXP
 */
SEXP _rcpp_module_boot_fims();

/**
 * @brief Callback definition to load the FIMS module.
 */
static const R_CallMethodDef CallEntries[] = {
    TMB_CALLDEFS,
    {"_rcpp_module_boot_fims", (DL_FUNC)&_rcpp_module_boot_fims, 0},
    {NULL, NULL, 0}};

/**
 * @brief FIMS shared object initializer.
 *
 */
__attribute__((visibility("default"))) void R_init_FIMS(DllInfo *dll) {
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
#ifdef TMB_CCALLABLES
  TMB_CCALLABLES("FIMS");
#endif
}
}

#endif  // FIMS_ONLOAD_INIT_TMB

#endif  // SRC_INIT_HPP