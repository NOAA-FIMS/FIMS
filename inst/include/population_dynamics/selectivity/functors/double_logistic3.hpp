/**
 * @file double_logistic3.hpp
 * @brief Declares the DoubleLogistic3Selectivity class.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC3_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC3_HPP

#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
#include "selectivity_base.hpp"

namespace fims_popdy {

/**
 * @brief Three-parameter double logistic selectivity class.
 */
template <typename Type>
struct DoubleLogistic3Selectivity : public SelectivityBase<Type> {
  fims::Vector<Type> p1; /**< Ascending limb width from 50% to 95%. */
  fims::Vector<Type> p2; /**< Horizontal shift of the ascending limb. */
  fims::Vector<Type> p3; /**< Descending limb width from 50% to 5%. */

  DoubleLogistic3Selectivity() : SelectivityBase<Type>() {}

  virtual ~DoubleLogistic3Selectivity() {}

  /**
   * @brief Evaluate the three-parameter double logistic selectivity function.
   *
   * @param x The independent variable, e.g., age or size.
   */
  virtual const Type evaluate(const Type &x) {
    return fims_math::double_logistic3<Type>(p1[0], p2[0], p3[0], x);
  }

  /**
   * @copydoc DoubleLogistic3Selectivity::evaluate(const Type &x)
   * @param pos Position index, e.g., which year. If the index is out of bounds
   * then it returns the first element, which is the time-invariant case.
   */
  virtual const Type evaluate(const Type &x, size_t pos) {
    return fims_math::double_logistic3<Type>(
        p1.get_force_scalar(pos), p2.get_force_scalar(pos),
        p3.get_force_scalar(pos), x);
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_LOGISTIC3_HPP */
