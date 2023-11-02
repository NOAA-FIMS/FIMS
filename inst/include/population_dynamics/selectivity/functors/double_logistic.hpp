/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC_HPP

//#include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "selectivity_base.hpp"

namespace fims_popdy {

/**
 * @brief DoubleLogisticSelectivity class that returns the double logistic
 * function value from fims_math.
 */
template <typename Type>
struct DoubleLogisticSelectivity : public SelectivityBase<Type> {
  Type inflection_point_asc;  /*!< 50% quantile of the value of the quantity of interest
                (x)  on the ascending limb of the double logistic curve; e.g. age
                at  which 50% of the fish are selected */
  Type slope_asc;   /*!<scalar multiplier of difference between quantity of
                interest   value (x) and inflection_point on the ascending limb of the
                double   logistic   curve*/
  Type inflection_point_desc; /*!< 50% quantile of the value of the quantity of interest
               (x) on the descending limb of the double logistic curve; e.g.
               age at which 50% of the fish are selected */
  Type slope_desc;  /*!<scalar multiplier of difference between quantity of
               interest  value (x) and inflection_point on the descending limb of the
               double  logistic  curve */

  DoubleLogisticSelectivity() : SelectivityBase<Type>() {}

  virtual ~DoubleLogisticSelectivity() {}
  /**
   * @brief Method of the double logistic selectivity class that implements the
   * double logistic function from FIMS math.
   *
   * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope_{asc} (x - inflection_point_{asc}))}
   * \left(1-\frac{1.0}{ 1.0 + exp(-1.0 * slope_{desc} (x - inflection_point_{desc}))}
   * \right)\f$
   *
   * @param x  The independent variable in the double logistic function (e.g.,
   * age or size in selectivity).
   */
  virtual const Type evaluate(const Type &x) {
    return fims_math::double_logistic<Type>(inflection_point_asc, slope_asc, inflection_point_desc,
                                       slope_desc, x);
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC_HPP */
