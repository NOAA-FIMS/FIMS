/**
 * @file growth_model_adapter.hpp
 * @brief adapts product-style growth models to the GrowthBase interface
 * used by caa.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP
#define POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP

#include <cmath>
#include <memory>
#include <stdexcept>

#include "../../common/fims_vector.hpp"
#include "../../common/fims_math.hpp"
#include "growth_model.hpp"
#include "functors/growth_base.hpp"
#include "functors/von_bertalanffy.hpp"

namespace fims_popdy {

/**
 * @brief Generic capability interface for growth models that can feed the
 * growth-derived ALK / bin-based WAA path in catch-at-age.
 */
template <typename Type>
class GrowthDerivedObservationBase : public GrowthBase<Type> {
 public:
  GrowthDerivedObservationBase() : GrowthBase<Type>() {}
  virtual ~GrowthDerivedObservationBase() = default;

  /**
   * @brief Set the minimum modeled age used to translate cached age indices.
   * @param min_age Minimum age on the natural scale.
   */
  virtual void SetAgeOffset(double min_age) = 0;

  /**
   * @brief Initialize any cached growth products.
   * @param n_years Number of modeled years.
   * @param n_ages Number of modeled ages.
   * @param n_sexes Number of modeled sexes.
   */
  virtual void Initialize(std::size_t n_years,
                          std::size_t n_ages,
                          std::size_t n_sexes = 1) = 0;

  /**
   * @brief Report whether this growth object can support the dynamic ALK path.
   * @return True when growth-derived ALK calculations are available.
   */
  virtual bool SupportsGrowthDerivedALK() const = 0;

  /**
   * @brief Prepare growth products for the current model state.
   */
  virtual void PrepareGrowthProducts() = 0;

  /**
   * @brief Return prepared growth products without triggering preparation.
   * @return Pointer to prepared growth products, or nullptr if unavailable.
   */
  virtual const GrowthProducts<Type>* TryGetPreparedGrowthProducts() const = 0;

  /**
   * @brief Evaluate weight at a supplied length.
   * @param length Length on the natural scale.
   * @return Weight on the natural scale.
   */
  virtual Type EvaluateWeightAtLength(const Type& length) const = 0;
};

/**
 * @brief VonB growth model adapter implementing the generic growth-derived
 * observation capability for catch-at-age.
 */
template <typename Type>
class VonBertalanffyGrowthModelAdapter : public GrowthDerivedObservationBase<Type> {
 public:
  VonBertalanffyGrowthModelAdapter() : GrowthDerivedObservationBase<Type>() {}

  enum class LengthReferenceParameterization {
    kEstimatedL1EstimatedGap = 0,
    kConstantL1EstimatedGap,
    kEstimatedL1BelowConstantL2,
    kBothConstant
  };

  void UseEstimatedLengthAtRefAge1WithEstimatedGap() {
    length_reference_parameterization_ =
        LengthReferenceParameterization::kEstimatedL1EstimatedGap;
    growth_products_prepared_ = false;
  }

  void UseConstantLengthAtRefAge1WithEstimatedGap(
      Type constant_length_at_ref_age_1) {
    length_reference_parameterization_ =
        LengthReferenceParameterization::kConstantL1EstimatedGap;
    constant_length_at_ref_age_1_ = constant_length_at_ref_age_1;
    growth_products_prepared_ = false;
  }

  void UseEstimatedLengthAtRefAge1BelowConstantLengthAtRefAge2(
      Type constant_length_at_ref_age_2) {
    length_reference_parameterization_ =
        LengthReferenceParameterization::kEstimatedL1BelowConstantL2;
    constant_length_at_ref_age_2_ = constant_length_at_ref_age_2;
    growth_products_prepared_ = false;
  }

  void UseConstantReferenceLengths(Type constant_length_at_ref_age_1,
                                   Type constant_length_at_ref_age_2) {
    length_reference_parameterization_ =
        LengthReferenceParameterization::kBothConstant;
    constant_length_at_ref_age_1_ = constant_length_at_ref_age_1;
    constant_length_at_ref_age_2_ = constant_length_at_ref_age_2;
    growth_products_prepared_ = false;
  }

