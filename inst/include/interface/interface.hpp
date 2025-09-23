/**
 * @file interface.hpp
 * @brief An interface to the modeling platforms, e.g., TMB.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */

#ifndef FIMS_INTERFACE_HPP
#define FIMS_INTERFACE_HPP

/*
 * @brief Interface file. Uses pre-processing macros
 * to interface with multiple modeling platforms.
 */

// traits for interfacing with TMB

#ifdef TMB_MODEL
// use isnan macro in math.h instead of TMB's isnan for fixing the r-cmd-check
// issue
#include <math.h>

#include <TMB.hpp>

// define REPORT, ADREPORT, and SIMULATE
#define FIMS_REPORT_F(name, F)                                         \
  if (isDouble<Type>::value &&                                         \
      F->current_parallel_region < static_cast<Type>(0)) {             \
    Rf_defineVar(Rf_install(#name), PROTECT(asSEXP(name)), F->report); \
    UNPROTECT(1);                                                      \
  }

#define FIMS_REPORT_F_(name, obj, F)                                         \
  if (isDouble<Type>::value &&                                         \
      F->current_parallel_region < static_cast<Type>(0)) {             \
    Rf_defineVar(Rf_install(name), PROTECT(asSEXP(obj)), F->report); \
    UNPROTECT(1);                                                      \
  }

#define ADREPORT_F(name, F) F->reportvector.push(name, #name);

template <typename Type>
vector<Type> ADREPORTvector(vector<vector<Type> > x) {
  int outer_dim = x.size();
  int dim = 0;
  for (int i = 0; i < outer_dim; i++) {
    dim += x(i).size();
  }
  vector<Type> res(dim);
  int idx = 0;
  for (int i = 0; i < outer_dim; i++) {
    int inner_dim = x(i).size();
    for (int j = 0; j < inner_dim; j++) {
      res(idx) = x(i)(j);
      idx += 1;
    }
  }
  return res;
}

#define FIMS_SIMULATE_F(F) if (isDouble<Type>::value && F->do_simulate)

#endif /* TMB_MODEL */

#ifndef TMB_MODEL
/**
 * @brief TMB macro that simulates data.
 */
#define FIMS_SIMULATE_F(F)
/**
 * @brief TMB macro that reports variables.
 */
#define FIMS_REPORT_F(name, F)
/**
 * @brief TMB macro that reports variables and uncertainties.
 */
#define ADREPORT_F(name, F)
#endif

#endif /* FIMS_INTERFACE_HPP */
