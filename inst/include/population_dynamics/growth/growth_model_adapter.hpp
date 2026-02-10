/**
 * @file growth_model_adapter.hpp
 * @brief adapts product-style growth models to the GrowthBase interface
 * used by caa.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP
#define POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP

#include <cmath>
#include <memory>

#include "growth_model.hpp"
#include "functors/growth_base.hpp"
#include "functors/von_bertalanffy.hpp"

namespace fims_popdy {

/**
 * @brief vb growth model adapter so caa can keep calling evaluate(age).
 */
template <typename Type>
class VonBertalanffyGrowthModelAdapter : public GrowthBase<Type> {
 public:
  VonBertalanffyGrowthModelAdapter() : GrowthBase<Type>() {}

  void SetVonBertalanffyParameters(Type L1, Type L2, Type K,
                                   Type age_L1, Type age_L2) {
    L1_ = L1;
    L2_ = L2;
    K_ = K;
    age_L1_ = age_L1;
    age_L2_ = age_L2;
    vb_params_set_ = true;
  }

  void SetLengthWeightParameters(Type a_wl, Type b_wl) {
    a_wl_ = a_wl;
    b_wl_ = b_wl;
    lw_params_set_ = true;
  }

  void SetLengthSdParams(Type sd1, Type sdA) {
    sd_L1_ = sd1;
    sd_LA_ = sdA;
  }

  // If ages do not start at zero, set the minimum age here so we can
  // translate age values to zero-based indices for cached products.
  void SetAgeOffset(double min_age) {
    age_offset_ = min_age;
    age_offset_set_ = true;
    if (model_) {
      model_->SetAgeOffset(static_cast<Type>(age_offset_));
    }
  }

  void Initialize(std::size_t n_years,
                  std::size_t n_ages,
                  std::size_t n_sexes = 1) {
    EnsureParamsSet();
    n_years_ = n_years;
    n_ages_ = n_ages;
    n_sexes_ = n_sexes;
    model_ = std::make_shared<GrowthModel<Type>>(n_years, n_ages, n_sexes);
    model_->SetVonBertalanffyParameters(L1_, L2_, K_, age_L1_, age_L2_);
    model_->SetLengthWeightParameters(a_wl_, b_wl_);
    model_->SetLengthSdParams(sd_L1_, sd_LA_);
    if (age_offset_set_) {
      model_->SetAgeOffset(static_cast<Type>(age_offset_));
    }
  }

  virtual const Type evaluate(const double& a) const override {
    if (a < 0.0) {
      throw std::runtime_error(
          "Negative age not supported");
    }
    const double a_round = std::round(a);
    const double tol = 1e-8;
    if (std::fabs(a - a_round) > tol) {
      throw std::runtime_error(
          "Non-integer age not supported yet");
    }
    if (!model_) {
      return EvaluateWithFunctor(a);
    }

    model_->Prepare();
    const auto& p = model_->GetProducts();
    const double offset = age_offset_set_ ? age_offset_ : 0.0;
    const double age_index_raw = a_round - offset;
    if (age_index_raw < 0.0) {
      throw std::runtime_error(
          "Age below model minimum");
    }
    const double age_index_round = std::round(age_index_raw);
    if (std::fabs(age_index_raw - age_index_round) > tol) {
      throw std::runtime_error(
          "Age not aligned with model age bins");
    }
    const std::size_t age_index =
        static_cast<std::size_t>(age_index_round);
    if (age_index >= p.n_ages) {
      throw std::runtime_error(
          "Age out of range for growth products");
    }
    return p.MeanWAA(0, age_index, 0);
  }

  /**
   * @brief Prepare and return growth products for reporting.
   */
  const GrowthProducts<Type>& GetProductsForReporting() {
    if (!model_) {
      if (n_ages_ == 0) {
        throw std::runtime_error(
            "Growth model not initialized; n_ages is 0");
      }
      Initialize(n_years_ == 0 ? 1 : n_years_, n_ages_,
                 n_sexes_ == 0 ? 1 : n_sexes_);
    }
    model_->Prepare();
    return model_->GetProducts();
  }

 private:
  Type L1_ = Type(0.0);
  Type L2_ = Type(0.0);
  Type K_ = Type(0.0);
  Type a_wl_ = Type(0.0);
  Type b_wl_ = Type(3.0);
  Type sd_L1_ = Type(3.0);
  Type sd_LA_ = Type(7.0);
  Type age_L1_ = Type(0.0);
  Type age_L2_ = Type(0.0);
  std::size_t n_years_ = 0;
  std::size_t n_ages_ = 0;
  std::size_t n_sexes_ = 1;
  double age_offset_ = 0.0;
  bool age_offset_set_ = false;
  bool vb_params_set_ = false;
  bool lw_params_set_ = false;

  mutable std::shared_ptr<GrowthModel<Type>> model_;

  Type EvaluateWithFunctor(const double& a) const {
    EnsureParamsSet();
    fims_popdy::VonBertalanffyGrowth<Type> vb;
    vb.L1 = L1_;
    vb.L2 = L2_;
    vb.K = K_;
    vb.age_L1 = age_L1_;
    vb.age_L2 = age_L2_;
    vb.a_wl = a_wl_;
    vb.b_wl = b_wl_;
    return vb.evaluate(a);
  }

  void EnsureParamsSet() const {
    if (!vb_params_set_ || !lw_params_set_) {
      throw std::runtime_error(
          "VonBertalanffyGrowth parameters not set");
    }
  }

};

}  // namespace fims_popdy

#endif  // POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP
