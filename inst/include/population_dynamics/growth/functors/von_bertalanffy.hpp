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
 * L(a) = L1 + (L2 - L1) * (1 - exp(-K * (a - A1))) /
 *        (1 - exp(-K * (A2 - A1)))
 */
template <typename Type>
struct VonBertalanffyGrowth : public GrowthBase<Type> {
  Type L1 = Type(0.0);
  Type L2 = Type(0.0);
  Type K = Type(0.0);
  Type age_L1 = Type(0.0);
  Type age_L2 = Type(0.0);

  // length-weight conversion
  Type a_wl = Type(0.0);
  Type b_wl = Type(3.0);

  VonBertalanffyGrowth() : GrowthBase<Type>() {}
  virtual ~VonBertalanffyGrowth() {}

  Type length_at_age(const Type& age) const {
    const Type denom = Type(1.0) -
        fims_math::exp(-K * (age_L2 - age_L1));
    // AD-safe floor to avoid divide-by-zero/NaN when denominator is tiny.
    const Type denom_safe = fims_math::ad_max(
        fims_math::ad_fabs(denom), static_cast<Type>(1e-8));
    const Type numer = Type(1.0) -
        fims_math::exp(-K * (age - age_L1));
    return L1 + (L2 - L1) * numer / denom_safe;
  }

  Type weight_at_age(const Type& age) const {
    Type L = length_at_age(age);
    return a_wl * fims_math::pow(L, b_wl);
  }

  virtual const Type evaluate(const double& a) const override {
    return weight_at_age(Type(a));
  }
};
}  // namespace fims_popdy

#endif  // POPULATION_DYNAMICS_GROWTH_VON_BERTALANFFY_HPP
