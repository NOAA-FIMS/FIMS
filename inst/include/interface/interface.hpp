/*
 * File: interface.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 */

#ifndef FIMS_INTERFACE_HPP
#define FIMS_INTERFACE_HPP
/*
 * Interface file. Uses pre-processing macros
 * to interface with multiple modeling platforms.
 */

// traits for interfacing with TMB

#ifdef TMB_MODEL
// use isnan macro in math.h instead of TMB's isnan for fixing the r-cmd-check
// issue
#include <math.h>

#include <TMB.hpp>

// define REPORT, ADREPORT, and SIMULATE
#define REPORT_F(name, F)                                              \
  if (isDouble<Type>::value && F->current_parallel_region < 0) {       \
    Rf_defineVar(Rf_install(#name), PROTECT(asSEXP(name)), F->report); \
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
 #define FIMS_SIMULATE_F(F)
#endif

#endif /* FIMS_INTERFACE_HPP */
