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
#include "../population_dynamics/growth/growth.hpp"

#ifdef TMB_MODEL
// use isnan macro in math.h instead of TMB's isnan for fixing the r-cmd-check
// issue
#include <math.h>
//#define TMB_LIB_INIT R_init_FIMS
#include <TMB.hpp>

/**
 *  @brief ModelTraits class that contains the DataVector
 * and ParameterVector types.
 */
template <typename T>
struct ModelTraits {
  typedef typename CppAD::vector<T> DataVector;      /**< This is a vector
        of the data that is differentiable */
  typedef typename CppAD::vector<T> ParameterVector; /**< This is a
  vector of the parameters that is differentiable */
  typedef typename tmbutils::vector<T>
      EigenVector; /**< This is a vector as defined in TMB's namespace Eigen */
};

// define REPORT, ADREPORT, and SIMULATE
#define REPORT_F(name, F)                                              \
  if (isDouble<T>::value && F->current_parallel_region < 0) {          \
    Rf_defineVar(Rf_install(#name), PROTECT(asSEXP(name)), F->report); \
    UNPROTECT(1);                                                      \
  }

#endif /* TMB_MODEL */

#endif /* FIMS_INTERFACE_HPP */
