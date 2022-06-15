/*
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
};

#endif /* TMB_MODEL */

#endif /* FIMS_INTERFACE_HPP */
