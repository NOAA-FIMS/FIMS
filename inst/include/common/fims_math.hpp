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
 * @brief Calculates a single logistic function.
 *
 * @details This function computes a logistic growth curve, which is
 * commonly used to model S-shaped growth in various systems. The formula
 * for the logistic function is:
 *
 * \f[
 * f(x) = \frac{1}{1 + e^{-k(x - x_0)}}
 * \f]
 *
 * where \f$v\f$ is the inflection point and \f$k\f$ is the slope of
 * the curve.
 *
 * @tparam Type The numeric type of the function's arguments.
 * @param inflection_point The inflection point of the logistic curve
 * (\f$v\f$).
 * @param slope The slope of the logistic curve (\f$k\f$).
 * @param x The independent variable.
 * @return The result of the logistic function.
 *
 * @note This function is declared `inline` for potential performance
 * benefits.
 */
template <class Type>
inline const Type logistic(const Type &inflection_point, const Type &slope,
                           const Type &x) {
  return static_cast<Type>(1.0) /
         (static_cast<Type>(1.0) +
          exp(Type(-1.0) * slope * (x - inflection_point)));
}

/**
 * @brief Calculates the logit function for bounding of parameters.
 *
 * \f[ -\mathrm{log}(b-x) + \mathrm{log}(x-a) \f]
 * @param a The lower bound \f$a\f$.
 * @param b The upper bound \f$b\f$.
 * @param x The parameter in bounded space.
 * @return The parameter in real space.
 *
 */
template <class Type>
inline const Type logit(const Type &a, const Type &b, const Type &x) {
  return -fims_math::log(b - x) + fims_math::log(x - a);
}

/**
 * @brief Calculates the inverse logit function for bounding of parameters.
 *
 * \f$ a+\frac{b-a}{1+\mathrm{exp}(-\mathrm{logit}(x))}\f$
 * @param a The lower bound \f$a\f$.
 * @param b The upper bound \f$b\f$.
 * @param logit_x The parameter in real space.
 * @return The parameter in bounded space.
 *
 */
template <class Type>
inline const Type inv_logit(const Type &a, const Type &b, const Type &logit_x) {
  return a + (b - a) / (static_cast<Type>(1.0) + fims_math::exp(-logit_x));
}

 /**
 * @brief Calculates a double logistic function.
 *
 * @details This function computes a double logistic curve, which can
 * be used to model processes that rise and then fall. It is composed
 * of two logistic functions, one for the ascending phase and one for
 * the descending phase. The formulation is as follows:
 *
 * \f[
 * f(x) = \frac{1}{1 + e^{-k_1(x - v_1)}} \cdot
 * \left( 1 - \frac{1}{1 + e^{-k_2(x - v_2)}} \right)
 * \f]
 * where \f$k_1\f$ and \f$v_1\f$ are the slope and inflection point of
 * the ascending curve, and \f$k_2\f$ and \f$v_2\f$ are for the
 * descending curve.
 *
 * @tparam Type The numeric type of the function's arguments.
 * @param inflection_point_asc The inflection point of the ascending
 * logistic curve (\f$v_1\f$).
 * @param slope_asc The slope of the ascending logistic curve (\f$k_1\f$).
 * @param inflection_point_desc The inflection point of the descending logistic
 * curve (\f$v_2\f$), where `inflection_point_desc` > `inflection_point_asc`.
 * @param slope_desc The slope of the descending logistic curve (\f$k_2\f$).
 * @param x The independent variable.
 * @return The result of the double logistic function.
 *
 * @note This function is declared `inline` for potential performance
 * benefits.
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
 * @brief Calculates a smooth approximation of the absolute value.
 *
 * @details This function computes a differentiable approximation of the
 * absolute value using a squared Euclidean norm, which avoids the
 * non-differentiable point at zero. This is particularly useful in
 * optimization algorithms where a smooth, continuous derivative is
 * required. The formula used is:
 *
 * \f[
 * \text{ad_fabs}(x) = \sqrt{x^2 + C}
 * \f]
 *
 * where \f$C\f$ is a small constant to prevent the derivative from
 * becoming infinite at zero. A common choice for \f$C\f$ is a small
 * positive value like \f$10^{-5}\f$. As \f$C\f$ approaches zero, the
 * function approaches the true absolute value, \f$|x|\f$.
 *
 * @tparam Type The numeric type of the function's arguments.
 * @param x The value for which to calculate the pseudo-absolute value, i.e.,
 * the value you want to keep positive.
 * @param C A small constant to ensure differentiability at zero. The
 * default value is 1e-5.
 * @return The smooth, differentiable pseudo-absolute value of `x`.
 */
