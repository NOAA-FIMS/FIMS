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

#include <cmath>
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
    using std::exp;
    const Type denom = Type(1.0) - exp(-K * (age_L2 - age_L1));
    if (denom == Type(0.0)) {
      return L1;
    }
    const Type numer = Type(1.0) - exp(-K * (age - age_L1));
    return L1 + (L2 - L1) * numer / denom;
  }

  Type weight_at_age(const Type& age) const {
    using std::pow;
    Type L = length_at_age(age);
    return a_wl * pow(L, b_wl);
  }

  virtual const Type evaluate(const double& a) const override {
    return weight_at_age(Type(a));
  }
};
}  // namespace fims_popdy

#endif  // POPULATION_DYNAMICS_GROWTH_VON_BERTALANFFY_HPP
