/**
 * @file fims_math.hpp
 * @brief A collection of mathematical functions used in FIMS.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_MATH_HPP
#define FIMS_MATH_HPP

// note: this is modeling platform specific, must be controlled by
// preprocessing macros
#include <cmath>
#include <random>
#include <sstream>

#include "../interface/interface.hpp"
#include "fims_vector.hpp"

namespace fims_math {
#ifdef STD_LIB

/**
 * @brief The exponential function.
 *
 * @param x value to exponentiate. Please use fims_math::exp<double>(x) if x is
 * an integer.
 * @return the exponentiated value
 */
template <class Type>
inline const Type exp(const Type &x) {
  return std::exp(x);
}

/**
 * @brief The natural log function (base e)
 * @param x the value to take the log of. Please use fims_math::log<double>(x)
 * if x is an integer.
 * @return
 */
template <class Type>
inline const Type log(const Type &x) {
  return std::log(x);
}

template <class Type>
inline const Type cos(const Type &x) {
  return std::cos(x);
}

template <class Type>
inline const Type sqrt(const Type &x) {
  return std::sqrt(x);
}

template <class Type>
inline const Type pow(const Type &x, const Type &y) {
  return std::pow(x, y);
}

template <class Type>
inline const Type lgamma(const Type &x) {
  return std::lgamma(x);
}
#endif

#ifdef TMB_MODEL

// Add the following line to CMakeLists.txt to enable documentation of TMB_MODEL
// in doxygen or none of the following is rendered.
// set(DOXYGEN_PREDEFINED "TMB_MODEL=1" "ENABLE_TMB_CODE")

/**
 * @brief The exponential function for a TMB model. The function specifically
 * uses std::exp, defined in cmath header, instead of ::exp because the
 * standard library function works with TMBad library, which is designed to
 * recognize and apply its automatic differentiation capabilities to functions
 * from the standard library. Also note that this function cannot be tested
 * using the compilation flag -DTMB_MODEL through CMake and Google Test.
 * @param x The value to exponentiate. Please use fims_math::exp<double>(x) if
 * x is an integer.
 * @return The exponentiated value of x.
 */
template <class Type>
inline const Type exp(const Type &x) {
  // use std::exp for double type, look for TMB version of exp if AD type
  using std::exp;
  return exp(x);
}

/**
 * @brief The natural log function (base e) for a TMB model. The function
 * specifically uses std::log, defined in cmath header, instead of ::log
 * because the standard library function works with TMBad library, which is
 * designed to recognize and apply its automatic differentiation capabilities
 * to functions from the standard library. Also note that this function cannot
 * be tested using the compilation flag -DTMB_MODEL through CMake and Google
 * Test.
 * @param x The value to log. Please use fims_math::log<double>(x) if x is an
 * integer.
 * @return The natural log of x.
 */
template <class Type>
inline const Type log(const Type &x) {
  // use std::log for double type, look for TMB version of log if AD type
  using std::log;
  return log(x);
}

/**
 * @brief The cosine of an angle function for a TMB model. The function
 * specifically uses std::cos, defined in cmath header, instead of ::cos
 * because the standard library function works with TMBad library, which is
 * designed to recognize and apply its automatic differentiation capabilities
 * to functions from the standard library. Also note that this function cannot
 * be tested using the compilation flag -DTMB_MODEL through CMake and Google
 * Test.
 * @param x The value to take the cosine of. Please use
 * fims_math::cos<double>(x) if x is an integer.
 * @return The cosine of the angle x.
 */
template <class Type>
inline const Type cos(const Type &x) {
  // use std::cos for double type, look for TMB version of cos if AD type
  using std::cos;
  return cos(x);
}

/**
 * @brief The square root function for a TMB model. The function specifically
 * uses std::sqrt, defined in cmath header, instead of ::sqrt because the
 * standard library function works with TMBad library, which is designed to
 * recognize and apply its automatic differentiation capabilities to functions
 * from the standard library. Also note that this function cannot be tested
 * using the compilation flag -DTMB_MODEL through CMake and Google Test.
 * @param x The value to take the square root of. Please use
 * fims_math::sqrt<double>(x) if x is an integer.
 * @return The square root of x.
 */
template <class Type>
inline const Type sqrt(const Type &x) {
  // use std::std for double type, look for TMB version of std if AD type
  using std::sqrt;
  return sqrt(x);
}

/**
 * @brief The power function for a TMB model. The function specifically uses
 * std::pow, defined in cmath header, instead of ::pow because the standard
 * library function works with TMBad library, which is designed to recognize
 * and apply its automatic differentiation capabilities to functions from the
 * standard library. Also note that this function cannot be tested using the
 * compilation flag -DTMB_MODEL through CMake and Google Test.
 * @param x The value to take the power of. Please use
 * fims_math::pow<double>(x) if x is an integer.
 * @param y The exponent to raise x to.
 * @return The power of x.
 */
template <class Type>
inline const Type pow(const Type &x, const Type &y) {
  // use std::pow for double type, look for TMB version of pow if AD type
  using std::pow;
  return pow(x, y);
}

