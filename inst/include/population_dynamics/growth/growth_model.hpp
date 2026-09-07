/**
 * @file growth_model.hpp
 * @brief Concrete growth model implementation (Phase 1).
 *
 * Wraps a growth functor (e.g. VonBertalanffySchnute) and produces growth
 * products in (year, age, sex) space.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_MODEL_HPP
#define POPULATION_DYNAMICS_GROWTH_MODEL_HPP

#include <cstddef>
#include <stdexcept>

#include "growth_model_base.hpp"
#include "growth_products.hpp"
#include "functors/vonb_schnute.hpp"
#include "../../common/def.hpp"

namespace fims_popdy {

/**
 * @brief Concrete growth model using VonBertalanffySchnute growth (Phase 1).
 *
 * Assumptions:
 * - single sex (n_sexes = 1)
 * - no time variation (n_years = 1)
 * - fixed mean-growth parameters
 * - optional covariance-based delta-method variability
 */
template <typename Type>
class GrowthModel : public GrowthModelBase<Type> {
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

  /// Set fixed VonBertalanffySchnute parameters (explicit reference ages).
  void SetVonBertalanffySchnuteParameters(Type mean_length_young,
                                          Type mean_length_old,
                                          Type growth_coefficient,
                                          Type reference_age_for_length_young,
                                          Type reference_age_for_length_old) {
    vb_.mean_length_young = mean_length_young;
    vb_.mean_length_old = mean_length_old;
    vb_.growth_coefficient = growth_coefficient;
    vb_.reference_age_for_length_young = reference_age_for_length_young;
    vb_.reference_age_for_length_old = reference_age_for_length_old;
    needs_update_ = true;
  }

