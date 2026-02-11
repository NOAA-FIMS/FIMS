/**
 * @file growth_model_adapter.hpp
 * @brief adapts product-style growth models to the GrowthBase interface
 * used by caa.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP
#define POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP

#include <cmath>
#include <memory>

#include "../../common/fims_vector.hpp"
#include "../../common/fims_math.hpp"
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

  fims::Vector<Type>& L1Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return L1_v_;
  }
  fims::Vector<Type>& L2Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return L2_v_;
  }
  fims::Vector<Type>& KVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return K_v_;
  }
  fims::Vector<Type>& AgeL1Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return age_L1_v_;
  }
  fims::Vector<Type>& AgeL2Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return age_L2_v_;
  }
  fims::Vector<Type>& AwlVector() {
    use_param_vectors_ = true;
    lw_params_set_ = true;
    return a_wl_v_;
  }
  fims::Vector<Type>& BwlVector() {
    use_param_vectors_ = true;
    lw_params_set_ = true;
    return b_wl_v_;
  }
  fims::Vector<Type>& SDgrowthVector() {
    use_param_vectors_ = true;
    return SDgrowth_v_;
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
    SyncParamsToModel();
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

    SyncParamsToModel();
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
    SyncParamsToModel();
    model_->Prepare();
    return model_->GetProducts();
  }

 private:
  // Stored in log-scale for positive parameters.
  fims::Vector<Type> L1_v_;
  fims::Vector<Type> L2_v_;
  fims::Vector<Type> K_v_;
  fims::Vector<Type> age_L1_v_;
  fims::Vector<Type> age_L2_v_;
  fims::Vector<Type> a_wl_v_;
  fims::Vector<Type> b_wl_v_;
  fims::Vector<Type> SDgrowth_v_;
  bool use_param_vectors_ = false;
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
    vb.L1 = CurrentL1();
    vb.L2 = CurrentL2();
    vb.K = CurrentK();
    vb.age_L1 = CurrentAgeL1();
    vb.age_L2 = CurrentAgeL2();
    vb.a_wl = CurrentAwl();
    vb.b_wl = CurrentBwl();
    return vb.evaluate(a);
  }

  void SyncParamsToModel() const {
    if (!model_) return;
    EnsureParamsSet();
    model_->SetVonBertalanffyParameters(CurrentL1(), CurrentL2(), CurrentK(),
                                        CurrentAgeL1(), CurrentAgeL2());
    model_->SetLengthWeightParameters(CurrentAwl(), CurrentBwl());
    model_->SetLengthSdParams(CurrentSD1(), CurrentSDA());
  }

  Type CurrentL1() const { return fims_math::exp(L1_v_[0]); }
  Type CurrentL2() const { return fims_math::exp(L2_v_[0]); }
  Type CurrentK() const { return fims_math::exp(K_v_[0]); }
  Type CurrentAgeL1() const { return age_L1_v_[0]; }
  Type CurrentAgeL2() const { return age_L2_v_[0]; }
  Type CurrentAwl() const { return fims_math::exp(a_wl_v_[0]); }
  Type CurrentBwl() const { return fims_math::exp(b_wl_v_[0]); }
  Type CurrentSD1() const { return fims_math::exp(SDgrowth_v_[0]); }
  Type CurrentSDA() const { return fims_math::exp(SDgrowth_v_[1]); }

  void EnsureParamsSet() const {
    if (!use_param_vectors_ || L1_v_.size() < 1 || L2_v_.size() < 1 ||
        K_v_.size() < 1 || age_L1_v_.size() < 1 || age_L2_v_.size() < 1 ||
        a_wl_v_.size() < 1 || b_wl_v_.size() < 1 || SDgrowth_v_.size() < 2) {
      throw std::runtime_error(
          "VonBertalanffyGrowth parameters not set");
    }
  }

};

}  // namespace fims_popdy

#endif  // POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP
