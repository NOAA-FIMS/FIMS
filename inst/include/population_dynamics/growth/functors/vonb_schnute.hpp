/**
 * @file vonb_schnute.hpp
 * @brief Defines the VonBSchnuteGrowth class, which inherits from the
 * GrowthBase class.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_VONB_SCHNUTE_HPP
#define POPULATION_DYNAMICS_GROWTH_VONB_SCHNUTE_HPP

#include <stdexcept>
#include "../../../common/fims_math.hpp"
#include "growth_base.hpp"

namespace fims_popdy {

/**
 * @brief VonB-Schnute growth functor for length-at-age and weight-at-age.
 *
 * Parameterization using length at two reference ages:
 * L(a) = mean_length_young +
 *        (mean_length_old - mean_length_young) *
 *        (1 - exp(-von_bertalanffy_coefficient_K * (a - reference_age_for_length_1))) /
 *        (1 - exp(-von_bertalanffy_coefficient_K *
 *                 (reference_age_for_length_2 - reference_age_for_length_1)))
 * @note Users can anchor at t0 by setting reference_age_for_length_1 = 0
 * and mean_length_young = L(0). This is valid as long as
 * reference_age_for_length_2 > reference_age_for_length_1.
 */
template <typename Type>
struct VonBSchnuteGrowth : public GrowthBase<Type> {
  Type mean_length_young = Type(0.0); /**< expected length at reference age 1 */
  Type mean_length_old = Type(0.0); /**< expected length at reference age 2 */
  Type von_bertalanffy_coefficient_K = Type(0.0); /**< growth coefficient */
  Type reference_age_for_length_1 = Type(0.0); /**< first reference age */
  Type reference_age_for_length_2 = Type(0.0); /**< second reference age */

  Type length_weight_a = Type(0.0); /**< coefficient in W = a * L^b */
  Type length_weight_b = Type(3.0); /**< exponent in W = a * L^b */

  VonBSchnuteGrowth() : GrowthBase<Type>() {}
  virtual ~VonBSchnuteGrowth() {}

  /**
   * @brief Validate the VonB-Schnute parameters used by length-at-age
   * calculations.
   *
   * Throws when the stored growth parameter values are incompatible with the
   * raw VonB-Schnute length-at-age calculation.
   */
  void ValidateLengthParameters() const {
    if (von_bertalanffy_coefficient_K <= Type(0.0)) {
      throw std::runtime_error(
          "VonBSchnuteGrowth von_bertalanffy_coefficient_K must be > 0");
    }

    if (mean_length_young <= Type(0.0)) {
      throw std::runtime_error(
          "VonBSchnuteGrowth mean_length_young must be > 0");
    }

    if (mean_length_old <= Type(0.0)) {
      throw std::runtime_error(
          "VonBSchnuteGrowth mean_length_old must be > 0");
    }

    if (reference_age_for_length_2 <= reference_age_for_length_1) {
      throw std::runtime_error(
          "VonBSchnuteGrowth reference_age_for_length_2 must be > "
          "reference_age_for_length_1");
    }

    if (mean_length_old <= mean_length_young) {
      throw std::runtime_error(
          "VonBSchnuteGrowth mean_length_old must be > "
          "mean_length_young");
    }
  }

  /**
   * @brief Validate the length-weight parameters used by weight-at-age
   * calculations.
   */
  void ValidateWeightParameters() const {
    if (length_weight_a <= Type(0.0)) {
      throw std::runtime_error(
          "VonBSchnuteGrowth length_weight_a must be > 0");
    }

    if (length_weight_b <= Type(0.0)) {
      throw std::runtime_error(
          "VonBSchnuteGrowth length_weight_b must be > 0");
    }
  }

  /**
   * @brief Evaluate mean length at age.
   * @param age Age on the natural scale.
   * @return Mean length at the requested age.
   */
  Type length_at_age(const Type& age) const {
    ValidateLengthParameters();

    if (reference_age_for_length_1 > Type(0.0) &&
        age < reference_age_for_length_1) {
      // For early ages, avoid back-extrapolating the curved growth equation.
      // This no-seasons ramp goes from length 0 at age 0 to L1 at A1;
      // seasonal growth may need a different transition in the future.
      const Type age_nonnegative = age <= Type(0.0) ? Type(0.0) : age;
      return mean_length_young * age_nonnegative /
             reference_age_for_length_1;
    }

    const Type denom = Type(1.0) -
        fims_math::exp(-von_bertalanffy_coefficient_K * (reference_age_for_length_2 -
                                                reference_age_for_length_1));
    // AD-safe floor to avoid divide-by-zero/NaN when denominator is tiny.
    const Type denom_safe = fims_math::ad_max(
        fims_math::ad_fabs(denom), static_cast<Type>(1e-8));
    const Type numer = Type(1.0) -
        fims_math::exp(-von_bertalanffy_coefficient_K *
                       (age - reference_age_for_length_1));
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
    const Type length_safe = fims_math::ad_max(
        length, static_cast<Type>(1e-8));
    return fims_math::log(length_safe);
  }

