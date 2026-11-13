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

    const Type age_delta_1 = age - reference_age_for_length_young;
    const Type age_delta_2 =
        reference_age_for_length_old - reference_age_for_length_young;

    const Type exp_num = fims_math::exp(-growth_coefficient * age_delta_1);
    const Type exp_den = fims_math::exp(-growth_coefficient * age_delta_2);

    const Type numer = Type(1.0) - exp_num;
    const Type denom = Type(1.0) - exp_den;
    const Type denom_safe =
        fims_math::ad_max(fims_math::ad_fabs(denom), static_cast<Type>(1e-8));

    const Type ratio = numer / denom_safe;
    const Type delta_length = mean_length_old - mean_length_young;
    const Type mean_length = mean_length_young + delta_length * ratio;
    const Type mean_length_safe =
        fims_math::ad_max(mean_length, static_cast<Type>(1e-8));

    const Type d_length_d_l1 = Type(1.0) - ratio;
    const Type d_length_d_l2 = ratio;

    const Type d_numer_d_k = age_delta_1 * exp_num;
    const Type d_denom_d_k = age_delta_2 * exp_den;
    const Type d_ratio_d_k = (d_numer_d_k * denom_safe - numer * d_denom_d_k) /
                             (denom_safe * denom_safe);
    const Type d_length_d_k = delta_length * d_ratio_d_k;

    d_log_laa_d_length_young = d_length_d_l1 / mean_length_safe;
    d_log_laa_d_length_old = d_length_d_l2 / mean_length_safe;
    d_log_laa_d_k = d_length_d_k / mean_length_safe;
  }

  /**
   * @brief Evaluate the gradient of log mean length at age with respect to
   * the log-scale FIMS VonBertalanffySchnute parameterization
   * [log(mean_length_young), log(mean_length_old),
   *  log(growth_coefficient)].
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
