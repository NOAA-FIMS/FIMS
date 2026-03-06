#ifndef FIMS_ADAPTERS_AD_MACROS_H
#define FIMS_ADAPTERS_AD_MACROS_H

#pragma once

#ifdef TMB_MODEL
#include <TMB.hpp>

#define ADREPORT_F(name, F) F->reportvector.push(name, #name)

#define FIMS_SIMULATE_F(F) \
    if (isDouble<Type>::value && F->do_simulate)

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

#define FIMS_SIMULATE_F(F) \
    if(false)

#endif

#endif
