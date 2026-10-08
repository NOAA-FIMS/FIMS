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
#include "functors/vonb_schnute.hpp"
#include "functors/vonb_traditional.hpp"

namespace fims_popdy {

/**
 * @brief Generic capability interface for growth models that can feed the
 * growth-derived age-to-length conversion / bin-based WAA path in catch-at-age.
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
  virtual void Initialize(std::size_t n_years, std::size_t n_ages,
                          std::size_t n_sexes = 1) = 0;

  /**
   * @brief Report whether this growth object can support the dynamic
   * age-to-length conversion path.
   * @return True when growth-derived age-to-length conversion calculations are
   * available.
   */
  virtual bool SupportsAgeToLengthConversionDerived() const = 0;

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
 * @brief von Bertalanffy growth model adapter implementing the generic
 * growth-derived observation capability for catch-at-age.
 *
 * This internal adapter is shared by public von Bertalanffy parameterizations.
 */
template <typename Type>
class VonBertalanffyGrowthModelAdapter
    : public GrowthDerivedObservationBase<Type> {
 public:
  VonBertalanffyGrowthModelAdapter() : GrowthDerivedObservationBase<Type>() {}

  /**
   * @brief Options for how the two reference lengths are supplied.
   */
  enum class LengthReferenceParameterization {
    kEstimatedReferenceLengths = 0,
    kConstantLengthYoungEstimatedLengthOld,
    kEstimatedLengthYoungBelowConstantLengthOld,
    kBothConstant
  };

  /**
   * @brief Options for the mean length-at-age parameterization.
   */
  enum class MeanGrowthParameterization { kSchnute = 0, kTraditional };

  /**
   * @brief Use the Schnute reference-length parameterization.
   */
  void UseVonBertalanffySchnute() {
    mean_growth_parameterization_ = MeanGrowthParameterization::kSchnute;
    growth_products_prepared_ = false;
  }

  /**
   * @brief Use the traditional Linf, K, t0 parameterization.
   */
  void UseTraditionalVonBertalanffy() {
    mean_growth_parameterization_ = MeanGrowthParameterization::kTraditional;
    growth_products_prepared_ = false;
  }

  /**
   * @brief Estimate both reference mean lengths.
   */
  void UseEstimatedReferenceMeanLengths() {
    length_reference_parameterization_ =
        LengthReferenceParameterization::kEstimatedReferenceLengths;
    growth_products_prepared_ = false;
  }

  /**
   * @brief Fix young reference length and estimate older reference mean length.
   *
   * @param constant_mean_length_young Fixed mean length at the younger
   * reference age.
   */
  void UseConstantMeanLengthYoungWithEstimatedMeanLengthOld(
      Type constant_mean_length_young) {
    length_reference_parameterization_ =
        LengthReferenceParameterization::kConstantLengthYoungEstimatedLengthOld;
    constant_mean_length_young_ = constant_mean_length_young;
    growth_products_prepared_ = false;
  }

  /**
   * @brief Estimate young reference length below a fixed old reference length.
   *
   * @param constant_mean_length_old Fixed mean length at the older reference
   * age.
   */
  void UseEstimatedMeanLengthYoungBelowConstantMeanLengthOld(
      Type constant_mean_length_old) {
    length_reference_parameterization_ = LengthReferenceParameterization::
        kEstimatedLengthYoungBelowConstantLengthOld;
    constant_mean_length_old_ = constant_mean_length_old;
    growth_products_prepared_ = false;
  }

  /**
   * @brief Fix both young and old reference mean lengths.
   *
   * @param constant_mean_length_young Fixed mean length at the younger
   * reference age.
   * @param constant_mean_length_old Fixed mean length at the older reference
   * age.
   */
  void UseConstantReferenceMeanLengths(Type constant_mean_length_young,
                                       Type constant_mean_length_old) {
    length_reference_parameterization_ =
        LengthReferenceParameterization::kBothConstant;
    constant_mean_length_young_ = constant_mean_length_young;
    constant_mean_length_old_ = constant_mean_length_old;
    growth_products_prepared_ = false;
  }

  /**
   * @brief Access the working-scale storage for the first reference-length
   * parameter.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& MeanLengthYoungVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return mean_length_young_vector_;
  }

  /**
   * @brief Access the log-scale length-at-reference-age-2 parameter vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& MeanLengthOldVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return mean_length_old_vector_;
  }

  /**
   * @brief Access the log-scale VonBertalanffySchnute growth coefficient
   * vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& GrowthCoefficientVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return growth_coefficient_vector_;
  }

  /**
   * @brief Access the log-scale asymptotic length, Linf, parameter vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& AsymptoticLengthVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return asymptotic_length_vector_;
  }

  /**
   * @brief Access the age-at-zero-length, t0, parameter vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& AgeAtZeroLengthVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return age_at_zero_length_vector_;
  }

  /**
   * @brief Access the first reference-age vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& ReferenceAgeForLengthYoungVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return reference_age_for_length_young_vector_;
  }

  /**
   * @brief Access the second reference-age vector.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& ReferenceAgeForLengthOldVector() {
    use_param_vectors_ = true;
    vb_params_set_ = true;
    growth_products_prepared_ = false;
    return reference_age_for_length_old_vector_;
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
   * @brief Access the log-scale length-at-age SD vector at the two reference
   * ages.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LengthAtAgeSdAtRefAgesVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return length_at_age_sd_at_reference_ages_vector_;
  }

  /**
   * @brief Access the log-SD vector for growth_coefficient.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogSdGrowthCoefficientVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return log_sd_growth_coefficient_vector_;
  }

  /**
   * @brief Access the log-SD vector for log(asymptotic_length).
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogSdAsymptoticLengthVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return log_sd_asymptotic_length_vector_;
  }

  /**
   * @brief Access the log-SD vector for age_at_zero_length.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogSdAgeAtZeroLengthVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return log_sd_age_at_zero_length_vector_;
  }

  /**
   * @brief Access the transformed correlation vector for asymptotic_length and
   * growth_coefficient.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogitCorrAsymptoticLengthGrowthCoefficientVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return asymptotic_length_growth_coefficient_logit_corr_vector_;
  }

  /**
   * @brief Access the transformed correlation vector for asymptotic_length and
   * age_at_zero_length.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogitCorrAsymptoticLengthAgeAtZeroLengthVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return asymptotic_length_age_at_zero_length_logit_corr_vector_;
  }

  /**
   * @brief Access the transformed correlation vector for growth_coefficient and
   * age_at_zero_length.
   * @return Mutable parameter vector.
   */
  fims::Vector<Type>& LogitCorrGrowthCoefficientAgeAtZeroLengthVector() {
    use_param_vectors_ = true;
    growth_products_prepared_ = false;
    return growth_coefficient_age_at_zero_length_logit_corr_vector_;
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
  void Initialize(std::size_t n_years, std::size_t n_ages,
                  std::size_t n_sexes = 1) override {
    EnsureParamsSet();
    if (n_sexes != 1) {
      throw std::runtime_error(
          "VonBertalanffySchnuteGrowthModelAdapter currently supports n_sexes "
          "== 1");
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
   * @brief Report that the adapter supports growth-derived age-to-length
   * conversion calculations.
   * @return Always true for this adapter.
   */
  bool SupportsAgeToLengthConversionDerived() const override { return true; }

  virtual const Type evaluate(int year, const double& a) override {
    if (a < 0.0) {
      throw std::runtime_error("Negative age not supported");
    }
    const double a_round = std::round(a);
    const double tol = 1e-8;
    if (std::fabs(a - a_round) > tol) {
      throw std::runtime_error("Non-integer age not supported yet");
    }
    EnsureParamsSet();
    if (mean_growth_parameterization_ == MeanGrowthParameterization::kSchnute) {
      const Type ref_age_young = CurrentReferenceAgeForLengthYoung();
      const Type ref_age_old = CurrentReferenceAgeForLengthOld();
      if (ref_age_old <= ref_age_young) {
        throw std::runtime_error(
            "VonBertalanffySchnuteGrowth reference_age_for_length_old must be "
            "> reference_age_for_length_young");
      }
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
      const std::size_t age_index = static_cast<std::size_t>(age_index_round);
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
        throw std::runtime_error("Growth model not initialized; n_ages is 0");
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
   * expectation over the same length bins used in the dynamic age-to-length
   * conversion path.
   *
   * @param length Length on the natural scale.
   * @return Weight on the natural scale.
   */
  Type EvaluateWeightAtLength(const Type& length) const override {
    EnsureParamsSet();
    const Type length_safe = fims_math::ad_max(length, static_cast<Type>(1e-8));
    return CurrentLengthWeightA() *
           fims_math::pow(length_safe, CurrentLengthWeightB());
  }

 private:
  // Stored parameter vectors on their working scales. Positive parameters
  // use log scale, reference ages and t0 stay on the natural scale, and
  // correlation terms use transformed working-scale values.
  fims::Vector<Type> mean_length_young_vector_;
  fims::Vector<Type> mean_length_old_vector_;
  fims::Vector<Type> asymptotic_length_vector_;
  fims::Vector<Type> growth_coefficient_vector_;
  fims::Vector<Type> age_at_zero_length_vector_;
  fims::Vector<Type> reference_age_for_length_young_vector_;
  fims::Vector<Type> reference_age_for_length_old_vector_;
  fims::Vector<Type> length_weight_a_vector_;
  fims::Vector<Type> length_weight_b_vector_;
  fims::Vector<Type> length_at_age_sd_at_reference_ages_vector_;
  fims::Vector<Type> log_sd_growth_coefficient_vector_;
  fims::Vector<Type> log_sd_asymptotic_length_vector_;
  fims::Vector<Type> log_sd_age_at_zero_length_vector_;
  fims::Vector<Type> asymptotic_length_growth_coefficient_logit_corr_vector_;
  fims::Vector<Type> asymptotic_length_age_at_zero_length_logit_corr_vector_;
  fims::Vector<Type> growth_coefficient_age_at_zero_length_logit_corr_vector_;
  bool use_param_vectors_ = false;
  MeanGrowthParameterization mean_growth_parameterization_ =
      MeanGrowthParameterization::kSchnute;
  LengthReferenceParameterization length_reference_parameterization_ =
      LengthReferenceParameterization::kEstimatedReferenceLengths;
  Type constant_mean_length_young_ = Type(0.0);
  Type constant_mean_length_old_ = Type(0.0);
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

    if (mean_growth_parameterization_ ==
        MeanGrowthParameterization::kTraditional) {
      fims_popdy::VonBertalanffyTraditionalGrowth<Type> vb;
      vb.asymptotic_length = CurrentAsymptoticLength();
      vb.growth_coefficient = CurrentGrowthCoefficient();
      vb.age_at_zero_length = CurrentAgeAtZeroLength();
      vb.length_weight_a = CurrentLengthWeightA();
      vb.length_weight_b = CurrentLengthWeightB();
      return vb.evaluate(year, a);
    }

    fims_popdy::VonBertalanffySchnuteGrowth<Type> vb;
    vb.mean_length_young = CurrentMeanLengthYoung();
    vb.mean_length_old = CurrentMeanLengthOld();
    vb.growth_coefficient = CurrentGrowthCoefficient();
    vb.reference_age_for_length_young = CurrentReferenceAgeForLengthYoung();
    vb.reference_age_for_length_old = CurrentReferenceAgeForLengthOld();
    vb.length_weight_a = CurrentLengthWeightA();
    vb.length_weight_b = CurrentLengthWeightB();
    return vb.evaluate(year, a);
  }

  void SyncParamsToModel() const {
    if (!model_) return;
    EnsureParamsSet();

    if (mean_growth_parameterization_ ==
        MeanGrowthParameterization::kTraditional) {
      model_->SetVonBertalanffyTraditionalParameters(
          CurrentAsymptoticLength(), CurrentGrowthCoefficient(),
          CurrentAgeAtZeroLength());
    } else {
      model_->SetVonBertalanffySchnuteParameters(
          CurrentMeanLengthYoung(), CurrentMeanLengthOld(),
          CurrentGrowthCoefficient(), CurrentReferenceAgeForLengthYoung(),
          CurrentReferenceAgeForLengthOld());
    }

    model_->SetLengthWeightParameters(CurrentLengthWeightA(),
                                      CurrentLengthWeightB());

    if (HasInterpolationSdInputs()) {
      model_->SetLengthSdReferenceAges(CurrentReferenceAgeForLengthYoung(),
                                       CurrentReferenceAgeForLengthOld());
      model_->SetLengthSdParams(CurrentLengthAtAgeSdAtReferenceAge1(),
                                CurrentLengthAtAgeSdAtReferenceAge2());
    }

    if (mean_growth_parameterization_ ==
            MeanGrowthParameterization::kTraditional &&
        HasTraditionalStructuredDeltaMethodInputs()) {
      model_->SetTraditionalGrowthParameterCovariance(
          CurrentLogAsymptoticLengthVariance(),
          CurrentLogAsymptoticLengthLogGrowthCoefficientCovariance(),
          CurrentLogAsymptoticLengthAgeAtZeroLengthCovariance(),
          CurrentGrowthCoefficientVariance(),
          CurrentLogGrowthCoefficientAgeAtZeroLengthCovariance(),
          CurrentAgeAtZeroLengthVariance());
    } else {
      model_->ClearGrowthParameterCovariance();
    }
  }

  Type CurrentSdLogAsymptoticLength() const {
    return fims_math::exp(log_sd_asymptotic_length_vector_[0]);
  }

  Type CurrentSdAgeAtZeroLength() const {
    return fims_math::exp(log_sd_age_at_zero_length_vector_[0]);
  }

  Type CurrentCorrAsymptoticLengthGrowthCoefficient() const {
    return fims_math::inv_logit(
        static_cast<Type>(-1.0), static_cast<Type>(1.0),
        asymptotic_length_growth_coefficient_logit_corr_vector_[0]);
  }

  Type CurrentCorrAsymptoticLengthAgeAtZeroLength() const {
    return fims_math::inv_logit(
        static_cast<Type>(-1.0), static_cast<Type>(1.0),
        asymptotic_length_age_at_zero_length_logit_corr_vector_[0]);
  }

  Type CurrentCorrGrowthCoefficientAgeAtZeroLength() const {
    return fims_math::inv_logit(
        static_cast<Type>(-1.0), static_cast<Type>(1.0),
        growth_coefficient_age_at_zero_length_logit_corr_vector_[0]);
  }

  Type CurrentLogAsymptoticLengthVariance() const {
    const Type sd = CurrentSdLogAsymptoticLength();
    return sd * sd;
  }

  Type CurrentAgeAtZeroLengthVariance() const {
    const Type sd = CurrentSdAgeAtZeroLength();
    return sd * sd;
  }

  Type CurrentLogAsymptoticLengthLogGrowthCoefficientCovariance() const {
    return CurrentCorrAsymptoticLengthGrowthCoefficient() *
           CurrentSdLogAsymptoticLength() * CurrentSdGrowthCoefficient();
  }

  Type CurrentLogAsymptoticLengthAgeAtZeroLengthCovariance() const {
    return CurrentCorrAsymptoticLengthAgeAtZeroLength() *
           CurrentSdLogAsymptoticLength() * CurrentSdAgeAtZeroLength();
  }

  Type CurrentLogGrowthCoefficientAgeAtZeroLengthCovariance() const {
    return CurrentCorrGrowthCoefficientAgeAtZeroLength() *
           CurrentSdGrowthCoefficient() * CurrentSdAgeAtZeroLength();
  }

  Type CurrentMeanLengthYoungStorage() const {
    return fims_math::exp(mean_length_young_vector_[0]);
  }

  Type CurrentMeanLengthOldStorage() const {
    return fims_math::exp(mean_length_old_vector_[0]);
  }

  Type CurrentAsymptoticLength() const {
    return fims_math::exp(asymptotic_length_vector_[0]);
  }

  Type CurrentAgeAtZeroLength() const { return age_at_zero_length_vector_[0]; }

  Type CurrentMeanLengthYoung() const {
    switch (length_reference_parameterization_) {
      case LengthReferenceParameterization::kEstimatedReferenceLengths:
        return CurrentMeanLengthYoungStorage();

      case LengthReferenceParameterization::
          kConstantLengthYoungEstimatedLengthOld:
        return constant_mean_length_young_;

      case LengthReferenceParameterization::
          kEstimatedLengthYoungBelowConstantLengthOld:
        return fims_math::inv_logit(static_cast<Type>(0.0),
                                    constant_mean_length_old_,
                                    mean_length_young_vector_[0]);

      case LengthReferenceParameterization::kBothConstant:
        return constant_mean_length_young_;
    }

    return CurrentMeanLengthYoungStorage();
  }

  Type CurrentMeanLengthOld() const {
    switch (length_reference_parameterization_) {
      case LengthReferenceParameterization::kEstimatedReferenceLengths:
      case LengthReferenceParameterization::
          kConstantLengthYoungEstimatedLengthOld:
        return CurrentMeanLengthOldStorage();

      case LengthReferenceParameterization::
          kEstimatedLengthYoungBelowConstantLengthOld:
        return constant_mean_length_old_;

      case LengthReferenceParameterization::kBothConstant:
        return constant_mean_length_old_;
    }

    return CurrentMeanLengthOldStorage();
  }

  Type CurrentGrowthCoefficient() const {
    return fims_math::exp(growth_coefficient_vector_[0]);
  }
  Type CurrentReferenceAgeForLengthYoung() const {
    return reference_age_for_length_young_vector_[0];
  }
  Type CurrentReferenceAgeForLengthOld() const {
    return reference_age_for_length_old_vector_[0];
  }
  Type CurrentLengthWeightA() const {
    return fims_math::exp(length_weight_a_vector_[0]);
  }
  Type CurrentLengthWeightB() const {
    return fims_math::exp(length_weight_b_vector_[0]);
  }
  Type CurrentLengthAtAgeSdAtReferenceAge1() const {
    return fims_math::exp(length_at_age_sd_at_reference_ages_vector_[0]);
  }
  Type CurrentLengthAtAgeSdAtReferenceAge2() const {
    return fims_math::exp(length_at_age_sd_at_reference_ages_vector_[1]);
  }
  Type CurrentSdGrowthCoefficient() const {
    return fims_math::exp(log_sd_growth_coefficient_vector_[0]);
  }

  Type CurrentGrowthCoefficientVariance() const {
    const Type sd = CurrentSdGrowthCoefficient();
    return sd * sd;
  }

  bool HasInterpolationSdInputs() const {
    return length_at_age_sd_at_reference_ages_vector_.size() > 0;
  }

  bool HasAnyTraditionalStructuredDeltaMethodInput() const {
    return log_sd_asymptotic_length_vector_.size() > 0 ||
           log_sd_growth_coefficient_vector_.size() > 0 ||
           log_sd_age_at_zero_length_vector_.size() > 0 ||
           asymptotic_length_growth_coefficient_logit_corr_vector_.size() > 0 ||
           asymptotic_length_age_at_zero_length_logit_corr_vector_.size() > 0 ||
           growth_coefficient_age_at_zero_length_logit_corr_vector_.size() > 0;
  }

  bool HasTraditionalStructuredDeltaMethodInputs() const {
    return log_sd_asymptotic_length_vector_.size() > 0 &&
           log_sd_growth_coefficient_vector_.size() > 0 &&
           log_sd_age_at_zero_length_vector_.size() > 0 &&
           asymptotic_length_growth_coefficient_logit_corr_vector_.size() > 0 &&
           asymptotic_length_age_at_zero_length_logit_corr_vector_.size() > 0 &&
           growth_coefficient_age_at_zero_length_logit_corr_vector_.size() > 0;
  }

  void EnsureParamsSet() const {
    if (!use_param_vectors_ || growth_coefficient_vector_.size() < 1 ||
        length_weight_a_vector_.size() < 1 ||
        length_weight_b_vector_.size() < 1) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter parameters not set");
    }

    if (mean_growth_parameterization_ == MeanGrowthParameterization::kSchnute &&
        (mean_length_young_vector_.size() < 1 ||
         mean_length_old_vector_.size() < 1 ||
         reference_age_for_length_young_vector_.size() < 1 ||
         reference_age_for_length_old_vector_.size() < 1)) {
      throw std::runtime_error(
          "VonBertalanffySchnuteGrowth parameters not set");
    }

    if (mean_growth_parameterization_ ==
            MeanGrowthParameterization::kTraditional &&
        (asymptotic_length_vector_.size() < 1 ||
         age_at_zero_length_vector_.size() < 1)) {
      throw std::runtime_error(
          "Traditional Von Bertalanffy growth parameters not set");
    }

    if (growth_coefficient_vector_.size() != 1 ||
        length_weight_a_vector_.size() != 1 ||
        length_weight_b_vector_.size() != 1) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter currently supports a single "
          "growth pattern; expected size 1 for shared growth and length-weight "
          "parameter vectors");
    }

    if (mean_growth_parameterization_ == MeanGrowthParameterization::kSchnute &&
        (mean_length_young_vector_.size() != 1 ||
         mean_length_old_vector_.size() != 1 ||
         reference_age_for_length_young_vector_.size() != 1 ||
         reference_age_for_length_old_vector_.size() != 1)) {
      throw std::runtime_error(
          "VonBertalanffySchnuteGrowthModelAdapter currently supports a single "
          "growth pattern; expected size 1 for von Bertalanffy--Schnute "
          "parameter vectors");
    }

    if (mean_growth_parameterization_ ==
            MeanGrowthParameterization::kTraditional &&
        (asymptotic_length_vector_.size() != 1 ||
         age_at_zero_length_vector_.size() != 1)) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter currently supports a single "
          "growth pattern; expected size 1 for traditional von Bertalanffy "
          "parameter vectors");
    }

    const bool has_sd = HasInterpolationSdInputs();
    const bool has_any_traditional_delta =
        HasAnyTraditionalStructuredDeltaMethodInput();
    const bool has_traditional_delta =
        HasTraditionalStructuredDeltaMethodInputs();

    if (mean_growth_parameterization_ == MeanGrowthParameterization::kSchnute &&
        !has_sd) {
      throw std::runtime_error(
          "VonBertalanffySchnuteGrowthModelAdapter requires interpolation "
          "variability inputs: length_at_age_sd_at_reference_ages");
    }

    if (mean_growth_parameterization_ ==
            MeanGrowthParameterization::kTraditional &&
        !has_sd && !has_any_traditional_delta) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter requires either "
          "length_at_age_sd_at_reference_ages or traditional delta-method "
          "growth variability inputs");
    }

    if (mean_growth_parameterization_ ==
            MeanGrowthParameterization::kTraditional &&
        has_any_traditional_delta && !has_traditional_delta) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter requires all six traditional "
          "delta-method variability inputs when using that path");
    }

    if (mean_growth_parameterization_ ==
            MeanGrowthParameterization::kTraditional &&
        has_sd && has_any_traditional_delta) {
      throw std::runtime_error(
          "von Bertalanffy growth adapter requires variability inputs for "
          "exactly one supported path. Supply either the interpolation inputs "
          "length_at_age_sd_at_reference_ages or the full traditional "
          "delta-method variability inputs, but not both");
    }

    if (has_sd && length_at_age_sd_at_reference_ages_vector_.size() != 2) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter expected exactly 2 "
          "length_at_age_sd_at_reference_ages values");
    }

    if (has_sd &&
        (reference_age_for_length_young_vector_.size() < 1 ||
         reference_age_for_length_old_vector_.size() < 1)) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter interpolation variability requires "
          "reference_age_for_length_young and reference_age_for_length_old");
    }

    if (has_sd &&
        (reference_age_for_length_young_vector_.size() != 1 ||
         reference_age_for_length_old_vector_.size() != 1)) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter currently supports a single "
          "interpolation variability parameter set; expected size 1 for each "
          "reference-age input");
    }

    if (mean_growth_parameterization_ ==
            MeanGrowthParameterization::kTraditional &&
        has_traditional_delta &&
        (log_sd_asymptotic_length_vector_.size() != 1 ||
         log_sd_growth_coefficient_vector_.size() != 1 ||
         log_sd_age_at_zero_length_vector_.size() != 1 ||
         asymptotic_length_growth_coefficient_logit_corr_vector_.size() != 1 ||
         asymptotic_length_age_at_zero_length_logit_corr_vector_.size() != 1 ||
         growth_coefficient_age_at_zero_length_logit_corr_vector_.size() != 1)) {
      throw std::runtime_error(
          "VonBertalanffyGrowthModelAdapter currently supports a single "
          "traditional delta-method variability parameter set; expected size 1 "
          "for each structured uncertainty input");
    }
  }
};

template <typename Type>
using VonBertalanffySchnuteGrowthModelAdapter =
    VonBertalanffyGrowthModelAdapter<Type>;

}  // namespace fims_popdy

#endif  // POPULATION_DYNAMICS_GROWTH_MODEL_ADAPTER_HPP
