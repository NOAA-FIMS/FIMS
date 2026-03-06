#ifndef FIMS_ADAPTERS_AD_MACROS_H
#define FIMS_ADAPTERS_AD_MACROS_H

#pragma once

#ifdef TMB_MODEL__
#include <TMB.hpp>





template <typename Type>
bool is_double(const Type& x) {
  return std::is_same<Type, double>::value;
}

#define ADREPORT_F(name, F) F->reportvector.push(name, #name)

#define FIMS_SIMULATE_F(F) \
    if (is_double<Type>(F->do_simulate))

// define REPORT, ADREPORT, and SIMULATE
#define FIMS_REPORT_F(name, F)                                         \
  if (isDouble<Type>::value &&                                         \
      F->current_parallel_region < static_cast<Type>(0)) {             \
    Rf_defineVar(Rf_install(#name), PROTECT(asSEXP(name)), F->report); \
    UNPROTECT(1);                                                      \
  }

#define FIMS_REPORT_F_(name, obj, F)                                 \
  if (isDouble<Type>::value &&                                       \
      F->current_parallel_region < static_cast<Type>(0)) {           \
    Rf_defineVar(Rf_install(name), PROTECT(asSEXP(obj)), F->report); \
    UNPROTECT(1);                                                    \
  }

#else

#define ADREPORT_F(name, F)

#define FIMS_REPORT_F(name, F) 

#define FIMS_REPORT_F_(name, obj, F) 

#define FIMS_SIMULATE_F(F) \
    if(false)

#endif

#endif
