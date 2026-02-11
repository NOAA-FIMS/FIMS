/**
 * @file growth_model.hpp
 * @brief Concrete growth model implementation (Phase 1).
 *
 * Wraps a growth functor (e.g. von Bertalanffy) and produces growth products
 * in (year, age, sex) space.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_MODEL_HPP
#define POPULATION_DYNAMICS_GROWTH_MODEL_HPP

#include <cstddef>
#include <stdexcept>

#include "growth_model_base.hpp"
#include "growth_products.hpp"
#include "functors/von_bertalanffy.hpp"
#include "../../common/def.hpp"

namespace fims_popdy {

/**
 * @brief Concrete growth model using von Bertalanffy growth (Phase 1).
 *
 * Phase 1 assumptions:
 * - single sex (n_sexes = 1)
 * - no time variation (n_years = 1)
 * - fixed parameters

 */
template <typename Type>
class GrowthModel : public GrowthModelBase<Type> {
 public:
  GrowthModel(std::size_t n_years,
              std::size_t n_ages,
              std::size_t n_sexes = 1)
      : n_years_(n_years),
        n_ages_(n_ages),
        n_sexes_(n_sexes),
        products_(n_years, n_ages, n_sexes) {}

  /// Set fixed von Bertalanffy parameters (explicit reference ages).
  void SetVonBertalanffyParameters(Type length_at_ref_age_1,
                                   Type length_at_ref_age_2,
                                   Type growth_coefficient_K,
                                   Type reference_age_for_length_1,
                                   Type reference_age_for_length_2) {
    vb_.length_at_ref_age_1 = length_at_ref_age_1;
    vb_.length_at_ref_age_2 = length_at_ref_age_2;
    vb_.growth_coefficient_K = growth_coefficient_K;
    vb_.reference_age_for_length_1 = reference_age_for_length_1;
    vb_.reference_age_for_length_2 = reference_age_for_length_2;
    needs_update_ = true;
  }

  /// fixed length-weight params (phase 1)
  void SetLengthWeightParameters(Type length_weight_a, Type length_weight_b) {
    vb_.length_weight_a = length_weight_a;
    vb_.length_weight_b = length_weight_b;
    needs_update_ = true;
  }

  /// Set SD at youngest and oldest ages
  void SetLengthSdParams(Type length_at_age_sd_at_reference_age_1,
                         Type length_at_age_sd_at_reference_age_2) {
    length_at_age_sd_at_reference_age_1_ =
        length_at_age_sd_at_reference_age_1;
    length_at_age_sd_at_reference_age_2_ =
        length_at_age_sd_at_reference_age_2;
    needs_update_ = true;
  }

  /// Compute and cache growth products
  void Prepare() override {
    if (!needs_update_) return;

    if (n_ages_ == 0) {
      throw std::runtime_error(
          "VonBertalanffyGrowth requires n_ages > 0");
    }

    // Fill mean length-at-age and sd
    const Type laa_min = vb_.length_at_age(vb_.reference_age_for_length_1);
    const Type laa_max = vb_.length_at_age(vb_.reference_age_for_length_2);
    const Type laa_delta_safe = fims_math::ad_max(
        fims_math::ad_fabs(laa_max - laa_min), static_cast<Type>(1e-8));
    const Type slope =
        (n_ages_ > 1)
            ? (length_at_age_sd_at_reference_age_2_ - length_at_age_sd_at_reference_age_1_) / laa_delta_safe
            : Type(0.0);
    for (std::size_t y = 0; y < n_years_; ++y) {
      for (std::size_t a = 0; a < n_ages_; ++a) {
        for (std::size_t s = 0; s < n_sexes_; ++s) {
          const Type age =
              static_cast<Type>(a) + age_offset_;

          // log-scale params live upstream; laa here is natural scale
          const Type laa = vb_.length_at_age(age);
          products_.MeanLAA(y, a, s) = laa;
          products_.SdLAA(y, a, s) =
              (n_ages_ > 1)
                  ? length_at_age_sd_at_reference_age_1_ + slope * (laa - laa_min)
                  : length_at_age_sd_at_reference_age_1_;
          products_.MeanWAA(y, a, s) = vb_.weight_at_age(age);
        }
      }
    }

    needs_update_ = false;
  }

  const GrowthProducts<Type>& GetProducts() const override {
    return products_;
  }

  /// For testing caching behavior
 private:
  std::size_t n_years_;
  std::size_t n_ages_;
  std::size_t n_sexes_;

  GrowthProducts<Type> products_;

  // Phase 1 functor
  VonBertalanffyGrowth<Type> vb_;

  // Caching state
  bool needs_update_ = true;
  Type length_at_age_sd_at_reference_age_1_ = static_cast<Type>(3.0);
  Type length_at_age_sd_at_reference_age_2_ = static_cast<Type>(7.0);
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