template <class Type>
const Type ad_fabs(const Type &x, Type C = 1e-5) {
  return sqrt((x * x) + C);
}

/**
 * @brief Calculates a smooth approximation of the minimum of two values.
 *
 * @details This function computes a differentiable approximation of the
 * minimum of two values, `a` and `b`. This is useful in optimization
 * algorithms where a smooth, continuous derivative is required. The
 * formula is derived from the property that the minimum of two numbers
 * can be expressed using the absolute value:
 *
 * \f[
 * \min(a, b) = \frac{1}{2} (a + b - |a - b|)
 * \f]
 *
 * This function substitutes the non-differentiable `abs()` function with \ref
 * fims_math::ad_fabs(), a smooth approximation of the absolute value, to
 * ensure differentiability at all points.
 *
 * @tparam Type The numeric type of the function's arguments.
 * @param a The first value.
 * @param b The second value.
 * @param C A small constant to ensure differentiability, passed to
 * \ref fims_math::ad_fabs().
 * @return A smooth, differentiable approximation of the minimum of `a` and `b`.
 */
template <typename Type>
inline const Type ad_min(const Type &a, const Type &b, Type C = 1e-5) {
  return (a + b - fims_math::ad_fabs(a - b, C)) * static_cast<Type>(0.5);
}

/**
 * @brief Calculates a smooth approximation of the maximum of two values.
 *
 * @details This function computes a differentiable approximation of the
 * maximum of two values, `a` and `b`. This is useful in optimization
 * algorithms where a smooth, continuous derivative is required. The
 * formula is derived from the property that the maximum of two numbers
 * can be expressed using the absolute value:
 *
 * \f[
 * \max(a, b) = \frac{1}{2} (a + b + |a - b|)
 * \f]
 *
 * This function substitutes the non-differentiable `abs()` function with \ref
 * fims_math::ad_fabs(), a smooth approximation of the absolute value, to
 * ensure differentiability at all points.
 *
 * @tparam Type The numeric type of the function's arguments.
 * @param a The first value.
 * @param b The second value.
 * @param C A small constant to ensure differentiability, passed to \ref
 * fims_math::ad_fabs().
 * @return A smooth, differentiable approximation of the maximum of `a` and `b`.
 */
template <typename Type>
inline const Type ad_max(const Type &a, const Type &b, Type C = 1e-5) {
  return (a + b + fims_math::ad_fabs(a - b, C)) * static_cast<Type>(.5);
}

/**
 * @brief Calculates the sum of elements in a vector.
 *
 * @details This function iterates through a vector of any numeric
 * type and calculates the sum of all its elements. The result is
 * returned as the same type as the elements in the input vector.
 *
 * @tparam T The numeric type of the elements in the vector.
 * @param v The input vector whose elements are to be summed.
 * @return The sum of all elements in the vector `v`.
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
 * @brief Calculates the sum of elements in a vector.
 *
 * @details This function iterates through a vector of any numeric
 * type and calculates the sum of all its elements. The result is
 * returned as the same type as the elements in the input vector.
 *
 * @tparam T The numeric type of the elements in the vector.
 * @param v The input vector whose elements are to be summed.
 * @return The sum of all elements in the vector `v`.
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
