/**
 * @file vonb_schnute.hpp
 * @brief Defines the VonBertalanffySchnuteGrowth class, which inherits from the
 * GrowthBase class.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_VONB_SCHNUTE_HPP
#define POPULATION_DYNAMICS_GROWTH_VONB_SCHNUTE_HPP

#include "../../../common/fims_math.hpp"
#include "growth_base.hpp"

namespace fims_popdy {

/**
 * @brief VonBertalanffySchnute growth functor for length-at-age and
 * weight-at-age.
 *
 * Parameterization using length at two reference ages:
 * L(a) = mean_length_young +
 *        (mean_length_old - mean_length_young) *
 *        (1 - exp(-growth_coefficient * (a - reference_age_for_length_young)))
 * / (1 - exp(-growth_coefficient * (reference_age_for_length_old -
 * reference_age_for_length_young)))
 * @note Users can anchor at t0 by setting reference_age_for_length_young = 0
 * and mean_length_young = L(0). This is valid as long as
 * reference_age_for_length_old > reference_age_for_length_young.
 */
template <typename Type>
struct VonBertalanffySchnuteGrowth : public GrowthBase<Type> {
  /** @brief Expected length at the first reference age. */
  Type mean_length_young = Type(0.0);
  /** @brief Expected length at the second reference age. */
  Type mean_length_old = Type(0.0);
  /** @brief Growth coefficient. */
  Type growth_coefficient = Type(0.0);
  /** @brief First reference age for the length parameterization. */
  Type reference_age_for_length_young = Type(0.0);
  /** @brief Second reference age for the length parameterization. */
  Type reference_age_for_length_old = Type(0.0);

  /** @brief Coefficient in the length-weight relationship, W = a * L^b. */
  Type length_weight_a = Type(0.0);
  /** @brief Exponent in the length-weight relationship, W = a * L^b. */
  Type length_weight_b = Type(3.0);

  VonBertalanffySchnuteGrowth() : GrowthBase<Type>() {}
  virtual ~VonBertalanffySchnuteGrowth() {}

  /**
   * @brief Evaluate mean length at age.
   * @param age Age on the natural scale.
   * @return Mean length at the requested age.
   */
  Type length_at_age(const Type& age) const {
    if (reference_age_for_length_young > Type(0.0) &&
        age < reference_age_for_length_young) {
      // For early ages, avoid back-extrapolating the curved growth equation.
      // This no-seasons ramp goes from length 0 at age 0 to length young at
      // age young; seasonal growth may need a different transition in the
      // future.
      const Type age_nonnegative = age <= Type(0.0) ? Type(0.0) : age;
      return mean_length_young * age_nonnegative /
             reference_age_for_length_young;
    }

    const Type denom =
        Type(1.0) -
        fims_math::exp(-growth_coefficient * (reference_age_for_length_old -
                                              reference_age_for_length_young));
    // AD-safe floor to avoid divide-by-zero/NaN when denominator is tiny.
    const Type denom_safe =
        fims_math::ad_max(fims_math::ad_fabs(denom), static_cast<Type>(1e-8));
    const Type numer =
        Type(1.0) - fims_math::exp(-growth_coefficient *
                                   (age - reference_age_for_length_young));
    return mean_length_young +
           (mean_length_old - mean_length_young) * numer / denom_safe;
  }

  /**
   * @brief Evaluate mean weight at age via the length-weight relationship.
   * @param age Age on the natural scale.
   * @return Mean weight at the requested age.
   */
  Type weight_at_age(const Type& age) const {
    Type length = length_at_age(age);
    return length_weight_a * fims_math::pow(length, length_weight_b);
  }

  virtual const Type evaluate(int year, const double& a) override {
    (void)year;
    return weight_at_age(Type(a));
  }
};
}  // namespace fims_popdy

#endif  // POPULATION_DYNAMICS_GROWTH_VONB_SCHNUTE_HPP
