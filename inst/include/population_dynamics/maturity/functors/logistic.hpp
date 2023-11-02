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

namespace fims_popdy {

/**
 *  @brief LogisticMaturity class that returns the logistic function value
 * from fims_math.
 */
template <typename Type>
struct LogisticMaturity : public MaturityBase<Type> {
  Type inflection_point; /*!< 50% quantile of the value of the quantity of interest (x);
               e.g. age at which 50% of the fish are mature */
  Type slope;  /*!<scalar multiplier of difference between quantity of interest
               value (x) and inflection_point */

  LogisticMaturity() : MaturityBase<Type>() {}

  /**
   * @brief Method of the logistic maturity class that implements the
   * logistic function from FIMS math.
   *
   * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - inflection_point))} \f]
   *
   * @param x  The independent variable in the logistic function (e.g., age or
   * size at maturity).
   */

  virtual const Type evaluate(const Type & x) {
    return fims_math::logistic<Type>(inflection_point, slope, x);
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP */
