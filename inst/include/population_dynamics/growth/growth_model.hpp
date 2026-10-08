/**
 * @file growth_model.hpp
 * @brief Concrete growth model implementation.
 *
 * Wraps a selected growth functor and produces growth products in
 * (year, age, sex) space.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_MODEL_HPP
#define POPULATION_DYNAMICS_GROWTH_MODEL_HPP

#include <cstddef>
#include <stdexcept>

#include "growth_model_base.hpp"
#include "growth_products.hpp"
#include "functors/vonb_schnute.hpp"
#include "functors/vonb_traditional.hpp"
#include "../../common/def.hpp"

namespace fims_popdy {

/**
 * @brief Concrete growth model using a selected growth parameterization.
 *
 * Assumptions:
 * - single sex (n_sexes = 1)
 * - no time variation (n_years = 1)
 * - fixed mean-growth parameters
 *
 * Supported length-at-age SD methods:
 * - Von Bertalanffy (Schnute): interpolation
 * - Von Bertalanffy (traditional): interpolation or covariance-based
 *   delta method
 */
template <typename Type>
class GrowthModel : public GrowthModelBase<Type> {
  enum class GrowthParameterization { kSchnute, kTraditional };

 public:
  /**
   * @brief Construct a growth model with fixed dimensions.
   * @param n_years Number of years.
   * @param n_ages Number of ages.
   * @param n_sexes Number of sexes.
   */
  GrowthModel(std::size_t n_years, std::size_t n_ages, std::size_t n_sexes = 1)
      : n_years_(n_years),
        n_ages_(n_ages),
        n_sexes_(n_sexes),
        products_(n_years, n_ages, n_sexes) {}

  /// Set fixed VonBertalanffySchnute parameters.
  void SetVonBertalanffySchnuteParameters(Type mean_length_young,
                                          Type mean_length_old,
                                          Type growth_coefficient,
                                          Type reference_age_for_length_young,
                                          Type reference_age_for_length_old) {
    growth_parameterization_ = GrowthParameterization::kSchnute;
    vb_.mean_length_young = mean_length_young;
    vb_.mean_length_old = mean_length_old;
    vb_.growth_coefficient = growth_coefficient;
    vb_.reference_age_for_length_young = reference_age_for_length_young;
    vb_.reference_age_for_length_old = reference_age_for_length_old;
    length_sd_reference_age_young_ = reference_age_for_length_young;
    length_sd_reference_age_old_ = reference_age_for_length_old;
    needs_update_ = true;
  }

  /// Set fixed traditional Von Bertalanffy parameters.
  void SetVonBertalanffyTraditionalParameters(Type asymptotic_length,
                                              Type growth_coefficient,
                                              Type age_at_zero_length) {
    growth_parameterization_ = GrowthParameterization::kTraditional;
    traditional_vb_.asymptotic_length = asymptotic_length;
    traditional_vb_.growth_coefficient = growth_coefficient;
    traditional_vb_.age_at_zero_length = age_at_zero_length;
    needs_update_ = true;
  }

  /// fixed length-weight params (phase 1)
  void SetLengthWeightParameters(Type length_weight_a, Type length_weight_b) {
    vb_.length_weight_a = length_weight_a;
    vb_.length_weight_b = length_weight_b;
    traditional_vb_.length_weight_a = length_weight_a;
    traditional_vb_.length_weight_b = length_weight_b;
    needs_update_ = true;
  }

  /// Set the reference-point SD values used by the interpolation
  /// variability path.
  void SetLengthSdParams(Type length_at_age_sd_at_reference_age_young,
                         Type length_at_age_sd_at_reference_age_old) {
    length_at_age_sd_at_reference_age_young_ =
        length_at_age_sd_at_reference_age_young;
    length_at_age_sd_at_reference_age_old_ =
        length_at_age_sd_at_reference_age_old;
    needs_update_ = true;
  }

  /// Set the ages that anchor the interpolation variability path.
  void SetLengthSdReferenceAges(Type reference_age_for_length_young,
                                Type reference_age_for_length_old) {
    length_sd_reference_age_young_ = reference_age_for_length_young;
    length_sd_reference_age_old_ = reference_age_for_length_old;
    needs_update_ = true;
  }

  /// Set covariance values used by the traditional delta-method growth
  /// variability path.
  ///
  /// The supplied covariance must match this exact working-scale order:
  /// 1. log(asymptotic_length)
  /// 2. log(growth_coefficient)
  /// 3. age_at_zero_length
  void SetTraditionalGrowthParameterCovariance(
      Type log_asymptotic_length_variance,
      Type log_asymptotic_length_log_growth_coefficient_covariance,
      Type log_asymptotic_length_age_at_zero_length_covariance,
      Type log_growth_coefficient_variance,
      Type log_growth_coefficient_age_at_zero_length_covariance,
      Type age_at_zero_length_variance) {
    ValidateTraditionalGrowthParameterCovariance(
        log_asymptotic_length_variance,
        log_asymptotic_length_log_growth_coefficient_covariance,
        log_asymptotic_length_age_at_zero_length_covariance,
        log_growth_coefficient_variance,
        log_growth_coefficient_age_at_zero_length_covariance,
        age_at_zero_length_variance);

    log_asymptotic_length_variance_ = log_asymptotic_length_variance;
    log_asymptotic_length_log_growth_coefficient_covariance_ =
        log_asymptotic_length_log_growth_coefficient_covariance;
    log_asymptotic_length_age_at_zero_length_covariance_ =
        log_asymptotic_length_age_at_zero_length_covariance;
    log_growth_coefficient_variance_ = log_growth_coefficient_variance;
    log_growth_coefficient_age_at_zero_length_covariance_ =
        log_growth_coefficient_age_at_zero_length_covariance;
    age_at_zero_length_variance_ = age_at_zero_length_variance;
    use_delta_method_variability_ = true;
    needs_update_ = true;
  }

  /// Disable the delta-method growth variability path and return to the
  /// reference-point interpolation path.
  void ClearGrowthParameterCovariance() {
    log_asymptotic_length_variance_ = static_cast<Type>(0.0);
    log_asymptotic_length_log_growth_coefficient_covariance_ =
        static_cast<Type>(0.0);
    log_asymptotic_length_age_at_zero_length_covariance_ =
        static_cast<Type>(0.0);
    log_growth_coefficient_variance_ = static_cast<Type>(0.0);
    log_growth_coefficient_age_at_zero_length_covariance_ =
        static_cast<Type>(0.0);
    age_at_zero_length_variance_ = static_cast<Type>(0.0);
    use_delta_method_variability_ = false;
    needs_update_ = true;
  }

  /// Compute and cache growth products
  void Prepare() override {
    if (!needs_update_) return;

    if (n_ages_ == 0) {
      throw std::runtime_error("GrowthModel requires n_ages > 0");
    }
    ValidateMeanGrowthParameterization();

    Type laa_min = Type(0.0);
    Type slope = Type(0.0);

    if (!use_delta_method_variability_) {
      ValidateLengthSdReferenceAges();
      laa_min = EvaluateLengthAtAge(length_sd_reference_age_young_);
      const Type laa_max = EvaluateLengthAtAge(length_sd_reference_age_old_);
      const Type laa_delta_safe = fims_math::ad_max(
          fims_math::ad_fabs(laa_max - laa_min), static_cast<Type>(1e-8));
      slope = (n_ages_ > 1) ? (length_at_age_sd_at_reference_age_old_ -
                               length_at_age_sd_at_reference_age_young_) /
                                  laa_delta_safe
                            : Type(0.0);
    }

    // Fill mean length-at-age, sd, and mean weight-at-age.
    for (std::size_t y = 0; y < n_years_; ++y) {
      for (std::size_t a = 0; a < n_ages_; ++a) {
        for (std::size_t s = 0; s < n_sexes_; ++s) {
          const Type age = static_cast<Type>(a) + age_offset_;

          // log-scale params live upstream; laa here is natural scale
          const Type laa = EvaluateLengthAtAge(age);
          const Type sd_laa = ComputeLengthSdAtAge(age, laa, laa_min, slope);
          const Type waa = EvaluateWeightAtAge(age);

          products_.MeanLAA(y, a, s) = laa;
          products_.SdLAA(y, a, s) = sd_laa;
          products_.MeanWAA(y, a, s) = waa;
        }
      }
    }

    needs_update_ = false;
  }

  const GrowthProducts<Type>& GetProducts() const override { return products_; }

 private:
  /// Validate constraints that belong to the selected mean-growth curve.
  void ValidateMeanGrowthParameterization() const {
    if (growth_parameterization_ == GrowthParameterization::kSchnute &&
        vb_.reference_age_for_length_old <=
            vb_.reference_age_for_length_young) {
      throw std::runtime_error(
          "VonBertalanffySchnuteGrowth reference_age_for_length_old must be > "
          "reference_age_for_length_young");
    }
  }