  /// fixed length-weight params (phase 1)
  void SetLengthWeightParameters(Type length_weight_a, Type length_weight_b) {
    vb_.length_weight_a = length_weight_a;
    vb_.length_weight_b = length_weight_b;
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

  /// Set the covariance values used by the delta-method growth
  /// variability path.
  ///
  /// The supplied covariance must match the current log-scale FIMS
  /// VonBertalanffySchnute estimation parameterization in this exact order:
  /// 1. log(mean_length_young)
  /// 2. log(mean_length_old)
  /// 3. log(growth_coefficient)
  void SetGrowthParameterCovariance(
      Type mean_length_young_variance,
      Type mean_length_young_mean_length_old_covariance,
      Type mean_length_young_growth_coefficient_covariance,
      Type mean_length_old_variance,
      Type mean_length_old_growth_coefficient_covariance,
      Type growth_coefficient_variance) {
    ValidateGrowthParameterCovariance(
        mean_length_young_variance,
        mean_length_young_mean_length_old_covariance,
        mean_length_young_growth_coefficient_covariance,
        mean_length_old_variance, mean_length_old_growth_coefficient_covariance,
        growth_coefficient_variance);

    mean_length_young_variance_ = mean_length_young_variance;
    mean_length_young_mean_length_old_covariance_ =
        mean_length_young_mean_length_old_covariance;
    mean_length_young_growth_coefficient_covariance_ =
        mean_length_young_growth_coefficient_covariance;
    mean_length_old_variance_ = mean_length_old_variance;
    mean_length_old_growth_coefficient_covariance_ =
        mean_length_old_growth_coefficient_covariance;
    growth_coefficient_variance_ = growth_coefficient_variance;
    use_delta_method_variability_ = true;
    needs_update_ = true;
  }

  /// Disable the delta-method growth variability path and return to the
  /// reference-point interpolation path.
  void ClearGrowthParameterCovariance() {
    mean_length_young_variance_ = static_cast<Type>(0.0);
    mean_length_young_mean_length_old_covariance_ = static_cast<Type>(0.0);
    mean_length_young_growth_coefficient_covariance_ = static_cast<Type>(0.0);
    mean_length_old_variance_ = static_cast<Type>(0.0);
    mean_length_old_growth_coefficient_covariance_ = static_cast<Type>(0.0);
    growth_coefficient_variance_ = static_cast<Type>(0.0);
    use_delta_method_variability_ = false;
    needs_update_ = true;
  }

  /// Compute and cache growth products
  void Prepare() override {
    if (!needs_update_) return;

    if (n_ages_ == 0) {
      throw std::runtime_error(
          "VonBertalanffySchnuteGrowth requires n_ages > 0");
    }
    if (vb_.reference_age_for_length_old <=
        vb_.reference_age_for_length_young) {
      throw std::runtime_error(
          "VonBertalanffySchnuteGrowth reference_age_for_length_old must be > "
          "reference_age_for_length_young");
    }

    Type laa_min = Type(0.0);
    Type slope = Type(0.0);

    if (!use_delta_method_variability_) {
      laa_min = vb_.length_at_age(vb_.reference_age_for_length_young);
      const Type laa_max = vb_.length_at_age(vb_.reference_age_for_length_old);
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
          const Type laa = vb_.length_at_age(age);
          const Type sd_laa = ComputeLengthSdAtAge(age, laa, laa_min, slope);
          const Type waa = vb_.weight_at_age(age);

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
  /// Validate that the supplied 3-parameter covariance matrix on the
  /// log-scale FIMS estimation parameterization is usable for the
  /// delta-method growth variability path.
  void ValidateGrowthParameterCovariance(
      Type mean_length_young_variance,
      Type mean_length_young_mean_length_old_covariance,
      Type mean_length_young_growth_coefficient_covariance,
      Type mean_length_old_variance,
      Type mean_length_old_growth_coefficient_covariance,
      Type growth_coefficient_variance) const {
    if (mean_length_young_variance < Type(0.0) ||
        mean_length_old_variance < Type(0.0) ||
        growth_coefficient_variance < Type(0.0)) {
      throw std::runtime_error("Growth parameter variances must be >= 0");
    }

    if (mean_length_young_mean_length_old_covariance *
            mean_length_young_mean_length_old_covariance >
        mean_length_young_variance * mean_length_old_variance) {
      throw std::runtime_error(
          "Growth covariance between mean_length_young and "
          "mean_length_old is inconsistent with the supplied variances");
    }

    if (mean_length_young_growth_coefficient_covariance *
            mean_length_young_growth_coefficient_covariance >
        mean_length_young_variance * growth_coefficient_variance) {
      throw std::runtime_error(
          "Growth covariance between mean_length_young and "
          "growth_coefficient is inconsistent with the supplied variances");
    }

    if (mean_length_old_growth_coefficient_covariance *
            mean_length_old_growth_coefficient_covariance >
        mean_length_old_variance * growth_coefficient_variance) {
      throw std::runtime_error(
          "Growth covariance between mean_length_old and "
          "growth_coefficient is inconsistent with the supplied variances");
    }

    const Type determinant =
        mean_length_young_variance *
            (mean_length_old_variance * growth_coefficient_variance -
             mean_length_old_growth_coefficient_covariance *
                 mean_length_old_growth_coefficient_covariance) -
        mean_length_young_mean_length_old_covariance *
            (mean_length_young_mean_length_old_covariance *
                 growth_coefficient_variance -
             mean_length_young_growth_coefficient_covariance *
                 mean_length_old_growth_coefficient_covariance) +
        mean_length_young_growth_coefficient_covariance *
            (mean_length_young_mean_length_old_covariance *
                 mean_length_old_growth_coefficient_covariance -
             mean_length_young_growth_coefficient_covariance *
                 mean_length_old_variance);

    if (determinant < Type(0.0)) {
      throw std::runtime_error(
          "Growth parameter covariance matrix must be positive "
          "semi-definite");
    }
  }

  /// Compute length-at-age SD using either the interpolation path or the
  /// covariance-based delta-method path.
  Type ComputeLengthSdAtAge(const Type& age, const Type& laa,
                            const Type& laa_min, const Type& slope) const {
    if (use_delta_method_variability_) {
      const Type log_var = ComputeLogLengthVarianceAtAge(age);
      const Type log_var_safe =
          fims_math::ad_max(log_var, static_cast<Type>(0.0));
      const Type sd_laa = laa * fims_math::sqrt(log_var_safe);
      return fims_math::ad_max(sd_laa, static_cast<Type>(1e-8));
    }

    const Type sd_laa =
        (n_ages_ > 1)
            ? length_at_age_sd_at_reference_age_young_ + slope * (laa - laa_min)
            : length_at_age_sd_at_reference_age_young_;

    return fims_math::ad_max(sd_laa, static_cast<Type>(1e-8));
  }

  /// Compute delta-method variance of log length at age using the current
  /// 3-parameter covariance matrix.
  Type ComputeLogLengthVarianceAtAge(const Type& age) const {
    Type d_log_laa_d_log_length_young = Type(0.0);
    Type d_log_laa_d_log_length_old = Type(0.0);
    Type d_log_laa_d_log_k = Type(0.0);

    vb_.log_length_at_age_logscale_gradient(age, d_log_laa_d_log_length_young,
                                            d_log_laa_d_log_length_old,
                                            d_log_laa_d_log_k);

    return d_log_laa_d_log_length_young * d_log_laa_d_log_length_young *
               mean_length_young_variance_ +
           Type(2.0) * d_log_laa_d_log_length_young *
               d_log_laa_d_log_length_old *
               mean_length_young_mean_length_old_covariance_ +
           Type(2.0) * d_log_laa_d_log_length_young * d_log_laa_d_log_k *
               mean_length_young_growth_coefficient_covariance_ +
           d_log_laa_d_log_length_old * d_log_laa_d_log_length_old *
               mean_length_old_variance_ +
           Type(2.0) * d_log_laa_d_log_length_old * d_log_laa_d_log_k *
               mean_length_old_growth_coefficient_covariance_ +
           d_log_laa_d_log_k * d_log_laa_d_log_k * growth_coefficient_variance_;
  }

  std::size_t n_years_;
  std::size_t n_ages_;
  std::size_t n_sexes_;

  GrowthProducts<Type> products_;

  // Phase 1 functor
  VonBertalanffySchnuteGrowth<Type> vb_;

  // Caching state
  bool needs_update_ = true;
  Type length_at_age_sd_at_reference_age_young_ = static_cast<Type>(3.0);
  Type length_at_age_sd_at_reference_age_old_ = static_cast<Type>(7.0);
  bool use_delta_method_variability_ = false;
  Type mean_length_young_variance_ = static_cast<Type>(0.0);
  Type mean_length_young_mean_length_old_covariance_ = static_cast<Type>(0.0);
  Type mean_length_young_growth_coefficient_covariance_ =
      static_cast<Type>(0.0);
  Type mean_length_old_variance_ = static_cast<Type>(0.0);
  Type mean_length_old_growth_coefficient_covariance_ = static_cast<Type>(0.0);
  Type growth_coefficient_variance_ = static_cast<Type>(0.0);
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
