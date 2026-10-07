/**
 * @file vonb_traditional.hpp
 * @brief Defines the traditional Von Bertalanffy growth functor.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_VONB_TRADITIONAL_HPP
#define POPULATION_DYNAMICS_GROWTH_VONB_TRADITIONAL_HPP

#include "../../../common/fims_math.hpp"
#include "growth_base.hpp"

namespace fims_popdy {

/**
 * @brief Traditional Von Bertalanffy growth functor for length-at-age and
 * weight-at-age.
 *
 * Parameterization:
 * \f[
 * L(a) = L_\infty \left(1 - \exp(-K(a - t_0))\right)
 * \f]
 *
 * where `asymptotic_length` is \f$L_\infty\f$, `growth_coefficient` is
 * \f$K\f$, and `age_at_zero_length` is \f$t_0\f$.
 */
template <typename Type>
struct VonBertalanffyTraditionalGrowth : public GrowthBase<Type> {
  /** @brief Asymptotic mean length, L-infinity, on the natural length scale. */
  Type asymptotic_length = Type(0.0);
  /** @brief Brody growth coefficient, K, on the natural scale. */
  Type growth_coefficient = Type(0.0);
  /** @brief Theoretical age at length zero, t0, on the natural age scale. */
  Type age_at_zero_length = Type(0.0);

  /** @brief Coefficient in the length-weight relationship, W = a * L^b. */
  Type length_weight_a = Type(0.0);
  /** @brief Exponent in the length-weight relationship, W = a * L^b. */
  Type length_weight_b = Type(3.0);

  VonBertalanffyTraditionalGrowth() : GrowthBase<Type>() {}
  virtual ~VonBertalanffyTraditionalGrowth() {}

  /**
   * @brief Evaluate mean length at age.
   * @param age Age on the natural scale.
   * @return Mean length at the requested age.
   */
  Type length_at_age(const Type& age) const {
    const Type age_delta = age - age_at_zero_length;
    const Type survival_like =
        fims_math::exp(-growth_coefficient * age_delta);
    return asymptotic_length * (Type(1.0) - survival_like);
  }

  /**
   * @brief Evaluate log mean length at age.
   * @param age Age on the natural scale.
   * @return Log mean length at the requested age.
   */
  Type log_length_at_age(const Type& age) const {
    const Type length = length_at_age(age);
    const Type length_safe =
        fims_math::ad_max(length, static_cast<Type>(1e-8));
    return fims_math::log(length_safe);
  }

  /**
   * @brief Evaluate the gradient of log mean length at age with respect to the
   * natural-scale traditional Von Bertalanffy parameters.
   * @param age Age on the natural scale.
   * @param d_log_laa_d_asymptotic_length Output derivative with respect to
   * asymptotic_length.
   * @param d_log_laa_d_growth_coefficient Output derivative with respect to
   * growth_coefficient.
   * @param d_log_laa_d_age_at_zero_length Output derivative with respect to
   * age_at_zero_length.
   */
  void log_length_at_age_gradient(
      const Type& age, Type& d_log_laa_d_asymptotic_length,
      Type& d_log_laa_d_growth_coefficient,
      Type& d_log_laa_d_age_at_zero_length) const {
    const Type age_delta = age - age_at_zero_length;
    const Type survival_like =
        fims_math::exp(-growth_coefficient * age_delta);
    const Type one_minus_survival_like = Type(1.0) - survival_like;
    const Type length = asymptotic_length * one_minus_survival_like;
    const Type length_safe =
        fims_math::ad_max(length, static_cast<Type>(1e-8));

    const Type d_length_d_asymptotic_length = one_minus_survival_like;
    const Type d_length_d_growth_coefficient =
        asymptotic_length * age_delta * survival_like;
    const Type d_length_d_age_at_zero_length =
        -asymptotic_length * growth_coefficient * survival_like;

    d_log_laa_d_asymptotic_length =
        d_length_d_asymptotic_length / length_safe;
    d_log_laa_d_growth_coefficient =
        d_length_d_growth_coefficient / length_safe;
    d_log_laa_d_age_at_zero_length =
        d_length_d_age_at_zero_length / length_safe;
  }

  /**
   * @brief Evaluate the gradient of log mean length at age with respect to the
   * traditional working-scale parameterization [log(L-infinity), log(K), t0].
   * @param age Age on the natural scale.
   * @param d_log_laa_d_log_asymptotic_length Output derivative with respect to
   * log(asymptotic_length).
   * @param d_log_laa_d_log_growth_coefficient Output derivative with respect to
   * log(growth_coefficient).
   * @param d_log_laa_d_age_at_zero_length Output derivative with respect to
   * age_at_zero_length.
   */
  void log_length_at_age_working_scale_gradient(
      const Type& age, Type& d_log_laa_d_log_asymptotic_length,
      Type& d_log_laa_d_log_growth_coefficient,
      Type& d_log_laa_d_age_at_zero_length) const {
    Type d_log_laa_d_asymptotic_length = Type(0.0);
    Type d_log_laa_d_growth_coefficient = Type(0.0);
    Type d_log_laa_d_age_at_zero_length_natural = Type(0.0);

    log_length_at_age_gradient(age, d_log_laa_d_asymptotic_length,
                               d_log_laa_d_growth_coefficient,
                               d_log_laa_d_age_at_zero_length_natural);

    d_log_laa_d_log_asymptotic_length =
        d_log_laa_d_asymptotic_length * asymptotic_length;
    d_log_laa_d_log_growth_coefficient =
        d_log_laa_d_growth_coefficient * growth_coefficient;
    d_log_laa_d_age_at_zero_length =
        d_log_laa_d_age_at_zero_length_natural;
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

#endif  // POPULATION_DYNAMICS_GROWTH_VONB_TRADITIONAL_HPP
