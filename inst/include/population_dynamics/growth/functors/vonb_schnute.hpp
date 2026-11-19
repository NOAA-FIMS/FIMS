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
 * @brief Von Bertalanffy growth using the Schnute parameterization.
 *
 * Mean length at age is defined using lengths at two reference ages:
 *
 * \f[
 * L(a) = L_y + (L_o - L_y)
 * \frac{1 - \exp[-K(a-a_y)]}{1 - \exp[-K(a_o-a_y)]}
 * \f]
 *
 * where:
 * - \f$a\f$ is the age being evaluated.
 * - \f$L_y\f$ is the mean length at the younger reference age
 *   (`mean_length_young`).
 * - \f$L_o\f$ is the mean length at the older reference age
 *   (`mean_length_old`).
 * - \f$a_y\f$ is the younger reference age
 *   (`reference_age_for_length_young`).
 * - \f$a_o\f$ is the older reference age
 *   (`reference_age_for_length_old`).
 * - \f$K\f$ is the growth coefficient (`growth_coefficient`).
 *
 * @details The equation above describes the main growth curve.
 * When the younger reference age is greater than zero, mean length
 * at earlier ages is calculated using a linear ramp from age zero
 * to the younger reference age. Ages used in the ramp are bounded
 * below by zero.
 *
 * For numerical stability, the implementation applies smooth
 * approximations to the absolute value and maximum operations,
 * using 1e-8 as the lower-bound argument. These approximations
 * can affect the curve when the denominator is small.
 *
 * @note When `reference_age_for_length_young` is zero,
 * `mean_length_young` represents mean length at age zero, L(0).
 * Setting the reference age to zero does not imply that the
 * traditional von Bertalanffy zero-length age parameter, t0,
 * is zero.
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
   * @brief Growth fraction at or after the younger reference age.
   * @details Requires ordered reference ages and nonnegative K.
   * Uses the linear-growth limit as K approaches zero.
   * @param age Age on the natural scale.
   * @param derivative Optional output derivative with respect to K.
   */
  Type growth_fraction(const Type& age, Type* derivative = nullptr) const {
    const Type elapsed_age = age - reference_age_for_length_young;
    const Type reference_age_span =
        reference_age_for_length_old - reference_age_for_length_young;

    // Evaluate (1 - exp(-x)) / x and, optionally, its derivative.
    auto scaled_decay = [](const Type& x, Type* decay_derivative) -> Type {
      const Type threshold = Type(1e-3);
      // Keep both expressions safe because AD may evaluate the inactive branch.
      const Type series_x = fims_math::cond_exp_lt(x, threshold, x, Type(0.0));
      const Type regular_x = fims_math::cond_exp_lt(x, threshold, Type(1.0), x);

      const Type series =
          Type(1.0) +
          series_x * (Type(-0.5) +
                      series_x * (Type(1.0 / 6.0) +
                                  series_x * (Type(-1.0 / 24.0) +
                                              series_x * Type(1.0 / 120.0))));
      const Type regular = (Type(1.0) - fims_math::exp(-regular_x)) / regular_x;

      if (decay_derivative != nullptr) {
        const Type series_derivative =
            Type(-0.5) + series_x * (Type(1.0 / 3.0) +
                                     series_x * (Type(-1.0 / 8.0) +
                                                 series_x * Type(1.0 / 30.0)));
        const Type regular_derivative =
            (fims_math::exp(-regular_x) - regular) / regular_x;

        *decay_derivative = fims_math::cond_exp_lt(
            x, threshold, series_derivative, regular_derivative);
      }

      return fims_math::cond_exp_lt(x, threshold, series, regular);
    };

    Type elapsed_derivative = Type(0.0);
    Type reference_derivative = Type(0.0);
    const Type elapsed_decay =
        scaled_decay(growth_coefficient * elapsed_age,
                     derivative != nullptr ? &elapsed_derivative : nullptr);
    const Type reference_decay =
        scaled_decay(growth_coefficient * reference_age_span,
                     derivative != nullptr ? &reference_derivative : nullptr);
    const Type age_ratio = elapsed_age / reference_age_span;
    const Type decay_ratio = elapsed_decay / reference_decay;

    if (derivative != nullptr) {
      *derivative = age_ratio *
                    (elapsed_age * elapsed_derivative -
                     decay_ratio * reference_age_span * reference_derivative) /
                    reference_decay;
    }

    return age_ratio * decay_ratio;
  }

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

    return mean_length_young +
           (mean_length_old - mean_length_young) * growth_fraction(age);
  }

  /**
   * @brief Evaluate log mean length at age.
   * @param age Age on the natural scale.
   * @return Log mean length at the requested age.
   */
  Type log_length_at_age(const Type& age) const {
    const Type length = length_at_age(age);
    const Type length_safe = fims_math::ad_max(length, static_cast<Type>(1e-8));
    return fims_math::log(length_safe);
  }

  /**
   * @brief Evaluate the gradient of log mean length at age with respect to
   * the current natural-scale FIMS VonBertalanffySchnute parameterization.
   * @param age Age on the natural scale.
   * @param d_log_laa_d_length_young Output derivative with respect to
   * mean_length_young.
   * @param d_log_laa_d_length_old Output derivative with respect to
   * mean_length_old.
   * @param d_log_laa_d_k Output derivative with respect to
   * growth_coefficient.
   */
  void log_length_at_age_gradient(const Type& age,
                                  Type& d_log_laa_d_length_young,
                                  Type& d_log_laa_d_length_old,
                                  Type& d_log_laa_d_k) const {
    if (reference_age_for_length_young > Type(0.0) &&
        age < reference_age_for_length_young) {
      const Type age_nonnegative = age <= Type(0.0) ? Type(0.0) : age;
      const Type ratio = age_nonnegative / reference_age_for_length_young;
      const Type mean_length = mean_length_young * ratio;
      const Type mean_length_safe =
          fims_math::ad_max(mean_length, static_cast<Type>(1e-8));

      d_log_laa_d_length_young = ratio / mean_length_safe;
      d_log_laa_d_length_old = Type(0.0);
      d_log_laa_d_k = Type(0.0);
      return;
    }

    Type d_ratio_d_k = Type(0.0);
    const Type ratio = growth_fraction(age, &d_ratio_d_k);
    const Type delta_length = mean_length_old - mean_length_young;
    const Type mean_length = mean_length_young + delta_length * ratio;
    const Type mean_length_safe =
        fims_math::ad_max(mean_length, static_cast<Type>(1e-8));

    const Type d_length_d_l1 = Type(1.0) - ratio;
    const Type d_length_d_l2 = ratio;
    const Type d_length_d_k = delta_length * d_ratio_d_k;
    d_log_laa_d_length_young = d_length_d_l1 / mean_length_safe;
    d_log_laa_d_length_old = d_length_d_l2 / mean_length_safe;
    d_log_laa_d_k = d_length_d_k / mean_length_safe;
  }

  /**
   * @brief Evaluate the gradient of log mean length at age with respect to
   * [log(mean_length_young), log(mean_length_old),
   *  log(growth_coefficient)].
   * @details These derivatives use physical reference lengths on log scales
   * for delta-method variability, not the optimizer's log length difference.
   * @param age Age on the natural scale.
   * @param d_log_laa_d_log_length_young Output derivative with respect to
   * log(mean_length_young).
   * @param d_log_laa_d_log_length_old Output derivative with respect to
   * log(mean_length_old).
   * @param d_log_laa_d_log_k Output derivative with respect to
   * log(growth_coefficient).
   */
  void log_length_at_age_logscale_gradient(const Type& age,
                                           Type& d_log_laa_d_log_length_young,
                                           Type& d_log_laa_d_log_length_old,
                                           Type& d_log_laa_d_log_k) const {
    Type d_log_laa_d_length_young = Type(0.0);
    Type d_log_laa_d_length_old = Type(0.0);
    Type d_log_laa_d_k = Type(0.0);

    log_length_at_age_gradient(age, d_log_laa_d_length_young,
                               d_log_laa_d_length_old, d_log_laa_d_k);

    d_log_laa_d_log_length_young = d_log_laa_d_length_young * mean_length_young;
    d_log_laa_d_log_length_old = d_log_laa_d_length_old * mean_length_old;
    d_log_laa_d_log_k = d_log_laa_d_k * growth_coefficient;
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
