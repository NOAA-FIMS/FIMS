/**
 * @file logistic.hpp
 * @brief Declares the DoubleLogisticSelectivity class which implements the
 * logistic function from fims_math in the selectivity module.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC_HPP

//#include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
#include "selectivity_base.hpp"

namespace fims_popdy {

/**
 * @brief DoubleLogisticSelectivity class that returns the double logistic
 * function value from fims_math.
 */
template <typename Type>
struct DoubleLogisticSelectivity : public SelectivityBase<Type> {
  fims::Vector<Type> inflection_point_asc; /**< 50% quantile of the value of the
             quantity of interest (x)  on the ascending limb of the double
             logistic curve; e.g. age at  which 50% of the fish are selected */
  fims::Vector<Type> slope_asc; /**<scalar multiplier of difference between
            quantity of interest   value (x) and inflection_point on the
            ascending limb of the double   logistic   curve*/
  fims::Vector<Type> inflection_point_desc; /**< 50% quantile of the value of
             the quantity of interest (x) on the descending limb of the double
             logistic curve; e.g. age at which 50% of the fish are selected */
  fims::Vector<Type> slope_desc; /**<scalar multiplier of difference between
            quantity of interest  value (x) and inflection_point on the
            descending limb of the double  logistic  curve */

  DoubleLogisticSelectivity() : SelectivityBase<Type>() {}

  virtual ~DoubleLogisticSelectivity() {}

  /**
   * @brief Method of the double logistic selectivity class that implements the
   * double logistic function from FIMS math.
   *
   * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope\_asc (x - inflection_point\_asc))}
   * \left(1.0-\frac{1.0}{ 1.0 + exp(-1.0 * slope\_desc (x -
   * inflection_point\_desc))} \right)\f$
   *
   * @param x  The independent variable in the double logistic function (e.g.,
   * age or size in selectivity).
   */
  virtual const Type evaluate(const Type &x) {
    return fims_math::double_logistic<Type>(
        inflection_point_asc[0], slope_asc[0], inflection_point_desc[0],
        slope_desc[0], x);
  }

  /**
   * @brief Method of the double logistic selectivity class that implements the
   * double logistic function from FIMS math.
   *
   * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope\_asc_t (x -
   * inflection_point\_asc_t))} \left(1.0-\frac{1.0}{ 1.0 + exp(-1.0 *
   * slope\_desc_t (x - inflection_point\_desc_t))} \right)\f$
   *
   * @param x  The independent variable in the double logistic function (e.g.,
   * age or size in selectivity).
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate(const Type &x, size_t pos) {
    return fims_math::double_logistic<Type>(
        inflection_point_asc.get_force_scalar(pos),
        slope_asc.get_force_scalar(pos),
        inflection_point_desc.get_force_scalar(pos),
        slope_desc.get_force_scalar(pos), x);
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC_HPP */
