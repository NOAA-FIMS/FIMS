/**
 * @file von_bertalanffy.hpp
 * @brief Defines the VonBertalanffyGrowth class, which inherits from the
 * GrowthBase class.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_VON_BERTALANFFY_HPP
#define POPULATION_DYNAMICS_GROWTH_VON_BERTALANFFY_HPP

#include "../../../common/fims_math.hpp"
#include "growth_base.hpp"

namespace fims_popdy {

/**
 * @brief Von Bertalanffy growth functor for length-at-age and weight-at-age.
 *
 * Parameterization using length at two reference ages:
 * L(a) = length_at_ref_age_1 +
 *        (length_at_ref_age_2 - length_at_ref_age_1) *
 *        (1 - exp(-growth_coefficient_K * (a - reference_age_for_length_1))) /
 *        (1 - exp(-growth_coefficient_K *
 *                 (reference_age_for_length_2 - reference_age_for_length_1)))
 */
template <typename Type>
struct VonBertalanffyGrowth : public GrowthBase<Type> {
  Type length_at_ref_age_1 = Type(0.0);
  Type length_at_ref_age_2 = Type(0.0);
  Type growth_coefficient_K = Type(0.0);
  Type reference_age_for_length_1 = Type(0.0);
  Type reference_age_for_length_2 = Type(0.0);

  // length-weight conversion
  Type length_weight_a = Type(0.0);
  Type length_weight_b = Type(3.0);

  VonBertalanffyGrowth() : GrowthBase<Type>() {}
  virtual ~VonBertalanffyGrowth() {}

  Type length_at_age(const Type& age) const {
    const Type denom = Type(1.0) -
        fims_math::exp(-growth_coefficient_K * (reference_age_for_length_2 -
                                              reference_age_for_length_1));
    // AD-safe floor to avoid divide-by-zero/NaN when denominator is tiny.
    const Type denom_safe = fims_math::ad_max(
        fims_math::ad_fabs(denom), static_cast<Type>(1e-8));
    const Type numer = Type(1.0) -
        fims_math::exp(-growth_coefficient_K *
                       (age - reference_age_for_length_1));
    return length_at_ref_age_1 +
           (length_at_ref_age_2 - length_at_ref_age_1) * numer / denom_safe;
  }

  Type weight_at_age(const Type& age) const {
    Type length = length_at_age(age);
    return length_weight_a * fims_math::pow(length, length_weight_b);
  }

  virtual const Type evaluate(const double& a) const override {
    return weight_at_age(Type(a));
  }
};
}  // namespace fims_popdy

#endif  // POPULATION_DYNAMICS_GROWTH_VON_BERTALANFFY_HPP