  /// Validate reference ages used by the interpolation variability path.
  void ValidateLengthSdReferenceAges() const {
    if (length_sd_reference_age_old_ <= length_sd_reference_age_young_) {
      throw std::runtime_error(
          "GrowthModel length SD reference age old must be > reference age "
          "young");
    }
  }

  /// Evaluate mean length at age using the selected growth parameterization.
  Type EvaluateLengthAtAge(const Type& age) const {
    switch (growth_parameterization_) {
      case GrowthParameterization::kSchnute:
        return vb_.length_at_age(age);
      case GrowthParameterization::kTraditional:
        return traditional_vb_.length_at_age(age);
    }

    return vb_.length_at_age(age);
  }

  /// Evaluate mean weight at age using the selected growth parameterization.
  Type EvaluateWeightAtAge(const Type& age) const {
    switch (growth_parameterization_) {
      case GrowthParameterization::kSchnute:
        return vb_.weight_at_age(age);
      case GrowthParameterization::kTraditional:
        return traditional_vb_.weight_at_age(age);
    }

    return vb_.weight_at_age(age);
  }

  /// Validate covariance for [log(Linf), log(K), t0].
  void ValidateTraditionalGrowthParameterCovariance(
      Type log_asymptotic_length_variance,
      Type log_asymptotic_length_log_growth_coefficient_covariance,
      Type log_asymptotic_length_age_at_zero_length_covariance,
      Type log_growth_coefficient_variance,
      Type log_growth_coefficient_age_at_zero_length_covariance,
      Type age_at_zero_length_variance) const {
    if (log_asymptotic_length_variance < Type(0.0) ||
        log_growth_coefficient_variance < Type(0.0) ||
        age_at_zero_length_variance < Type(0.0)) {
      throw std::runtime_error(
          "Traditional growth parameter variances must be >= 0");
    }

    if (log_asymptotic_length_log_growth_coefficient_covariance *
            log_asymptotic_length_log_growth_coefficient_covariance >
        log_asymptotic_length_variance * log_growth_coefficient_variance) {
      throw std::runtime_error(
          "Traditional growth covariance between log_asymptotic_length and "
          "log_growth_coefficient is inconsistent with the supplied variances");
    }

    if (log_asymptotic_length_age_at_zero_length_covariance *
            log_asymptotic_length_age_at_zero_length_covariance >
        log_asymptotic_length_variance * age_at_zero_length_variance) {
      throw std::runtime_error(
          "Traditional growth covariance between log_asymptotic_length and "
          "age_at_zero_length is inconsistent with the supplied variances");
    }

    if (log_growth_coefficient_age_at_zero_length_covariance *
            log_growth_coefficient_age_at_zero_length_covariance >
        log_growth_coefficient_variance * age_at_zero_length_variance) {
      throw std::runtime_error(
          "Traditional growth covariance between log_growth_coefficient and "
          "age_at_zero_length is inconsistent with the supplied variances");
    }

    const Type determinant =
        log_asymptotic_length_variance *
            (log_growth_coefficient_variance * age_at_zero_length_variance -
             log_growth_coefficient_age_at_zero_length_covariance *
                 log_growth_coefficient_age_at_zero_length_covariance) -
        log_asymptotic_length_log_growth_coefficient_covariance *
            (log_asymptotic_length_log_growth_coefficient_covariance *
                 age_at_zero_length_variance -
             log_asymptotic_length_age_at_zero_length_covariance *
                 log_growth_coefficient_age_at_zero_length_covariance) +
        log_asymptotic_length_age_at_zero_length_covariance *
            (log_asymptotic_length_log_growth_coefficient_covariance *
                 log_growth_coefficient_age_at_zero_length_covariance -
             log_asymptotic_length_age_at_zero_length_covariance *
                 log_growth_coefficient_variance);

    if (determinant < Type(0.0)) {
      throw std::runtime_error(
          "Traditional growth parameter covariance matrix must be positive "
          "semi-definite");
    }
  }

  /// Smoothly guard log-length variance without imposing a large CV floor.
  Type SafeLogLengthVariance(const Type& log_var) const {
    if (growth_parameterization_ == GrowthParameterization::kTraditional) {
      return fims_math::ad_max(log_var, static_cast<Type>(0.0),
                               static_cast<Type>(1e-12));
    }

    return fims_math::ad_max(log_var, static_cast<Type>(0.0));
  }

  /// Compute length-at-age SD using either the interpolation path or the
  /// covariance-based delta-method path.
  Type ComputeLengthSdAtAge(const Type& age, const Type& laa,
                            const Type& laa_min, const Type& slope) const {
    if (use_delta_method_variability_) {
      const Type log_var = ComputeLogLengthVarianceAtAge(age);
      const Type log_var_safe = SafeLogLengthVariance(log_var);
      const Type sd_laa = laa * fims_math::sqrt(log_var_safe);
      return fims_math::ad_max(sd_laa, static_cast<Type>(1e-8));
    }

    const Type sd_laa =
        (n_ages_ > 1)
            ? length_at_age_sd_at_reference_age_young_ + slope * (laa - laa_min)
            : length_at_age_sd_at_reference_age_young_;

    return fims_math::ad_max(sd_laa, static_cast<Type>(1e-8));
  }

  /// Compute delta-method variance of log length at age using the active
  /// parameterization-specific covariance matrix.
  Type ComputeLogLengthVarianceAtAge(const Type& age) const {
    return ComputeTraditionalLogLengthVarianceAtAge(age);
  }

  /// Compute delta-method variance for [log(Linf), log(K), t0].
  Type ComputeTraditionalLogLengthVarianceAtAge(const Type& age) const {
    Type d_log_laa_d_log_asymptotic_length = Type(0.0);
    Type d_log_laa_d_log_growth_coefficient = Type(0.0);
    Type d_log_laa_d_age_at_zero_length = Type(0.0);

    traditional_vb_.log_length_at_age_working_scale_gradient(
        age, d_log_laa_d_log_asymptotic_length,
        d_log_laa_d_log_growth_coefficient,
        d_log_laa_d_age_at_zero_length);

    return d_log_laa_d_log_asymptotic_length *
               d_log_laa_d_log_asymptotic_length *
               log_asymptotic_length_variance_ +
           Type(2.0) * d_log_laa_d_log_asymptotic_length *
               d_log_laa_d_log_growth_coefficient *
               log_asymptotic_length_log_growth_coefficient_covariance_ +
           Type(2.0) * d_log_laa_d_log_asymptotic_length *
               d_log_laa_d_age_at_zero_length *
               log_asymptotic_length_age_at_zero_length_covariance_ +
           d_log_laa_d_log_growth_coefficient *
               d_log_laa_d_log_growth_coefficient *
               log_growth_coefficient_variance_ +
           Type(2.0) * d_log_laa_d_log_growth_coefficient *
               d_log_laa_d_age_at_zero_length *
               log_growth_coefficient_age_at_zero_length_covariance_ +
           d_log_laa_d_age_at_zero_length * d_log_laa_d_age_at_zero_length *
               age_at_zero_length_variance_;
  }

  std::size_t n_years_;
  std::size_t n_ages_;
  std::size_t n_sexes_;

  GrowthProducts<Type> products_;

  GrowthParameterization growth_parameterization_ =
      GrowthParameterization::kSchnute;
  VonBertalanffySchnuteGrowth<Type> vb_;
  VonBertalanffyTraditionalGrowth<Type> traditional_vb_;

  // Caching state
  bool needs_update_ = true;
  Type length_at_age_sd_at_reference_age_young_ = static_cast<Type>(3.0);
  Type length_at_age_sd_at_reference_age_old_ = static_cast<Type>(7.0);
  Type length_sd_reference_age_young_ = static_cast<Type>(0.0);
  Type length_sd_reference_age_old_ = static_cast<Type>(1.0);
  bool use_delta_method_variability_ = false;

  Type log_asymptotic_length_variance_ = static_cast<Type>(0.0);
  Type log_asymptotic_length_log_growth_coefficient_covariance_ =
      static_cast<Type>(0.0);
  Type log_asymptotic_length_age_at_zero_length_covariance_ =
      static_cast<Type>(0.0);
  Type log_growth_coefficient_variance_ = static_cast<Type>(0.0);
  Type log_growth_coefficient_age_at_zero_length_covariance_ =
      static_cast<Type>(0.0);
  Type age_at_zero_length_variance_ = static_cast<Type>(0.0);

  Type age_offset_ = static_cast<Type>(0.0);

 public:
  /// Set an age offset if population ages do not start at zero.
  void SetAgeOffset(Type offset) {
    age_offset_ = offset;
    needs_update_ = true;
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_GROWTH_MODEL_HPP */
