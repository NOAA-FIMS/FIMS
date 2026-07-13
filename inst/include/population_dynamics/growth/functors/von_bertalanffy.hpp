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

#include <stdexcept>
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
 * @note Users can anchor at t0 by setting reference_age_for_length_1 = 0
 * and length_at_ref_age_1 = L(0). This is valid as long as
 * reference_age_for_length_2 > reference_age_for_length_1.
 */
template <typename Type>
struct VonBertalanffyGrowth : public GrowthBase<Type> {
  Type length_at_ref_age_1 = Type(0.0); /**< expected length at reference age 1 */
  Type length_at_ref_age_2 = Type(0.0); /**< expected length at reference age 2 */
  Type growth_coefficient_K = Type(0.0); /**< Von Bertalanffy growth coefficient */
  Type reference_age_for_length_1 = Type(0.0); /**< first reference age */
  Type reference_age_for_length_2 = Type(0.0); /**< second reference age */

  Type length_weight_a = Type(0.0); /**< coefficient in W = a * L^b */
  Type length_weight_b = Type(3.0); /**< exponent in W = a * L^b */

  VonBertalanffyGrowth() : GrowthBase<Type>() {}
  virtual ~VonBertalanffyGrowth() {}

  /**
   * @brief Validate the Von Bertalanffy parameters used by length-at-age
   * calculations.
   *
   * Throws when the stored growth parameter values are incompatible with the
   * raw Von Bertalanffy length-at-age calculation.
   */
  void ValidateLengthParameters() const {
    if (growth_coefficient_K <= Type(0.0)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth growth_coefficient_K must be > 0");
    }

    if (length_at_ref_age_1 <= Type(0.0)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth length_at_ref_age_1 must be > 0");
    }

    if (length_at_ref_age_2 <= Type(0.0)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth length_at_ref_age_2 must be > 0");
    }

    if (reference_age_for_length_2 <= reference_age_for_length_1) {
      throw std::runtime_error(
          "VonBertalanffyGrowth reference_age_for_length_2 must be > "
          "reference_age_for_length_1");
    }

    if (length_at_ref_age_2 <= length_at_ref_age_1) {
      throw std::runtime_error(
          "VonBertalanffyGrowth length_at_ref_age_2 must be > "
          "length_at_ref_age_1");
    }
  }

  /**
   * @brief Validate the length-weight parameters used by weight-at-age
   * calculations.
   */
  void ValidateWeightParameters() const {
    if (length_weight_a <= Type(0.0)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth length_weight_a must be > 0");
    }

    if (length_weight_b <= Type(0.0)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth length_weight_b must be > 0");
    }
  }

  /**
   * @brief Evaluate mean length at age.
   * @param age Age on the natural scale.
   * @return Mean length at the requested age.
   */
  Type length_at_age(const Type& age) const {
    ValidateLengthParameters();

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
   * the current natural-scale FIMS Von Bertalanffy parameterization.
   * @param age Age on the natural scale.
   * @param d_log_laa_d_l1 Output derivative with respect to
   * length_at_ref_age_1.
   * @param d_log_laa_d_l2 Output derivative with respect to
   * length_at_ref_age_2.
   * @param d_log_laa_d_k Output derivative with respect to
   * growth_coefficient_K.
   */
  void log_length_at_age_gradient(const Type& age,
                                  Type& d_log_laa_d_l1,
                                  Type& d_log_laa_d_l2,
                                  Type& d_log_laa_d_k) const {
    ValidateLengthParameters();

    const Type age_delta_1 = age - reference_age_for_length_1;
    const Type age_delta_2 =
        reference_age_for_length_2 - reference_age_for_length_1;

    const Type exp_num =
        fims_math::exp(-growth_coefficient_K * age_delta_1);
    const Type exp_den =
        fims_math::exp(-growth_coefficient_K * age_delta_2);

    const Type numer = Type(1.0) - exp_num;
    const Type denom = Type(1.0) - exp_den;
    const Type denom_safe = fims_math::ad_max(
        fims_math::ad_fabs(denom), static_cast<Type>(1e-8));

    const Type ratio = numer / denom_safe;
    const Type delta_length = length_at_ref_age_2 - length_at_ref_age_1;
    const Type mean_length = length_at_ref_age_1 + delta_length * ratio;
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
   * the log-scale FIMS Von Bertalanffy parameterization
   * [log(length_at_ref_age_1), log(length_at_ref_age_2),
   *  log(growth_coefficient_K)].
   * @param age Age on the natural scale.
   * @param d_log_laa_d_log_l1 Output derivative with respect to
   * log(length_at_ref_age_1).
   * @param d_log_laa_d_log_l2 Output derivative with respect to
   * log(length_at_ref_age_2).
   * @param d_log_laa_d_log_k Output derivative with respect to
   * log(growth_coefficient_K).
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

    d_log_laa_d_log_l1 = d_log_laa_d_l1 * length_at_ref_age_1;
    d_log_laa_d_log_l2 = d_log_laa_d_l2 * length_at_ref_age_2;
    d_log_laa_d_log_k = d_log_laa_d_k * growth_coefficient_K;
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

#endif  // POPULATION_DYNAMICS_GROWTH_VON_BERTALANFFY_HPP
