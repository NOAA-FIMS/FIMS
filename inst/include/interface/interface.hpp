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
#include "../common/def.hpp"

#ifdef TMB_MODEL
// use isnan macro in math.h instead of TMB's isnan for fixing the r-cmd-check
// issue
#include <math.h>

//#define TMB_LIB_INIT R_init_FIMS
#include <TMB.hpp>
#include "../common/def.hpp"

//define REPORT, ADREPORT, and SIMULATE
#define REPORT_F(name,F)     \
  if(isDouble<Type>::value && F->current_parallel_region<0) { \
    Rf_defineVar(Rf_install(#name),     \
              PROTECT(asSEXP(name)),F->report);   \
    UNPROTECT(1);      \
  }
#define ADREPORT_F(name,F) F->reportvector.push(name,#name);

#define SIMULATE_F(F)    \
  if(isDouble<Type>::value && F->do_simulate)

#endif

/**
 * @brief Default trait. These are "T" specific
 * traits that depend on modeling platform.
 */
template <typename T>
struct ModelTraits {
    typedef typename std::vector<T> DataVector; /**< This is a vector
        of the data that is differentiable */
    typedef typename std::vector<T> ParameterVector; /**< This is a*/

    static ParameterVector MakeVariableVector(const std::vector<T>& v) {
        ParameterVector ret(v.size());

        for (int i = 0; i < v.size(); i++) {
            ret[i] = v[i];
        }
        return ret;
    }

};
#ifdef TMB_MODEL

template <>
struct ModelTraits<TMB_FIMS_FIRST_ORDER> {
    typedef double real_t; /**< The real type */
    typedef TMB_FIMS_SECOND_ORDER variable_t; /**< The variable type */
    typedef typename CppAd::vector<double> DataVector; /**< The data vector type */
    typedef typename CppAd::vector<TMB_FIMS_FIRST_ORDER> ParameterVector; /**< The variable vector*/

    static ParameterVector MakeVariableVector(const std::vector<TMB_FIMS_FIRST_ORDER>& v) {
        ParameterVector ret(v.size());

        for (int i = 0; i < v.size(); i++) {
            ret[i] = v[i];
        }
        return ret;
    }
};

template <>
struct ModelTraits<TMB_FIMS_SECOND_ORDER> {
    typedef double real_t; /**< The real type */
    typedef TMB_FIMS_SECOND_ORDER variable_t; /**< The variable type */
    typedef typename CppAd::vector<double> DataVector; /**< The data vector type */
    typedef typename CppAd::vector<double> ParameterVector; /**< The variable vector*/

    static ParameterVector MakeVariableVector(const std::vector<TMB_FIMS_SECOND_ORDER>& v) {
        ParameterVector ret(v.size());

        for (int i = 0; i < v.size(); i++) {
            ret[i] = v[i];
        }
        return ret;
    }
};

template <>
struct ModelTraits<TMB_FIMS_THIRD_ORDER> {
    typedef double real_t; /**< The real type */
    typedef TMB_FIMS_THIRD_ORDER variable_t; /**< The variable type */
    typedef typename CppAd::vector<double> DataVector; /**< The data vector type */
    typedef typename CppAd::vector<TMB_FIMS_THIRD_ORDER> ParameterVector; /**< The variable vector
  

    static ParameterVector MakeVariableVector(const std::vector<TMB_FIMS_THIRD_ORDER>& v) {
        ParameterVector ret(v.size());

        for (int i = 0; i < v.size(); i++) {
            ret[i] = v[i];
        }
        return ret;
    }
};

/**
 *  @brief ModelTraits class that contains the DataVector
 * and ParameterVector types.
 */
//template <typename T>
//struct ModelTraits {
//    typedef typename CppAD::vector<T> DataVector; /**< This is a vector
//        of the data that is differentiable */
//    typedef typename CppAD::vector<T> ParameterVector; /**< This is a
//  vector of the parameters that is differentiable */
//    typedef typename tmbutils::vector<T>
//    EigenVector; /**< This is a vector as defined in TMB's namespace Eigen */
//};

#endif /* TMB_MODEL */

#endif /* FIMS_INTERFACE_HPP */
