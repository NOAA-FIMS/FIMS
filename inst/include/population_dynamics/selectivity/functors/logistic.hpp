/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to declare the LogisticSelectivity class
 * which implements the logistic function from fims_math in the selectivity
 * module.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP

//#include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "selectivity_base.hpp"

namespace fims
{

/**
 *  @brief LogisticSelectivity class that returns the logistic function value
 * from fims_math.
 */
template <typename T> struct LogisticSelectivity : public SelectivityBase<T>
{
  T median; /*!< 50% quantile of the value of the quantity of interest (x);
               e.g. age at which 50% of the fish are selected */
  T slope;  /*!<scalar multiplier of difference between quantity of interest
               value (x) and median */

  LogisticSelectivity () : SelectivityBase<T> () {}

  virtual ~LogisticSelectivity () {}

  /**
   * @brief Method of the logistic selectivity class that implements the
   * logistic function from FIMS math.
   *
   * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - median))} \f]
   *
   * @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual const T
  evaluate (const T &x)
  {
    return fims::logistic<T> (median, slope, x);
  }
};

} // namespace fims

#endif /* POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP */
