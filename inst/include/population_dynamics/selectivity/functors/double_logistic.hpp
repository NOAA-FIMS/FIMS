/**
 * @file double_logistic.hpp
 * @brief Declares the DoubleLogisticSelectivity class which implements the
 * logistic function from fims_math in the selectivity module.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC_HPP

#include "common/fims_math.hpp"
#include "common/fims_vector.hpp"
#include "selectivity_base.hpp"

namespace fims_popdy {

/**
 * @brief DoubleLogisticSelectivity class that returns the double logistic
 * function value from fims_math.
 */
template <typename Type>
struct DoubleLogisticSelectivity : public SelectivityBase<Type> {
  /**
   * @brief The age at which the ascending limb is 0.5 for models that start at
   * age zero and each age bin represents one year.
   * @details Overall selectivity is below 0.5 at this point because it is the
   * product of both limbs.
   */
  fims::Vector<Type> inflection_point_asc;
  /**
   * @brief How quickly selectivity rises on the ascending limb; larger values
   * give a steeper, more knife-edge rise.
   * @details Scalar multiplier applied to the difference between x and
   * inflection_point_asc on the ascending limb.
   */
  fims::Vector<Type> slope_asc;
  /**
   * @brief The age at which the descending limb is 0.5 for models that start at
   * age zero and each age bin represents one year.
   * @details Overall selectivity is below 0.5 at this point because it is the
   * product of both limbs.
   */
  fims::Vector<Type> inflection_point_desc;
  /**
   * @brief How quickly selectivity falls on the descending limb; larger values
   * give a steeper decline.
   * @details Scalar multiplier applied to the difference between x and
   * inflection_point_desc on the descending limb.
   */
  fims::Vector<Type> slope_desc;

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
  virtual const Type evaluate(const Type& x) {
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
   * @param pos Position index, e.g., which year. If the index is out of bounds
   * then it returns the first element, which would be the case when you do not
   * have time-varying selectivity.
   */
  virtual const Type evaluate(const Type& x, size_t pos) {
    return fims_math::double_logistic<Type>(
        inflection_point_asc.get_force_scalar(pos),
        slope_asc.get_force_scalar(pos),
        inflection_point_desc.get_force_scalar(pos),
        slope_desc.get_force_scalar(pos), x);
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC_HPP */
