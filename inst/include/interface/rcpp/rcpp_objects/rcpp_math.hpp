/**
 * @file rcpp_math.hpp
 * @brief The Rcpp interface to declare different mathematical functions that
 * are written in C++ but can be used within R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_MATH_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_MATH_HPP

#include "rcpp_interface_base.hpp"
#include "../../../common/fims_math.hpp"

/**
 * @brief A rcpp interface to the logit function.
 *
 * @param a Lower bound of the logit function, typically 0.0.
 * @param b Upper bound of the logit function, typically 1.0.
 * @param x A single numeric value (double) to be transformed on the real line.
 * @return A double in real space rather than the bounded space.
 */
double logit_rcpp(double a, double b, double x) {
  return fims_math::logit<double>(a, b, x);
}

/**
 * @brief A rcpp interface to the inverse-logit function.
 *
 * @param a Lower bound of the logit function, typically 0.0.
 * @param b Upper bound of the logit function, typically 1.0.
 * @param logit_x A single numeric value (double) in real space.
 * @return A double in the bounded space rather than real space.
 */
double inv_logit_rcpp(double a, double b, double logit_x) {
  return fims_math::inv_logit<double>(a, b, logit_x);
}

#endif
