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

#include "TMB/TMB.h"

#ifdef QUADRA_MODEL
#include "Quadra/math/special_functions.hpp"

namespace had {
inline bool operator!=(const AReal& lhs, const AReal& rhs) {
  return !(lhs == rhs);
}
inline double asDouble(const AReal& x) { return x.val; }
}  // namespace had

// Compatibility overloads keep TMB's globally visible distribution helpers
// from capturing Quadra scalar calls when both backends are in one build.
inline had::AReal dnorm(const had::AReal& x, const had::AReal& mean,
                       const had::AReal& sd, int give_log = 0) {
  const had::AReal z = (x - mean) / sd;
  const had::AReal log_density =
      -0.5 * z * z - log(sd) - 0.5 * std::log(2.0 * M_PI);
  return give_log ? log_density : exp(log_density);
}

inline had::AReal dmultinom(const vector<had::AReal>& x,
                           const vector<had::AReal>& p,
                           int give_log = 0) {
  had::AReal total = 0.0;
  had::AReal log_probability = 0.0;
  for (int i = 0; i < x.size(); ++i) {
    total += x[i];
    log_probability -= ::lgamma(x[i] + 1.0);
    log_probability += x[i] * log(p[i]);
  }
  log_probability += ::lgamma(total + 1.0);
  return give_log ? log_probability : exp(log_probability);
}
#endif

// define REPORT, ADREPORT, and SIMULATE
#define FIMS_REPORT_F(name, F)                                         \
  if constexpr (isDouble<Type>::value)                                 \
  if (                                                               \
      F->current_parallel_region < static_cast<Type>(0)) {             \
    Rf_defineVar(Rf_install(#name), PROTECT(asSEXP(name)), F->report); \
    UNPROTECT(1);                                                      \
  }

#define FIMS_REPORT_F_(name, obj, F)                                 \
  if constexpr (isDouble<Type>::value)                               \
  if (                                                              \
      F->current_parallel_region < static_cast<Type>(0)) {           \
    Rf_defineVar(Rf_install(name), PROTECT(asSEXP(obj)), F->report); \
    UNPROTECT(1);                                                    \
  }

#ifdef QUADRA_MODEL
#define ADREPORT_F(name, F)                          \
  if constexpr (!std::is_same_v<Type, had::AReal>) { \
    F->reportvector.push(name, #name);               \
  }
#else
#define ADREPORT_F(name, F) F->reportvector.push(name, #name);
#endif

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

#define FIMS_SIMULATE_F(F) if constexpr (isDouble<Type>::value) if (F->do_simulate)

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
