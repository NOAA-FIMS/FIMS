/**
 * @file logistic.hpp
 * @brief Defines the LogisticMaturity class, which inherits from the
 * MaturityBase class.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 *
 */
#ifndef POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP
#define POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP

#include "common/fims_math.hpp"
#include "common/fims_vector.hpp"
#include "maturity_base.hpp"

namespace fims_popdy {

/**
 *  @brief LogisticMaturity class that returns the logistic function value
 * from fims_math.
 */
template <typename Type>
struct LogisticMaturity : public MaturityBase<Type> {
  /**
   * @brief The age at which 50% of fish are mature for models that start at
   * age zero and each age bin represents one year.
   */
  fims::Vector<Type> inflection_point;
  /**
   * @brief How quickly the proportion mature changes with age around the
   * inflection point, per year of age. Larger absolute values give a steeper,
   * more knife-edge maturity curve.
   */
  fims::Vector<Type> slope;

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

  virtual const Type evaluate(const Type& x) {
    return fims_math::logistic<Type>(inflection_point[0], slope[0], x);
  }

  /**
   * @brief Method of the logistic maturity class that implements the
   * logistic function from FIMS math.
   *
   * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope_t (x - {inflection\_point}_t))} \f]
   *
   * Time-varying parameters have 1 value per model year. The year after the
   * last model year (`pos` equal to the number of values) uses the last
   * year's values.
   *
   * @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate(const Type& x, size_t pos) {
    // Only the position 1 past the end is moved back, so a parameter with too
    // few values still throws in get_force_scalar() for later years.
    size_t inflection_point_pos =
        (pos == inflection_point.size()) ? pos - 1 : pos;
    size_t slope_pos = (pos == slope.size()) ? pos - 1 : pos;
    return fims_math::logistic<Type>(
        inflection_point.get_force_scalar(inflection_point_pos),
        slope.get_force_scalar(slope_pos), x);
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP */
