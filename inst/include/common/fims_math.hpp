/*! \file fims_math.hpp
 */
// note: To document a global C function, typedef, enum or preprocessor
// definition you must first document the file that contains it

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

#include "../interface/interface.hpp"

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
// #include <TMB.hpp>

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

template <>
inline const double exp(const double &x) {
  return std::exp(x);
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

template <>
inline const double log(const double &x) {
  return std::log(x);
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
 * @brief A logit function for bounding of parameters
 *
 * \f$ -\mathrm{log}(b-x) + \mathrm{log}(x-a) \f$
 * @param a lower bound
 * @param b upper bound
 * @param x the parameter in bounded space
 * @return the parameter in real space
 *
 */
template <class T>
inline const T logit(const T &a, const T &b, const T &x) {
  return -fims::log(b - x) + fims::log(x - a);
}

/**
 * @brief An inverse logit function for bounding of parameters
 *
 * \f$ a+\frac{b-a}{1+\mathrm{exp}(-\mathrm{logit}(x))}\f$
 * @param a lower bound
 * @param b upper bound
 * @param logit_x the parameter in real space
 * @return the parameter in bounded space
 *
 */
template <class T>
inline const T inv_logit(const T &a, const T &b, const T &logit_x) {
  return a + (b - a) / (1 + fims::exp(-logit_x));
}

/**
 * @brief The general double logistic function
 *
 * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope_{asc} (x - median_{asc}))}
 * \left(1-\frac{1.0}{ 1.0 + exp(-1.0 * slope_{desc} (x - median_{desc}))}
 * \right)\f$
 *
 * @param median_asc the median (inflection point) of the ascending limb of the
 * double logistic function
 * @param slope_asc the slope of the ascending limb of the double logistic
 * function
 * @param median_desc the median (inflection point) of the descending limb of
 * the double logistic function, where median_desc > median_asc
 * @param slope_desc the slope of the descending limb of the double logistic
 * function
 * @param x the index the logistic function should be evaluated at
 * @return
 */

template <class T>
inline const T double_logistic(const T &median_asc, const T &slope_asc,
                               const T &median_desc, const T &slope_desc,
                               const T &x) {
  return (1.0) / (1.0 + exp(-1.0 * slope_asc * (x - median_asc))) *
         (1.0 - (1.0) / (1.0 + exp(-1.0 * slope_desc * (x - median_desc))));
}

/**
 *
 * Used when x could evaluate to zero, which will result in a NaN for
 * derivative values.
 *
 * Evaluates:
 *
 * \f$ (x^2+C)^.5 \f$
 *
 * @param x value to keep positive
 * @param C default = 1e-5
 * @return
 */
template <class T>
const T ad_fabs(const T &x, T C = 1e-5) {
  return sqrt((x * x) + C);  //, .5);
}

/**
 *
 * Returns the minimum between a and b in a continuous manner using:
 *
 * (a + b - fims::ad_fabs(a - b))*.5;
 * Reference: \ref fims::ad_fabs()
 *
 * This is an approximation with minimal error.
 *
 * @param a
 * @param b
 * @param C default = 1e-5
 * @return
 */
template <typename T>
inline const T ad_min(const T &a, const T &b, T C = 1e-5) {
  return (a + b - fims::ad_fabs(a - b, C)) * .5;
}

/**
 * Returns the maximum between a and b in a continuous manner using:
 *
 * (a + b + fims::ad_fabs(a - b)) *.5;
 * Reference: \ref fims::ad_fabs()
 * This is an approximation with minimal error.
 *
 * @param a
 * @param b
 * @param C default = 1e-5
 * @return
 */
template <typename T>
inline const T ad_max(const T &a, const T &b, T C = 1e-5) {
  return (a + b + fims::ad_fabs(a - b, C)) * static_cast<T>(.5);
}

}  // namespace fims

#endif /* FIMS_MATH_HPP */
