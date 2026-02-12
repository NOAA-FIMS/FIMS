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

  fims::Vector<Type>& LengthAtRefAge1Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return length_at_ref_age_1_vector_;
  }
  fims::Vector<Type>& LengthAtRefAge2Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return length_at_ref_age_2_vector_;
  }
  fims::Vector<Type>& GrowthCoefficientKVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return growth_coefficient_K_vector_;
  }
  fims::Vector<Type>& ReferenceAgeForLength1Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return reference_age_for_length_1_vector_;
  }
  fims::Vector<Type>& ReferenceAgeForLength2Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    return reference_age_for_length_2_vector_;
  }
  fims::Vector<Type>& LengthWeightAVector() {
    use_param_vectors_ = true;
    lw_params_set_ = true;
    return length_weight_a_vector_;
  }
  fims::Vector<Type>& LengthWeightBVector() {
    use_param_vectors_ = true;
    lw_params_set_ = true;
    return length_weight_b_vector_;
  }
  fims::Vector<Type>& LengthAtAgeSdAtRefAgesVector() {
    use_param_vectors_ = true;
    return length_at_age_sd_at_ref_ages_vector_;
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
    EnsureParamsSet();
    const Type ref_age_1 = CurrentReferenceAgeForLength1();
    const Type ref_age_2 = CurrentReferenceAgeForLength2();
    if (ref_age_2 <= ref_age_1) {
      throw std::runtime_error(
          "VonBertalanffyGrowth reference_age_for_length_2 must be > "
          "reference_age_for_length_1");
    }

    if (!model_) {
      return EvaluateWithFunctor(a);
    }

    SyncParamsToModel();
    model_->Prepare();
    const auto& p = model_->GetProducts();
    const double offset = age_offset_set_ ? age_offset_ : 0.0;
    const double age_index_raw = a_round - offset;
    const double age_index_round = std::round(age_index_raw);
    if (std::fabs(age_index_raw - age_index_round) <= tol &&
        age_index_round >= 0.0) {
      const std::size_t age_index =
          static_cast<std::size_t>(age_index_round);
      if (age_index < p.n_ages) {
        return p.MeanWAA(0, age_index, 0);
      }
    }

    // Outside cached model age bins: fall back to direct functor evaluation.
    return EvaluateWithFunctor(a);
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
  fims::Vector<Type> length_at_ref_age_1_vector_;
  fims::Vector<Type> length_at_ref_age_2_vector_;
  fims::Vector<Type> growth_coefficient_K_vector_;
  fims::Vector<Type> reference_age_for_length_1_vector_;
  fims::Vector<Type> reference_age_for_length_2_vector_;
  fims::Vector<Type> length_weight_a_vector_;
  fims::Vector<Type> length_weight_b_vector_;
  fims::Vector<Type> length_at_age_sd_at_ref_ages_vector_;
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
    vb.length_at_ref_age_1 = CurrentLengthAtRefAge1();
    vb.length_at_ref_age_2 = CurrentLengthAtRefAge2();
    vb.growth_coefficient_K = CurrentGrowthCoefficientK();
    vb.reference_age_for_length_1 = CurrentReferenceAgeForLength1();
    vb.reference_age_for_length_2 = CurrentReferenceAgeForLength2();
    vb.length_weight_a = CurrentLengthWeightA();
    vb.length_weight_b = CurrentLengthWeightB();
    return vb.evaluate(a);
  }

  void SyncParamsToModel() const {
    if (!model_) return;
    EnsureParamsSet();
    model_->SetVonBertalanffyParameters(CurrentLengthAtRefAge1(), CurrentLengthAtRefAge2(), CurrentGrowthCoefficientK(),
                                        CurrentReferenceAgeForLength1(), CurrentReferenceAgeForLength2());
    model_->SetLengthWeightParameters(CurrentLengthWeightA(), CurrentLengthWeightB());
    model_->SetLengthSdParams(CurrentLengthAtAgeSdAtReferenceAge1(), CurrentLengthAtAgeSdAtReferenceAge2());
  }

  Type CurrentLengthAtRefAge1() const { return fims_math::exp(length_at_ref_age_1_vector_[0]); }
  Type CurrentLengthAtRefAge2() const { return fims_math::exp(length_at_ref_age_2_vector_[0]); }
  Type CurrentGrowthCoefficientK() const { return fims_math::exp(growth_coefficient_K_vector_[0]); }
  Type CurrentReferenceAgeForLength1() const {
    return reference_age_for_length_1_vector_[0];
  }
  Type CurrentReferenceAgeForLength2() const {
    return reference_age_for_length_2_vector_[0];
  }
  Type CurrentLengthWeightA() const { return fims_math::exp(length_weight_a_vector_[0]); }
  Type CurrentLengthWeightB() const { return fims_math::exp(length_weight_b_vector_[0]); }
  Type CurrentLengthAtAgeSdAtReferenceAge1() const { return fims_math::exp(length_at_age_sd_at_ref_ages_vector_[0]); }
  Type CurrentLengthAtAgeSdAtReferenceAge2() const { return fims_math::exp(length_at_age_sd_at_ref_ages_vector_[1]); }

  void EnsureParamsSet() const {
    if (!use_param_vectors_ || length_at_ref_age_1_vector_.size() < 1 || length_at_ref_age_2_vector_.size() < 1 ||
        growth_coefficient_K_vector_.size() < 1 ||
        reference_age_for_length_1_vector_.size() < 1 ||
        reference_age_for_length_2_vector_.size() < 1 ||
        length_weight_a_vector_.size() < 1 || length_weight_b_vector_.size() < 1 || length_at_age_sd_at_ref_ages_vector_.size() < 2) {
      throw std::runtime_error(
          "VonBertalanffyGrowth parameters not set");
    }
  }

};

}  // namespace fims_popdy

#endif  // POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP
