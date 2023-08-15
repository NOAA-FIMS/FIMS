/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to define the LogisticMaturity class, which
 * inherits from the MaturityBase class
 */
#ifndef POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP
#define POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP

//#include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "maturity_base.hpp"

namespace fims {

/**
 *  @brief LogisticMaturity class that returns the logistic function value
 * from fims_math.
 */
template <typename T> struct LogisticMaturity : public MaturityBase<T> {
  T median; /*!< 50% quantile of the value of the quantity of interest (x); e.g.
               age at which 50% of the fish are mature */
  T slope;  /*!<scalar multiplier of difference between quantity of interest
               value (x) and median */

  LogisticMaturity() : MaturityBase<T>() {}

  /**
   * @brief Method of the logistic maturity class that implements the
   * logistic function from FIMS math.
   *
   * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - median))} \f]
   *
   * @param x  The independent variable in the logistic function (e.g., age or
   * size at maturity).
   */
  virtual const T evaluate(const T &x) {
    return fims::logistic<T>(median, slope, x);
  }
};

} // namespace fims

#endif /* POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP */
