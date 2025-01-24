/**
 * @file fims_math.hpp
 * @brief TODO: provide a brief description.
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
#endif

#ifdef TMB_MODEL

template<class Type>
vector<Type> rmultinom(Type N, vector<Type> p, vector<int> ages)
{
  //multinomial
  int dim = ages.size();
  vector<Type> x(dim);
  vector<Type> p_use(dim);
  for(int i = 0; i < dim; i++) p_use(i) = p(ages(i)-1);
  p_use /= sum(p_use);
  int Nint = CppAD::Integer(N);
  x.setZero();
  for(int i = 0; i < Nint; i++)
  {
    Type y = runif(0.0,1.0);
    for(int a = 0; a < dim; a++) if(y < p_use.head(a+1).sum())
    {
      x(a) += 1.0;
      break;
    }
  }
  return x;
}

/**
 * @brief The exponential function.
 * The code cannot be tested using the compilation flag
 * -DTMB_MODEL through CMake and Google Test
 * @param x value to exponentiate. Please use fims_math::exp<double>(x) if x is
 * an integer.
 * @return the exponentiated value
 */
template <class Type>
inline const Type exp(const Type &x) {
  using ::exp;
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
 * @param x the value to take the log of. Please use fims_math::log<double>(x)
 * if x is an integer.
 * @return The natural log of the value.
 */
template <class Type>
inline const Type log(const Type &x) {
  return log(x);
}

template <>
inline const double log(const double &x) {
  return std::log(x);
}

template <class Type>
inline const Type cos(const Type &x) {
    return cos(x);
}

template <>
inline const double cos(const double &x) {
    return std::cos(x);
}

template <class Type>
inline const Type sqrt(const Type &x) {
    return sqrt(x);
}

template <>
inline const double sqrt(const double &x) {
    return std::sqrt(x);
}

template <class Type>
inline const Type pow(const Type &x, const Type &y) {
    return pow(x, y);
}

template <>
inline const double pow(const double &x, const double &y) {
    return std::pow(x, y);
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
  return (1.0) / (1.0 + exp(-1.0 * slope * (x - inflection_point)));
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
  return a + (b - a) / (1.0 + fims_math::exp(-logit_x));
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
  return (1.0) / (1.0 + exp(-1.0 * slope_asc * (x - inflection_point_asc))) *
         (1.0 -
          (1.0) / (1.0 + exp(-1.0 * slope_desc * (x - inflection_point_desc))));
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
  return (a + b - fims_math::ad_fabs(a - b, C)) * 0.5;
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
    template<class T>
    T sum(const std::vector<T>& v) {
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
    template<class T>
    T sum(const fims::Vector<T>& v) {
        T ret = 0.0;
        for (int i = 0; i < v.size(); i++) {
            ret += v[i];
        }
        return ret;
    }

}  // namespace fims_math

#endif /* FIMS_MATH_HPP */