  /**
   * @brief Evaluate the gradient of log mean length at age with respect to
   * the current natural-scale FIMS VonB-Schnute parameterization.
   * @param age Age on the natural scale.
   * @param d_log_laa_d_l1 Output derivative with respect to
   * mean_length_young.
   * @param d_log_laa_d_l2 Output derivative with respect to
   * mean_length_old.
   * @param d_log_laa_d_k Output derivative with respect to
   * von_bertalanffy_coefficient_K.
   */
  void log_length_at_age_gradient(const Type& age,
                                  Type& d_log_laa_d_l1,
                                  Type& d_log_laa_d_l2,
                                  Type& d_log_laa_d_k) const {
    ValidateLengthParameters();

    if (reference_age_for_length_1 > Type(0.0) &&
        age < reference_age_for_length_1) {
      const Type age_nonnegative = age <= Type(0.0) ? Type(0.0) : age;
      const Type ratio = age_nonnegative / reference_age_for_length_1;
      const Type mean_length = mean_length_young * ratio;
      const Type mean_length_safe = fims_math::ad_max(
          mean_length, static_cast<Type>(1e-8));

      d_log_laa_d_l1 = ratio / mean_length_safe;
      d_log_laa_d_l2 = Type(0.0);
      d_log_laa_d_k = Type(0.0);
      return;
    }

    const Type age_delta_1 = age - reference_age_for_length_1;
    const Type age_delta_2 =
        reference_age_for_length_2 - reference_age_for_length_1;

    const Type exp_num =
        fims_math::exp(-von_bertalanffy_coefficient_K * age_delta_1);
    const Type exp_den =
        fims_math::exp(-von_bertalanffy_coefficient_K * age_delta_2);

    const Type numer = Type(1.0) - exp_num;
    const Type denom = Type(1.0) - exp_den;
    const Type denom_safe = fims_math::ad_max(
        fims_math::ad_fabs(denom), static_cast<Type>(1e-8));

    const Type ratio = numer / denom_safe;
    const Type delta_length = mean_length_old - mean_length_young;
    const Type mean_length = mean_length_young + delta_length * ratio;
    const Type mean_length_safe = fims_math::ad_max(
        mean_length, static_cast<Type>(1e-8));

    const Type d_length_d_l1 = Type(1.0) - ratio;
    const Type d_length_d_l2 = ratio;

    const Type d_numer_d_k = age_delta_1 * exp_num;
    const Type d_denom_d_k = age_delta_2 * exp_den;
    const Type d_ratio_d_k =
        (d_numer_d_k * denom_safe - numer * d_denom_d_k) /
        (denom_safe * denom_safe);
    const Type d_length_d_k = delta_length * d_ratio_d_k;

    d_log_laa_d_l1 = d_length_d_l1 / mean_length_safe;
    d_log_laa_d_l2 = d_length_d_l2 / mean_length_safe;
    d_log_laa_d_k = d_length_d_k / mean_length_safe;
  }

  /**
   * @brief Evaluate the gradient of log mean length at age with respect to
   * the log-scale FIMS VonB-Schnute parameterization
   * [log(mean_length_young), log(mean_length_old),
   *  log(von_bertalanffy_coefficient_K)].
   * @param age Age on the natural scale.
   * @param d_log_laa_d_log_l1 Output derivative with respect to
   * log(mean_length_young).
   * @param d_log_laa_d_log_l2 Output derivative with respect to
   * log(mean_length_old).
   * @param d_log_laa_d_log_k Output derivative with respect to
   * log(von_bertalanffy_coefficient_K).
   */
  void log_length_at_age_logscale_gradient(const Type& age,
                                           Type& d_log_laa_d_log_l1,
                                           Type& d_log_laa_d_log_l2,
                                           Type& d_log_laa_d_log_k) const {
    Type d_log_laa_d_l1 = Type(0.0);
    Type d_log_laa_d_l2 = Type(0.0);
    Type d_log_laa_d_k = Type(0.0);

    log_length_at_age_gradient(
        age, d_log_laa_d_l1, d_log_laa_d_l2, d_log_laa_d_k);

    d_log_laa_d_log_l1 = d_log_laa_d_l1 * mean_length_young;
    d_log_laa_d_log_l2 = d_log_laa_d_l2 * mean_length_old;
    d_log_laa_d_log_k = d_log_laa_d_k * von_bertalanffy_coefficient_K;
  }

  /**
   * @brief Evaluate mean weight at age via the length-weight relationship.
   * @param age Age on the natural scale.
   * @return Mean weight at the requested age.
   */
  Type weight_at_age(const Type& age) const {
    ValidateWeightParameters();
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