/**
 * @brief Computes the natural logarithm of the absolute value of the [gamma
 * function](https://en.wikipedia.org/wiki/Gamma_function) of x for a TMB
 * model. The function specifically uses std::lgamma, defined in cmath header,
 * instead of ::lgamma because the standard library function works with TMBad
 * library, which is designed to recognize and apply its automatic
 * differentiation capabilities to functions from the standard library. Also
 * note that this function cannot be tested using the compilation flag
 * -DTMB_MODEL through CMake and Google Test.
 * @param x The value to take the natural logarithm of the absolute value of
 * the gamma function of. Please use fims_math::lgamma<double>(x) if x is an
 * integer.
 * @return The natural logarithm of the absolute value of the gamma function of
 * x.
 */
template <class Type>
inline const Type lgamma(const Type &x) {
  // use std::lgamma for double type, look for TMB version of lgamma if AD type
  using std::lgamma;
  return lgamma(x);
}

#endif

/**
 * @brief The general logistic function
 *
 * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - inflection_point))} \f$
 *
 * @param inflection_point the inflection point of the logistic function
 * @param slope the slope of the logistic function
 * @param x the index the logistic function should be evaluated at
 * @return
 */
template <class Type>
inline const Type logistic(const Type &inflection_point, const Type &slope,
                           const Type &x) {
  return static_cast<Type>(1.0) /
         (static_cast<Type>(1.0) +
          exp(Type(-1.0) * slope * (x - inflection_point)));
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
template <class Type>
inline const Type logit(const Type &a, const Type &b, const Type &x) {
  return -fims_math::log(b - x) + fims_math::log(x - a);
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
template <class Type>
inline const Type inv_logit(const Type &a, const Type &b, const Type &logit_x) {
  return a + (b - a) / (static_cast<Type>(1.0) + fims_math::exp(-logit_x));
}

/**
 * @brief The general double logistic function
 *
 * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope_{asc} (x - inflection_point_{asc}))}
 * \left(1-\frac{1.0}{ 1.0 + exp(-1.0 * slope_{desc} (x -
 * inflection_point_{desc}))} \right)\f$
 *
 * @param inflection_point_asc the inflection point of the ascending limb of the
 * double logistic function
 * @param slope_asc the slope of the ascending limb of the double logistic
 * function
 * @param inflection_point_desc the inflection point of the descending limb of
 * the double logistic function, where inflection_point_desc >
 * inflection_point_asc
 * @param slope_desc the slope of the descending limb of the double logistic
 * function
 * @param x the index the logistic function should be evaluated at
 * @return
 */

template <class Type>
inline const Type double_logistic(const Type &inflection_point_asc,
                                  const Type &slope_asc,
                                  const Type &inflection_point_desc,
                                  const Type &slope_desc, const Type &x) {
  return (static_cast<Type>(1.0)) /
         (static_cast<Type>(1.0) +
          exp(Type(-1.0) * slope_asc * (x - inflection_point_asc))) *
         (static_cast<Type>(1.0) -
          (static_cast<Type>(1.0)) /
              (static_cast<Type>(1.0) +
               exp(Type(-1.0) * slope_desc * (x - inflection_point_desc))));
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
template <class Type>
const Type ad_fabs(const Type &x, Type C = 1e-5) {
  return sqrt((x * x) + C);
}

/**
 *
 * Returns the minimum between a and b in a continuous manner using:
 *
 * (a + b - fims_math::ad_fabs(a - b))*.5;
 * Reference: \ref fims_math::ad_fabs()
 *
 * This is an approximation with minimal error.
 *
 * @param a
 * @param b
 * @param C default = 1e-5
 * @return
 */

template <typename Type>
inline const Type ad_min(const Type &a, const Type &b, Type C = 1e-5) {
  return (a + b - fims_math::ad_fabs(a - b, C)) * static_cast<Type>(0.5);
}

/**
 * Returns the maximum between a and b in a continuous manner using:
 *
 * (a + b + fims_math::ad_fabs(a - b)) *.5;
 * Reference: \ref fims_math::ad_fabs()
 * This is an approximation with minimal error.
 *
 * @param a
 * @param b
 * @param C default = 1e-5
 * @return
 */
template <typename Type>
inline const Type ad_max(const Type &a, const Type &b, Type C = 1e-5) {
  return (a + b + fims_math::ad_fabs(a - b, C)) * static_cast<Type>(.5);
}

/**
 * Sum elements of a vector
 *
 * @brief
 *
 * @param v A vector of constants.
 * @return A single numeric value.
 */
template <class T>
T sum(const std::vector<T> &v) {
  T ret = 0.0;
  for (int i = 0; i < v.size(); i++) {
    ret += v[i];
  }
  return ret;
}

/**
 * Sum elements of a vector
 *
 * @brief
 *
 * @param v A vector of constants.
 * @return A single numeric value.
 */
template <class T>
T sum(const fims::Vector<T> &v) {
  T ret = 0.0;
  for (int i = 0; i < v.size(); i++) {
    ret += v[i];
  }
  return ret;
}

}  // namespace fims_math

#endif /* FIMS_MATH_HPP */
