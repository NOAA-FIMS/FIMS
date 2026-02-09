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
  void SetVonBertalanffyParameters(Type L1, Type L2, Type K,
                                   Type age_L1, Type age_L2) {
    vb_.L1 = L1;
    vb_.L2 = L2;
    vb_.K = K;
    vb_.age_L1 = age_L1;
    vb_.age_L2 = age_L2;
    needs_update_ = true;
  }

  /// fixed length-weight params (phase 1)
  void SetLengthWeightParameters(Type a_wl, Type b_wl) {
    vb_.a_wl = a_wl;
    vb_.b_wl = b_wl;
    needs_update_ = true;
  }

  /// Set SD at youngest and oldest ages
  void SetLengthSdParams(Type sd1, Type sdA) {
    sd_L1_ = sd1;
    sd_LA_ = sdA;
    needs_update_ = true;
  }

  /// Compute and cache growth products
  void Prepare() override {
    if (!needs_update_) return;

    if (vb_.age_L2 < vb_.age_L1 ||
        (n_ages_ > 1 && vb_.age_L2 == vb_.age_L1)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth age_L2 must be >= age_L1");
    }
    if (vb_.K < Type(0.0)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth K must be >= 0");
    }
    if (vb_.a_wl <= Type(0.0)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth a_wl must be > 0");
    }
    if (vb_.b_wl <= Type(0.0)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth b_wl must be > 0");
    }
    if (sd_L1_ < Type(0.0) || sd_LA_ < Type(0.0)) {
      throw std::runtime_error(
          "VonBertalanffyGrowth SDgrowth must be >= 0");
    }

    // Fill mean length-at-age and sd
    const Type laa_min =
        vb_.length_at_age(age_offset_ + static_cast<Type>(0));
    const Type laa_max =
        vb_.length_at_age(age_offset_ + static_cast<Type>(n_ages_ - 1));
    if (n_ages_ > 1 && laa_max <= laa_min) {
      throw std::runtime_error(
          "VonBertalanffyGrowth LAA range invalid; check growth params");
    }
    const Type slope =
        (n_ages_ > 1)
            ? (sd_LA_ - sd_L1_) / (laa_max - laa_min)
            : Type(0.0);
    for (std::size_t y = 0; y < n_years_; ++y) {
      for (std::size_t a = 0; a < n_ages_; ++a) {
        for (std::size_t s = 0; s < n_sexes_; ++s) {
          const Type age =
              static_cast<Type>(a) + age_offset_;

          // log-scale params live upstream; laa here is natural scale
          const Type laa = vb_.length_at_age(age);
          if (laa < Type(0.0)) {
            throw std::runtime_error(
                "VonBertalanffyGrowth length_at_age < 0; check growth params");
          }
          products_.MeanLAA(y, a, s) = laa;
          products_.SdLAA(y, a, s) =
              (n_ages_ > 1)
                  ? sd_L1_ + slope * (laa - laa_min)
                  : sd_L1_;
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
  Type sd_L1_ = static_cast<Type>(3.0);
  Type sd_LA_ = static_cast<Type>(7.0);
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