  /**
   * @brief Access the working-scale storage for the first reference-length parameter.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LengthAtRefAge1Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return length_at_ref_age_1_vector_;
  }

  /**
   * @brief Access the log-scale length-at-reference-age-2 parameter vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LengthAtRefAge2Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return length_at_ref_age_2_vector_;
  }

  /**
   * @brief Access the log-scale Von Bertalanffy growth coefficient vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& GrowthCoefficientKVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return growth_coefficient_K_vector_;
  }

  /**
   * @brief Access the first reference-age vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& ReferenceAgeForLength1Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return reference_age_for_length_1_vector_;
  }

  /**
   * @brief Access the second reference-age vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& ReferenceAgeForLength2Vector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return reference_age_for_length_2_vector_;
  }

  /**
   * @brief Access the log-scale length-weight-a vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LengthWeightAVector() {
    use_param_vectors_ = true;
    lw_params_set_ = true;
    growth_products_prepared_ = false;
    return length_weight_a_vector_;
  }

  /**
   * @brief Access the log-scale length-weight-b vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LengthWeightBVector() {
    use_param_vectors_ = true;
    lw_params_set_ = true;
    growth_products_prepared_ = false;
    return length_weight_b_vector_;
  }

  /**
   * @brief Access the log-scale length-at-age SD vector at the two reference ages.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LengthAtAgeSdAtRefAgesVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return length_at_age_sd_at_ref_ages_vector_;
  }

  /**
   * @brief Access the log-SD vector for length_at_ref_age_1.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogSdLengthAtRefAge1Vector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return log_sd_length_at_ref_age_1_vector_;
  }

  /**
   * @brief Access the log-SD vector for length_at_ref_age_2.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogSdLengthAtRefAge2Vector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return log_sd_length_at_ref_age_2_vector_;
  }

  /**
   * @brief Access the log-SD vector for growth_coefficient_K.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogSdGrowthCoefficientKVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return log_sd_growth_coefficient_K_vector_;
  }

  /**
   * @brief Access the transformed correlation vector for
   * length_at_ref_age_1 and length_at_ref_age_2.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogitCorrLengthAtRefAge1LengthAtRefAge2Vector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return logit_corr_length_at_ref_age_1_length_at_ref_age_2_vector_;
  }

  /**
   * @brief Access the transformed correlation vector for
   * length_at_ref_age_1 and growth_coefficient_K.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogitCorrLengthAtRefAge1KVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return logit_corr_length_at_ref_age_1_k_vector_;
  }

  /**
   * @brief Access the transformed correlation vector for
   * length_at_ref_age_2 and growth_coefficient_K.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogitCorrLengthAtRefAge2KVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return logit_corr_length_at_ref_age_2_k_vector_;
  }

  /**
   * @brief Set the minimum modeled age used by cached growth products.
   * @param min_age Minimum age on the natural scale.
   */
  void SetAgeOffset(double min_age) override {
    age_offset_ = min_age;
    age_offset_set_ = true;
    growth_products_prepared_ = false;
    if (model_) {
      model_->SetAgeOffset(static_cast<Type>(age_offset_));
    }
  }

