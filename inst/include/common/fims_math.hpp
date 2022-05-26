/*
 * File:   fims_math.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */
#ifndef FIMS_MATH_HPP
#define FIMS_MATH_HPP

// note: this is modeling platform specific, must be controlled by
// preprocessing macros
//#include "def.hpp"
#include <cmath>

namespace fims {
#ifdef STD_LIB
/**
 * @brief The exponential function.
 *
 * @param x value to exponentiate. Please use fims::exp<double>(x) if x is an
 * integer.
 * @return the exponentiated value
 */
template <class T>
inline const T exp(const T &x) {
  return std::exp(x);
}

/**
 * @brief The natural log function (base e)
 * @param x the value to take the log of. Please use fims::log<double>(x) if x
 * is an integer.
 * @return
 */
template <class T>
inline const T log(const T &x) {
  return std::log(x);
}
#endif

#ifdef TMB_MODEL
#include <TMB.hpp>

/**
 * @brief The exponential function.
 * The code cannot be tested using the compilation flag
 * -DTMB_MODEL through CMake and Google Test
 * @param x value to exponentiate. Please use fims::exp<double>(x) if x is an
 * integer.
 * @return the exponentiated value
 */
template <class T>
inline const T exp(const T &x) {
  return exp(x);
}

/**
 * @brief The natural log function (base e)
 * The code cannot be tested using the compilation flag
 * -DTMB_MODEL through CMake and Google Test.
 * @param x the value to take the log of. Please use fims::log<double>(x) if x
 * is an integer.
 * @return the log of the value
 */
template <class T>
inline const T log(const T &x) {
  return log(x);
}

#endif

/**
 * @brief The general logistic function
 *
 * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - median))} \f$
 *
 * @param median the median (inflection point) of the logistic function
 * @param slope the slope of the logistic function
 * @param x the index the logistic function should be evaluated at
 * @return
 */
template <class T>
inline const T logistic(const T &median, const T &slope, const T &x) {
  return (1.0) / (1.0 + exp(-1.0 * slope * (x - median)));
}

/**
 * @brief The general double logistic function
 *
 * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope_{1} (x - median_{1}))} \left(1-\frac{1.0}{ 1.0 + exp(-1.0 * slope_{2} (x - median_{2}))}  \right)\f$
 *
 * @param median1 the median (inflection point) of the ascending limb of the double double logistic function
 * @param slope1 the slope of the aascending limb of e double hlogistic function
 * @param median2 the median (inflection point) of the descending limb double descending limb of the double logistic function
 * @param slope2 the slope of the descending limb of the double logistic function
 * @param x the index the logistic function should be evaluated at
 * @return
 */

template <class T>
inline const T double_logistic(const T &median1, const T &slope1, const T &median2, const T &slope2, const T &x) {
  return (1.0) / (1.0 + exp(-1.0 * slope1 * (x - median1))) * (1.0 - (1.0) / (1.0 + exp(-1.0 * slope2 * (x - median2))));
}

}  // namespace fims

#endif /* FIMS_MATH_HPP */
