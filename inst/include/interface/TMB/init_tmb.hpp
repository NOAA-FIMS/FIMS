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
#include <R_ext/Rdynload.h>

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
    {NULL, NULL, 0}};

    /**
     * @brief The function that R calls when the package is loaded. 
     * It registers the TMB C callables for the FIMS module, 
     * allowing R to call C++ functions defined in the TMB library.
     *   */
extern "C" SEXP fims_post_load_init_tmb() {
#ifdef TMB_CCALLABLES
Rprintf("** Initializing TMB C callables for FIMS...\n");
  TMB_CCALLABLES("FIMS");
#endif
  return R_NilValue;
}

}

#endif  // SRC_INIT_HPP