  /**
   * @brief Initialize the backing growth model and cache dimensions.
   * @param n_years Number of modeled years.
   * @param n_ages Number of modeled ages.
   * @param n_sexes Number of modeled sexes.
   */
  void Initialize(std::size_t n_years,
                  std::size_t n_ages,
                  std::size_t n_sexes = 1) override {
    EnsureParamsSet();
    if (n_sexes != 1) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter currently supports n_sexes == 1");
    }
    n_years_ = n_years;
    n_ages_ = n_ages;
    n_sexes_ = n_sexes;
    growth_products_prepared_ = false;
    model_ = std::make_shared<GrowthModel<Type>>(n_years, n_ages, n_sexes);
    SyncParamsToModel();
    if (age_offset_set_) {
      model_->SetAgeOffset(static_cast<Type>(age_offset_));
    }
  }

  /**
   * @brief Report that the adapter supports growth-derived ALK calculations.
   * @return Always true for this adapter.
   */
  bool SupportsGrowthDerivedALK() const override { return true; }

  virtual const Type evaluate(int year, const double& a) override {
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
      return EvaluateWithFunctor(year, a);
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
        const std::size_t year_index =
            (year >= 0 && static_cast<std::size_t>(year) < p.n_years)
                ? static_cast<std::size_t>(year)
                : static_cast<std::size_t>(0);
        return p.MeanWAA(year_index, age_index, 0);
      }
    }

    // Outside cached model age bins: fall back to direct functor evaluation.
    return EvaluateWithFunctor(year, a);
  }

  /**
   * @brief Prepare growth products for the current model state.
   */
  void PrepareGrowthProducts() override {
    growth_products_prepared_ = false;
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
    growth_products_prepared_ = true;
  }

  /**
   * @brief Return prepared growth products without triggering preparation.
   * @return Pointer to prepared growth products, or nullptr if unavailable.
   */
  const GrowthProducts<Type>* TryGetPreparedGrowthProducts() const override {
    if (!model_ || !growth_products_prepared_) {
      return nullptr;
    }
    return &(model_->GetProducts());
  }

  /**
   * @brief Evaluate the length-weight relationship at a supplied length.
   *
   * This is used by fleet-level derived quantities that need a bin-based
   * expectation over the same length bins used in the dynamic ALK path.
   *
   * @param length Length on the natural scale.
   * @return Weight on the natural scale.
   */
  Type EvaluateWeightAtLength(const Type& length) const override {
    EnsureParamsSet();
    const Type length_safe =
        fims_math::ad_max(length, static_cast<Type>(1e-8));
    return CurrentLengthWeightA() *
           fims_math::pow(length_safe, CurrentLengthWeightB());
  }

 private:
  // Stored parameter vectors on their working scales. Positive parameters
  // use log scale, reference ages stay on the natural scale, and
  // correlation terms use transformed working-scale values.
  fims::Vector<Type> length_at_ref_age_1_vector_;
  fims::Vector<Type> length_at_ref_age_2_vector_;
  fims::Vector<Type> growth_coefficient_K_vector_;
  fims::Vector<Type> reference_age_for_length_1_vector_;
  fims::Vector<Type> reference_age_for_length_2_vector_;
  fims::Vector<Type> length_weight_a_vector_;
  fims::Vector<Type> length_weight_b_vector_;
  fims::Vector<Type> length_at_age_sd_at_ref_ages_vector_;
  fims::Vector<Type> log_sd_length_at_ref_age_1_vector_;
  fims::Vector<Type> log_sd_length_at_ref_age_2_vector_;
  fims::Vector<Type> log_sd_growth_coefficient_K_vector_;
  fims::Vector<Type> logit_corr_length_at_ref_age_1_length_at_ref_age_2_vector_;
  fims::Vector<Type> logit_corr_length_at_ref_age_1_k_vector_;
  fims::Vector<Type> logit_corr_length_at_ref_age_2_k_vector_;
  bool use_param_vectors_ = false;
  LengthReferenceParameterization length_reference_parameterization_ =
      LengthReferenceParameterization::kEstimatedL1EstimatedGap;
  Type constant_length_at_ref_age_1_ = Type(0.0);
  Type constant_length_at_ref_age_2_ = Type(0.0);
  std::size_t n_years_ = 0;
  std::size_t n_ages_ = 0;
  std::size_t n_sexes_ = 1;
  double age_offset_ = 0.0;
  bool age_offset_set_ = false;
  bool vb_params_set_ = false;
  bool lw_params_set_ = false;
  bool growth_products_prepared_ = false;

  mutable std::shared_ptr<GrowthModel<Type>> model_;

  Type EvaluateWithFunctor(int year, const double& a) const {
    EnsureParamsSet();
    fims_popdy::VonBertalanffyGrowth<Type> vb;
    vb.length_at_ref_age_1 = CurrentLengthAtRefAge1();
    vb.length_at_ref_age_2 = CurrentLengthAtRefAge2();
    vb.growth_coefficient_K = CurrentGrowthCoefficientK();
    vb.reference_age_for_length_1 = CurrentReferenceAgeForLength1();
    vb.reference_age_for_length_2 = CurrentReferenceAgeForLength2();
    vb.length_weight_a = CurrentLengthWeightA();
    vb.length_weight_b = CurrentLengthWeightB();
    return vb.evaluate(year, a);
  }

  void SyncParamsToModel() const {
    if (!model_) return;
    EnsureParamsSet();

    model_->SetVonBertalanffyParameters(
        CurrentLengthAtRefAge1(), CurrentLengthAtRefAge2(),
        CurrentGrowthCoefficientK(), CurrentReferenceAgeForLength1(),
        CurrentReferenceAgeForLength2());
    model_->SetLengthWeightParameters(CurrentLengthWeightA(),
                                      CurrentLengthWeightB());

    if (HasInterpolationSdInputs()) {
      model_->SetLengthSdParams(CurrentLengthAtAgeSdAtReferenceAge1(),
                                CurrentLengthAtAgeSdAtReferenceAge2());
    }

    if (HasStructuredDeltaMethodInputs()) {
      model_->SetGrowthParameterCovariance(
          CurrentLengthAtRefAge1Variance(),
          CurrentLengthAtRefAge1LengthAtRefAge2Covariance(),
          CurrentLengthAtRefAge1KCovariance(),
          CurrentLengthAtRefAge2Variance(),
          CurrentLengthAtRefAge2KCovariance(),
          CurrentGrowthCoefficientKVariance());
    } else {
      model_->ClearGrowthParameterCovariance();
    }
  }

  Type CurrentLengthAtRefAge1Storage() const {
    return fims_math::exp(length_at_ref_age_1_vector_[0]);
  }

  Type CurrentLengthAtRefAge2Storage() const {
    return fims_math::exp(length_at_ref_age_2_vector_[0]);
  }

  Type CurrentLengthAtRefAge1() const {
    switch (length_reference_parameterization_) {
      case LengthReferenceParameterization::kEstimatedL1EstimatedGap:
        return CurrentLengthAtRefAge1Storage();

      case LengthReferenceParameterization::kConstantL1EstimatedGap:
        return constant_length_at_ref_age_1_;

      case LengthReferenceParameterization::kEstimatedL1BelowConstantL2:
        return fims_math::inv_logit(
            static_cast<Type>(0.0),
            constant_length_at_ref_age_2_,
            length_at_ref_age_1_vector_[0]);

      case LengthReferenceParameterization::kBothConstant:
        return constant_length_at_ref_age_1_;
    }

    return CurrentLengthAtRefAge1Storage();
  }

  Type CurrentLengthAtRefAge2() const {
    switch (length_reference_parameterization_) {
      case LengthReferenceParameterization::kEstimatedL1EstimatedGap:
        return CurrentLengthAtRefAge1Storage() +
               CurrentLengthAtRefAge2Storage();

      case LengthReferenceParameterization::kConstantL1EstimatedGap:
        return constant_length_at_ref_age_1_ +
               CurrentLengthAtRefAge2Storage();

      case LengthReferenceParameterization::kEstimatedL1BelowConstantL2:
        return constant_length_at_ref_age_2_;

      case LengthReferenceParameterization::kBothConstant:
        return constant_length_at_ref_age_2_;
    }

    return CurrentLengthAtRefAge2Storage();
  }

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
  Type CurrentSdLengthAtRefAge1() const {
    return fims_math::exp(log_sd_length_at_ref_age_1_vector_[0]);
  }

  Type CurrentSdLengthAtRefAge2() const {
    return fims_math::exp(log_sd_length_at_ref_age_2_vector_[0]);
  }

  Type CurrentSdGrowthCoefficientK() const {
    return fims_math::exp(log_sd_growth_coefficient_K_vector_[0]);
  }

  Type CurrentCorrLengthAtRefAge1LengthAtRefAge2() const {
    return fims_math::inv_logit(
        static_cast<Type>(-1.0), static_cast<Type>(1.0),
        logit_corr_length_at_ref_age_1_length_at_ref_age_2_vector_[0]);
  }

  Type CurrentCorrLengthAtRefAge1K() const {
    return fims_math::inv_logit(
        static_cast<Type>(-1.0), static_cast<Type>(1.0),
        logit_corr_length_at_ref_age_1_k_vector_[0]);
  }

  Type CurrentCorrLengthAtRefAge2K() const {
    return fims_math::inv_logit(
        static_cast<Type>(-1.0), static_cast<Type>(1.0),
        logit_corr_length_at_ref_age_2_k_vector_[0]);
  }

  Type CurrentLengthAtRefAge1Variance() const {
    const Type sd = CurrentSdLengthAtRefAge1();
    return sd * sd;
  }

  Type CurrentLengthAtRefAge2Variance() const {
    const Type sd = CurrentSdLengthAtRefAge2();
    return sd * sd;
  }

  Type CurrentGrowthCoefficientKVariance() const {
    const Type sd = CurrentSdGrowthCoefficientK();
    return sd * sd;
  }

  Type CurrentLengthAtRefAge1LengthAtRefAge2Covariance() const {
    return CurrentCorrLengthAtRefAge1LengthAtRefAge2() *
           CurrentSdLengthAtRefAge1() *
           CurrentSdLengthAtRefAge2();
  }

  Type CurrentLengthAtRefAge1KCovariance() const {
    return CurrentCorrLengthAtRefAge1K() *
           CurrentSdLengthAtRefAge1() *
           CurrentSdGrowthCoefficientK();
  }

  Type CurrentLengthAtRefAge2KCovariance() const {
    return CurrentCorrLengthAtRefAge2K() *
           CurrentSdLengthAtRefAge2() *
           CurrentSdGrowthCoefficientK();
  }

  bool HasInterpolationSdInputs() const {
    return length_at_age_sd_at_ref_ages_vector_.size() > 0;
  }

  bool HasAnyStructuredDeltaMethodInput() const {
    return log_sd_length_at_ref_age_1_vector_.size() > 0 ||
           log_sd_length_at_ref_age_2_vector_.size() > 0 ||
           log_sd_growth_coefficient_K_vector_.size() > 0 ||
           logit_corr_length_at_ref_age_1_length_at_ref_age_2_vector_.size() > 0 ||
           logit_corr_length_at_ref_age_1_k_vector_.size() > 0 ||
           logit_corr_length_at_ref_age_2_k_vector_.size() > 0;
  }

  bool HasStructuredDeltaMethodInputs() const {
    return log_sd_length_at_ref_age_1_vector_.size() > 0 &&
           log_sd_length_at_ref_age_2_vector_.size() > 0 &&
           log_sd_growth_coefficient_K_vector_.size() > 0 &&
           logit_corr_length_at_ref_age_1_length_at_ref_age_2_vector_.size() > 0 &&
           logit_corr_length_at_ref_age_1_k_vector_.size() > 0 &&
           logit_corr_length_at_ref_age_2_k_vector_.size() > 0;
  }

  void EnsureParamsSet() const {
    if (!use_param_vectors_ || length_at_ref_age_1_vector_.size() < 1 ||
        length_at_ref_age_2_vector_.size() < 1 ||
        growth_coefficient_K_vector_.size() < 1 ||
        reference_age_for_length_1_vector_.size() < 1 ||
        reference_age_for_length_2_vector_.size() < 1 ||
        length_weight_a_vector_.size() < 1 ||
        length_weight_b_vector_.size() < 1) {
      throw std::runtime_error(
          "VonBertalanffyGrowth parameters not set");
    }

    if (length_at_ref_age_1_vector_.size() != 1 ||
        length_at_ref_age_2_vector_.size() != 1 ||
        growth_coefficient_K_vector_.size() != 1 ||
        reference_age_for_length_1_vector_.size() != 1 ||
        reference_age_for_length_2_vector_.size() != 1 ||
        length_weight_a_vector_.size() != 1 ||
        length_weight_b_vector_.size() != 1) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter currently supports a single "
          "growth pattern; expected size 1 for VonB and length-weight "
          "parameter vectors");
    }

    const bool has_sd = HasInterpolationSdInputs();
    const bool has_any_structured_delta = HasAnyStructuredDeltaMethodInput();
    const bool has_structured_delta = HasStructuredDeltaMethodInputs();

    if (!has_sd && !has_any_structured_delta) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter requires either "
          "length_at_age_sd_at_ref_ages or the structured delta-method "
          "growth variability inputs");
    }

    if (has_any_structured_delta && !has_structured_delta) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter requires all six structured "
          "delta-method variability inputs when using that path");
    }

    if (has_sd && has_structured_delta) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter requires variability inputs for "
          "exactly one supported path. Supply either the interpolation "
          "inputs length_at_age_sd_at_ref_ages or the full delta-method "
          "variability inputs, but not both");
    }

    if (has_sd && length_at_age_sd_at_ref_ages_vector_.size() != 2) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter expected exactly 2 "
          "length_at_age_sd_at_ref_ages values");
    }

    if (has_structured_delta &&
        (log_sd_length_at_ref_age_1_vector_.size() != 1 ||
         log_sd_length_at_ref_age_2_vector_.size() != 1 ||
         log_sd_growth_coefficient_K_vector_.size() != 1 ||
         logit_corr_length_at_ref_age_1_length_at_ref_age_2_vector_.size() != 1 ||
         logit_corr_length_at_ref_age_1_k_vector_.size() != 1 ||
         logit_corr_length_at_ref_age_2_k_vector_.size() != 1)) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter currently supports a single "
          "structured delta-method variability parameter set; expected "
          "size 1 for each structured uncertainty input");
    }
  }
};

}  // namespace fims_popdy

#endif  // POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP
