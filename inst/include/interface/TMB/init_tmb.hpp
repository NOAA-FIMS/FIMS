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

#ifdef __cplusplus
extern "C" {
#endif

// Your global pointer
DllInfo *g_dll = NULL;

// /**
//  * @brief TODO: Handles the initialization of the fims rcpp module.
//  *
//  * @return SEXP
//  */
 SEXP _rcpp_module_boot_fims();

// /**
//  * @brief Callback definition to load the FIMS module.
//  */
static const R_CallMethodDef CallEntries[] = {
    {"_rcpp_module_boot_fims", (DL_FUNC) &_rcpp_module_boot_fims, 0},
    // {"R_init_FIMS_internal", (DL_FUNC) &R_init_FIMS_internal, 1},
    TMB_CALLDEFS,
    {NULL, NULL, 0}};

/**
 * @brief FIMS shared object initializer.
 * @param dll TODO: provide a brief description.
 *
 */
void R_init_FIMS(DllInfo *dll) {
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
#ifdef TMB_CCALLABLES
  TMB_CCALLABLES("FIMS");
#endif
}

#ifdef __cplusplus
}
#endif

#endif  // SRC_INIT_HPP