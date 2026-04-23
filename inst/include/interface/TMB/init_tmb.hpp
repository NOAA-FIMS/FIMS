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


extern "C" SEXP fims_post_load_init_tmb() {
#ifdef TMB_CCALLABLES
  Rprintf("** Initializing TMB C callables for FIMS...\n");
  TMB_CCALLABLES("FIMS");
#endif
  return R_NilValue;
}

#endif  // SRC_INIT_HPP