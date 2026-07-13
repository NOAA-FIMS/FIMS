/**
 * @file rcpp_growth.hpp
 * @brief The Rcpp interface to declare different types of growth, e.g.,
 * empirical weight-at-age data. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_GROWTH_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_GROWTH_HPP

#include <algorithm>
#include <cmath>

#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
#include "../../../population_dynamics/growth/growth.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp growth
 * interfaces. This type should be inherited and not called from R directly.
 */
class GrowthInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static id of the GrowthInterfaceBase object.
   */
  static uint32_t id_g;
  /**
   * @brief The local id of the GrowthInterfaceBase object.
   */
  uint32_t id;
  /**
   * @brief The map associating the IDs of GrowthInterfaceBase to the objects.
   * This is a live object, which is an object that has been created and lives
   * in memory.
   */
  static std::map<uint32_t, std::shared_ptr<GrowthInterfaceBase>> live_objects;

  /**
   * @brief The constructor.
   */
  GrowthInterfaceBase() {
    this->id = GrowthInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    GrowthInterfaceBase */
    // GrowthInterfaceBase::live_objects[this->id] =
    // std::make_shared<GrowthInterfaceBase>(*this);
  }

  /**
   * @brief Construct a new Growth Interface Base object
   *
   * @param other
   */
  GrowthInterfaceBase(const GrowthInterfaceBase &other) : id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~GrowthInterfaceBase() {}

  /**
   * @brief Get the ID for the child growth interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief A method for each child growth interface object to inherit so
   * each growth option can have an evaluate() function.
   */
  virtual double evaluate(double age) = 0;
};

/**
 * @brief Rcpp interface base for growth models that can register a
 * growth-derived observation capability with the runtime model.
 *
 * This keeps the Rcpp producer side aligned with the generic
 * GrowthDerivedObservationBase contract used by catch-at-age while still
 * allowing concrete interfaces to manage their own model-specific parameters.
 */
class GrowthDerivedObservationInterfaceBase : public GrowthInterfaceBase {
 public:
  GrowthDerivedObservationInterfaceBase() : GrowthInterfaceBase() {}

  /**
   * @brief Copy constructor.
   * @param other Source interface object.
   */
  GrowthDerivedObservationInterfaceBase(
      const GrowthDerivedObservationInterfaceBase& other)
      : GrowthInterfaceBase(other) {}

  virtual ~GrowthDerivedObservationInterfaceBase() {}

 protected:
  template <typename Type>
  /**
   * @brief Look up the registered growth-derived observation object.
   * @return Shared pointer to the registered runtime growth object, or null.
   */
  std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>
  GetGrowthObservationFromInfo() const {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    typename fims_info::Information<Type>::growth_models_iterator it =
        info->growth_models.find(this->id);
    if (it == info->growth_models.end()) {
      FIMS_WARNING_LOG("Growth-derived observation model " +
                       fims::to_string(this->id) +
                       " not found in Information.");
      return nullptr;
    }

    std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>
        growth_observation =
            std::dynamic_pointer_cast<
                fims_popdy::GrowthDerivedObservationBase<Type>>(it->second);
    if (!growth_observation) {
      FIMS_WARNING_LOG("Growth-derived observation type mismatch for id " +
                       fims::to_string(this->id));
      return nullptr;
    }

    return growth_observation;
  }

  template <typename Type>
  /**
   * @brief Register a runtime growth-derived observation in Information.
   * @param growth_observation Runtime growth object to register.
   */
  void RegisterGrowthObservationInInfo(
      const std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>&
          growth_observation) const {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();
    info->growth_models[growth_observation->id] = growth_observation;
  }
};

/**
 * @brief Rcpp interface for EWAAGrowth to instantiate the object from R:
 * ewaa <- methods::new(EWAAGrowth). Where, EWAA stands for empirical weight at
 * age and growth is not actually estimated.
 */
class EWAAGrowthInterface : public GrowthInterfaceBase {
 public:
  /**
   * @brief Weights (mt) for each age class.
   */
  RealVector weights;
  /**
   * @brief Ages (years) for each age class.
   */
  RealVector ages;
  /**
   * @brief An integer specifying the number of years.
   *
   */
  SharedInt n_years;
  /**
   * @brief A map of empirical weight-at-age values allowing multiple modules to
   * access and modify the weights without copying values between modules.
   */
  std::shared_ptr<std::map<int, std::map<double, double>>> ewaa;
  /**
   * @brief Have weight and age vectors been set? The default is false.
   */
  bool initialized = false;

  /**
   * @brief The constructor.
   */
  EWAAGrowthInterface() : GrowthInterfaceBase() {
    this->ewaa = std::make_shared<std::map<int, std::map<double, double>>>();
    GrowthInterfaceBase::live_objects[this->id] =
        std::make_shared<EWAAGrowthInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        std::make_shared<EWAAGrowthInterface>(*this));
  }

  /**
   * @brief Construct a new EWAAGrowthInterface object
   *
   * @param other
   */
  EWAAGrowthInterface(const EWAAGrowthInterface &other)
      : GrowthInterfaceBase(other),
        weights(other.weights),
        ages(other.ages),
        n_years(other.n_years),
        ewaa(other.ewaa),
        initialized(other.initialized) {}

  /**
   * @brief The destructor.
   */
  virtual ~EWAAGrowthInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Create a map of input numeric vectors.
   * @param weights Type vector of weights.
   * @param ages Type vector of ages.
   * @param n_years An integer specifying the number of years.
   * @return std::map<T, T>.
   */
  inline std::map<int, std::map<double, double>> make_map(RealVector ages,
                                                          RealVector weights,
                                                          SharedInt n_years) {
    std::map<int, std::map<double, double>> mymap;
    const size_t n_years_plus_one = static_cast<size_t>(n_years.get() + 1);

    // Reject invalid year counts because map keys are expected to include
    // at least one model year.
    if (n_years.get() < 1) {
      Rcpp::stop("EWAA Error:: n_years must be at least 1");
    }

    // Reject empty vectors because we need at least one age-weight pair.
    if (weights.size() == 0 || ages.size() == 0) {
      Rcpp::stop("EWAA Error:: ages and weights must have at least one value");
    }

    // Accept either:
    // 1) one weight vector by age (shared across years), or
    // 2) a full year-by-age matrix flattened as (n_years + 1) * n_ages.
    if ((weights.size() != ages.size() * n_years_plus_one) &&
        (weights.size() != ages.size())) {
      Rcpp::stop(
          "weights size does not match ages size or ages size times "
          "(n_years + 1), where the plus one is for the beginning "
          "of the year after the terminal year spawning-biomass "
          "calculations. weights size: " +
          std::to_string(weights.size()) +
          " ages size: " + std::to_string(ages.size()) +
          " n_years: " + std::to_string(n_years.get()));
    } else if (weights.size() == ages.size()) {
      // One age-specific vector was provided, so replicate the same
      // weight-at-age values for every year key (0 through n_years).
      for (size_t y = 0; y < n_years_plus_one; y++) {
        for (size_t i = 0; i < ages.size(); i++) {
          mymap[y][ages[i]] = weights[i];
        }
      }
    } else if (weights.size() == ages.size() * n_years_plus_one) {
      // A flattened year-by-age matrix was provided, so map each block of
      // n_ages values to the corresponding year key (0 through n_years).
      for (size_t y = 0; y < n_years_plus_one; y++) {
        for (size_t i = 0; i < ages.size(); i++) {
          mymap[y][ages[i]] = weights[y * ages.size() + i];
        }
      }
    }
    return mymap;
  }

  /**
   * @brief Evaluate the growth using empirical weight at age.
   * @param age The age at of the individual to evaluate weight.
   * @details This can be called from R using ewaagrowth.evaluate(age).
   */
  virtual double evaluate(double age) {
    fims_popdy::EWAAGrowth<double> EWAAGrowth;

    // Build the EWAA map once from R inputs the first time evaluate() is
    // called.
    if (initialized == false) {
      EWAAGrowth.ewaa = make_map(this->ages, this->weights, this->n_years);
      initialized = true;
    } else {
      // Prevent re-initializing this object with a second evaluate() call.
      Rcpp::stop("this empirical weight at age object is already initialized");
    }
    return EWAAGrowth.evaluate(0, age);
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * growth interface with empirical weight at age. It also returns the ID,
   * the rank of 1, the dimensions, age bins, and the calculated values
   * themselves. This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;
    ss << "{\n";
    ss << " \"module_name\": \"Growth\",\n";
    ss << " \"module_type\": \"EWAA\",\n";
    ss << " \"module_id\":" << this->id << ",\n";
    ss << " \"parameters\": [\n{\n";
    ss << " \"name\": \"weight_at_age\",\n";
    ss << " \"id\": null,\n";
    ss << " \"type\": \"vector\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [\"n_years+1\",\"n_ages\"],\n";
    ss << "  \"dimensions\": [" << this->n_years.get() + 1 << ","
       << this->ages.size() << "]\n},\n";

    ss << " \"values\": [\n";
    for (size_t i = 0; i < weights.size() - 1; i++) {
      ss << "{\n";
      ss << "\"id\": null,\n";
      ss << "\"value\": " << weights[i] << ",\n";
      ss << "\"estimated_value\": " << weights[i] << ",\n";
      ss << "\"estimation_type\": \"constant\"\n";
      ss << "},\n";
    }
    ss << "{\n";
    ss << "\"id\": null,\n";
    ss << "\"value\": " << weights[weights.size() - 1] << ",\n";
    ss << "\"estimated_value\": " << weights[weights.size() - 1] << ",\n";
    ss << "\"estimation_type\": \"constant\"\n";
    ss << "}\n]\n";
    ss << "}\n]\n}\n";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::EWAAGrowth<Type>> ewaa_growth =
        std::make_shared<fims_popdy::EWAAGrowth<Type>>();

    // set relative info
    ewaa_growth->id = this->id;
    ewaa_growth->ewaa =
        make_map(this->ages, this->weights, this->n_years);  // this->ewaa;
    // add to Information
    info->growth_models[ewaa_growth->id] = ewaa_growth;

    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();

    return true;
  }

#endif
};

/**
 * @brief Rcpp-facing Von Bertalanffy growth module.
 * Inherits "derived observation" capability so this growth can support
 * growth-derived outputs used by downstream ALK/WAA paths.
 */
class VonBertalanffyGrowthInterface
    : public GrowthDerivedObservationInterfaceBase {
 public:
  VariableVector length_at_ref_age_1; /**< expected length at reference age 1 */
  VariableVector length_at_ref_age_2; /**< expected length at reference age 2 */
  VariableVector growth_coefficient_K; /**< Von Bertalanffy growth coefficient */
  VariableVector reference_age_for_length_1; /**< first reference age */
  VariableVector reference_age_for_length_2; /**< second reference age */
  VariableVector length_weight_a; /**< coefficient in W = a * L^b */
  VariableVector length_weight_b; /**< exponent in W = a * L^b */
  VariableVector length_at_age_sd_at_ref_ages; /**< natural-scale SD values at the two reference ages for the legacy interpolation path */
  VariableVector log_sd_length_at_ref_age_1; /**< working-scale VonB variability parameter for sd(log(length_at_ref_age_1)) */
  VariableVector log_sd_length_at_ref_age_2; /**< working-scale VonB variability parameter for sd(log(length_at_ref_age_2)) */
  VariableVector log_sd_growth_coefficient_K; /**< working-scale VonB variability parameter for sd(log(growth_coefficient_K)) */
  VariableVector logit_corr_length_at_ref_age_1_length_at_ref_age_2; /**< working-scale VonB variability parameter for corr(log(length_at_ref_age_1), log(length_at_ref_age_2)) */
  VariableVector logit_corr_length_at_ref_age_1_k; /**< working-scale VonB variability parameter for corr(log(length_at_ref_age_1), log(growth_coefficient_K)) */
  VariableVector logit_corr_length_at_ref_age_2_k; /**< working-scale VonB variability parameter for corr(log(length_at_ref_age_2), log(growth_coefficient_K)) */
  SharedInt n_ages = 0; /**< modeled number of ages for validation */

  /**
   * @brief Construct a new Von Bertalanffy growth interface.
   */
  VonBertalanffyGrowthInterface() : GrowthDerivedObservationInterfaceBase() {
    // Variability inputs are optional and mutually exclusive by path, so
    // leave them absent until a caller explicitly supplies one path.
    this->length_at_age_sd_at_ref_ages.resize(0);
    this->log_sd_length_at_ref_age_1.resize(0);
    this->log_sd_length_at_ref_age_2.resize(0);
    this->log_sd_growth_coefficient_K.resize(0);
    this->logit_corr_length_at_ref_age_1_length_at_ref_age_2.resize(0);
    this->logit_corr_length_at_ref_age_1_k.resize(0);
    this->logit_corr_length_at_ref_age_2_k.resize(0);

    // Register this interface instance in global registries so it can be
    // discovered and linked by ID during model initialization.
    GrowthInterfaceBase::live_objects[this->id] =
        std::make_shared<VonBertalanffyGrowthInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        std::make_shared<VonBertalanffyGrowthInterface>(*this));
  }

  /**
   * @brief Copy constructor.
   * @param other Source interface object.
   */
  VonBertalanffyGrowthInterface(const VonBertalanffyGrowthInterface& other)
      : GrowthDerivedObservationInterfaceBase(other),
        length_at_ref_age_1(other.length_at_ref_age_1),
        length_at_ref_age_2(other.length_at_ref_age_2),
        growth_coefficient_K(other.growth_coefficient_K),
        reference_age_for_length_1(other.reference_age_for_length_1),
        reference_age_for_length_2(other.reference_age_for_length_2),
        length_weight_a(other.length_weight_a),
        length_weight_b(other.length_weight_b),
        length_at_age_sd_at_ref_ages(other.length_at_age_sd_at_ref_ages),
        log_sd_length_at_ref_age_1(other.log_sd_length_at_ref_age_1),
        log_sd_length_at_ref_age_2(other.log_sd_length_at_ref_age_2),
        log_sd_growth_coefficient_K(other.log_sd_growth_coefficient_K),
        logit_corr_length_at_ref_age_1_length_at_ref_age_2(
            other.logit_corr_length_at_ref_age_1_length_at_ref_age_2),
        logit_corr_length_at_ref_age_1_k(
            other.logit_corr_length_at_ref_age_1_k),
        logit_corr_length_at_ref_age_2_k(
            other.logit_corr_length_at_ref_age_2_k),
        n_ages(other.n_ages) {}

  virtual ~VonBertalanffyGrowthInterface() {}

  // Return stable module ID used for linking this growth object to populations.
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object from
   * the Information object.
   */
  virtual void finalize() {
    if (this->finalized) {
      FIMS_WARNING_LOG("VonBertalanffy Growth " + fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true;

    std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<double>>
        growth_observation = this->GetGrowthObservationFromInfo<double>();
    if (!growth_observation) {
      return;
    }

    std::shared_ptr<fims_popdy::VonBertalanffyGrowthModelAdapter<double>> vb =
        std::dynamic_pointer_cast<
            fims_popdy::VonBertalanffyGrowthModelAdapter<double>>(
            growth_observation);
    if (!vb) {
      FIMS_WARNING_LOG("Growth model type mismatch for id " +
                       fims::to_string(this->id));
      return;
    }

    auto set_final = [](VariableVector& pv, const fims::Vector<double>& src,
                        bool log_scale) {
      for (size_t i = 0; i < pv.size(); i++) {
        if (pv[i].estimation_type_m.get() == "constant") {
          pv[i].final_value_m = pv[i].initial_value_m;
        } else {
          double v = src[i];
          if (log_scale) {
            v = fims_math::exp(v);
          }
          pv[i].final_value_m = v;
        }
      }
    };

    SetFinalLengthParameter(
        this->length_at_ref_age_1,
        FinalLengthAtRefAge1FromWorkingScale(vb->LengthAtRefAge1Vector()));
    SetFinalLengthParameter(
        this->length_at_ref_age_2,
        FinalLengthAtRefAge2FromWorkingScale(vb->LengthAtRefAge1Vector(),
                                             vb->LengthAtRefAge2Vector()));
    set_final(this->growth_coefficient_K, vb->GrowthCoefficientKVector(), true);
    set_final(this->reference_age_for_length_1,
              vb->ReferenceAgeForLength1Vector(), false);
    set_final(this->reference_age_for_length_2,
              vb->ReferenceAgeForLength2Vector(), false);
    set_final(this->length_weight_a, vb->LengthWeightAVector(), true);
    set_final(this->length_weight_b, vb->LengthWeightBVector(), true);
    set_final(this->length_at_age_sd_at_ref_ages,
              vb->LengthAtAgeSdAtRefAgesVector(), true);
    set_final(this->log_sd_length_at_ref_age_1,
              vb->LogSdLengthAtRefAge1Vector(), false);
    set_final(this->log_sd_length_at_ref_age_2,
              vb->LogSdLengthAtRefAge2Vector(), false);
    set_final(this->log_sd_growth_coefficient_K,
              vb->LogSdGrowthCoefficientKVector(), false);
    set_final(this->logit_corr_length_at_ref_age_1_length_at_ref_age_2,
              vb->LogitCorrLengthAtRefAge1LengthAtRefAge2Vector(), false);
    set_final(this->logit_corr_length_at_ref_age_1_k,
              vb->LogitCorrLengthAtRefAge1KVector(), false);
    set_final(this->logit_corr_length_at_ref_age_2_k,
              vb->LogitCorrLengthAtRefAge2KVector(), false);
  }

  virtual double evaluate(double age) {
    if (age < 0.0) {
      Rcpp::stop("Negative age not supported");
    }
    if (this->n_ages.get() <= 0) {
      Rcpp::stop("n_ages not set");
    }
    const double age_round = std::round(age);
    const double tol = 1e-8;
    if (std::fabs(age - age_round) > tol) {
      Rcpp::stop("Non-integer age not supported yet");
    }
    ValidateVonBInputs(false);

    fims_popdy::VonBertalanffyGrowth<double> vb;

    vb.length_at_ref_age_1 = this->length_at_ref_age_1[0].initial_value_m;
    vb.length_at_ref_age_2 = this->length_at_ref_age_2[0].initial_value_m;
    vb.growth_coefficient_K  = this->growth_coefficient_K[0].initial_value_m;
    vb.reference_age_for_length_1 = this->reference_age_for_length_1[0].initial_value_m;
    vb.reference_age_for_length_2 = this->reference_age_for_length_2[0].initial_value_m;

    vb.length_weight_a = this->length_weight_a[0].initial_value_m;
    vb.length_weight_b = this->length_weight_b[0].initial_value_m;

    return vb.evaluate(0, age);
  }

 private:
  enum class LengthReferenceParameterization {
    kEstimatedL1EstimatedGap = 0,
    kConstantL1EstimatedGap,
    kEstimatedL1BelowConstantL2,
    kBothConstant
  };

  bool IsEstimated(VariableVector& pv) {
    return pv.size() > 0 && pv[0].estimation_type_m.get() != "constant";
  }

  double InitialLengthAtRefAge1() {
    return this->length_at_ref_age_1[0].initial_value_m;
  }

  double InitialLengthAtRefAge2() {
    return this->length_at_ref_age_2[0].initial_value_m;
  }

  LengthReferenceParameterization GetLengthReferenceParameterization() {
    const bool estimate_l1 = IsEstimated(this->length_at_ref_age_1);
    const bool estimate_l2 = IsEstimated(this->length_at_ref_age_2);

    if (estimate_l1 && estimate_l2) {
      return LengthReferenceParameterization::kEstimatedL1EstimatedGap;
    }
    if (!estimate_l1 && estimate_l2) {
      return LengthReferenceParameterization::kConstantL1EstimatedGap;
    }
    if (estimate_l1 && !estimate_l2) {
      return LengthReferenceParameterization::kEstimatedL1BelowConstantL2;
    }
    return LengthReferenceParameterization::kBothConstant;
  }

  double WorkingScaleInitialLengthAtRefAge1() {
    switch (GetLengthReferenceParameterization()) {
      case LengthReferenceParameterization::kEstimatedL1BelowConstantL2:
        return fims_math::logit(
            0.0,
            InitialLengthAtRefAge2(),
            InitialLengthAtRefAge1());

      case LengthReferenceParameterization::kEstimatedL1EstimatedGap:
      case LengthReferenceParameterization::kConstantL1EstimatedGap:
      case LengthReferenceParameterization::kBothConstant:
        return fims_math::log(InitialLengthAtRefAge1());
    }

    return fims_math::log(InitialLengthAtRefAge1());
  }

  double WorkingScaleInitialLengthAtRefAge2() {
    switch (GetLengthReferenceParameterization()) {
      case LengthReferenceParameterization::kEstimatedL1EstimatedGap:
      case LengthReferenceParameterization::kConstantL1EstimatedGap:
        return fims_math::log(
            InitialLengthAtRefAge2() - InitialLengthAtRefAge1());

      case LengthReferenceParameterization::kEstimatedL1BelowConstantL2:
      case LengthReferenceParameterization::kBothConstant:
        return fims_math::log(InitialLengthAtRefAge2());
    }

    return fims_math::log(InitialLengthAtRefAge2());
  }

  double FinalLengthAtRefAge1FromWorkingScale(
      const fims::Vector<double>& length_at_ref_age_1_src) {
    switch (GetLengthReferenceParameterization()) {
      case LengthReferenceParameterization::kEstimatedL1EstimatedGap:
        return fims_math::exp(length_at_ref_age_1_src[0]);

      case LengthReferenceParameterization::kConstantL1EstimatedGap:
        return InitialLengthAtRefAge1();

      case LengthReferenceParameterization::kEstimatedL1BelowConstantL2:
        return fims_math::inv_logit(
            0.0,
            InitialLengthAtRefAge2(),
            length_at_ref_age_1_src[0]);

      case LengthReferenceParameterization::kBothConstant:
        return InitialLengthAtRefAge1();
    }

    return InitialLengthAtRefAge1();
  }

  double FinalLengthAtRefAge2FromWorkingScale(
      const fims::Vector<double>& length_at_ref_age_1_src,
      const fims::Vector<double>& length_at_ref_age_2_src) {
    switch (GetLengthReferenceParameterization()) {
      case LengthReferenceParameterization::kEstimatedL1EstimatedGap:
        return FinalLengthAtRefAge1FromWorkingScale(length_at_ref_age_1_src) +
               fims_math::exp(length_at_ref_age_2_src[0]);

      case LengthReferenceParameterization::kConstantL1EstimatedGap:
        return InitialLengthAtRefAge1() +
               fims_math::exp(length_at_ref_age_2_src[0]);

      case LengthReferenceParameterization::kEstimatedL1BelowConstantL2:
        return InitialLengthAtRefAge2();

      case LengthReferenceParameterization::kBothConstant:
        return InitialLengthAtRefAge2();
    }

    return InitialLengthAtRefAge2();
  }

  void SetFinalLengthParameter(VariableVector& pv, double final_value) const {
    for (size_t i = 0; i < pv.size(); i++) {
      if (pv[i].estimation_type_m.get() == "constant") {
        pv[i].final_value_m = pv[i].initial_value_m;
      } else {
        pv[i].final_value_m = final_value;
      }
    }
  }

  template <typename Type>
  void ConfigureLengthReferenceParameterization(
      const std::shared_ptr<fims_popdy::VonBertalanffyGrowthModelAdapter<Type>>&
          vb) {
    switch (GetLengthReferenceParameterization()) {
      case LengthReferenceParameterization::kEstimatedL1EstimatedGap:
        vb->UseEstimatedLengthAtRefAge1WithEstimatedGap();
        break;

      case LengthReferenceParameterization::kConstantL1EstimatedGap:
        vb->UseConstantLengthAtRefAge1WithEstimatedGap(
            static_cast<Type>(InitialLengthAtRefAge1()));
        break;

      case LengthReferenceParameterization::kEstimatedL1BelowConstantL2:
        vb->UseEstimatedLengthAtRefAge1BelowConstantLengthAtRefAge2(
            static_cast<Type>(InitialLengthAtRefAge2()));
        break;

      case LengthReferenceParameterization::kBothConstant:
        vb->UseConstantReferenceLengths(
            static_cast<Type>(InitialLengthAtRefAge1()),
            static_cast<Type>(InitialLengthAtRefAge2()));
        break;
    }
  }

  void ValidateVonBInputs(bool require_variability) {
    if (this->length_at_ref_age_1.size() < 1 ||
        this->length_at_ref_age_2.size() < 1 ||
        this->growth_coefficient_K.size() < 1 ||
        this->reference_age_for_length_1.size() < 1 ||
        this->reference_age_for_length_2.size() < 1 ||
        this->length_weight_a.size() < 1 ||
        this->length_weight_b.size() < 1) {
      Rcpp::stop("VonBertalanffyGrowth parameters not set");
    }

    const bool has_sd = this->length_at_age_sd_at_ref_ages.size() > 0;
    const bool has_any_structured_delta =
        this->log_sd_length_at_ref_age_1.size() > 0 ||
        this->log_sd_length_at_ref_age_2.size() > 0 ||
        this->log_sd_growth_coefficient_K.size() > 0 ||
        this->logit_corr_length_at_ref_age_1_length_at_ref_age_2.size() > 0 ||
        this->logit_corr_length_at_ref_age_1_k.size() > 0 ||
        this->logit_corr_length_at_ref_age_2_k.size() > 0;
    const bool has_structured_delta =
        this->log_sd_length_at_ref_age_1.size() > 0 &&
        this->log_sd_length_at_ref_age_2.size() > 0 &&
        this->log_sd_growth_coefficient_K.size() > 0 &&
        this->logit_corr_length_at_ref_age_1_length_at_ref_age_2.size() > 0 &&
        this->logit_corr_length_at_ref_age_1_k.size() > 0 &&
        this->logit_corr_length_at_ref_age_2_k.size() > 0;

    if (require_variability && !has_sd && !has_any_structured_delta) {
      Rcpp::stop(
          "VonBertalanffyGrowth requires either "
          "length_at_age_sd_at_ref_ages or the structured delta-method "
          "growth variability inputs");
    }

    if (has_any_structured_delta && !has_structured_delta) {
      Rcpp::stop(
          "VonBertalanffyGrowth requires all six structured delta-method "
          "variability inputs when using that path");
    }

    if (has_sd && has_structured_delta) {
      Rcpp::stop(
          "VonBertalanffyGrowth requires variability inputs for exactly one "
          "supported path. Supply either the interpolation inputs "
          "length_at_age_sd_at_ref_ages or the full delta-method "
          "variability inputs, but not both");
    }

    if (has_sd && this->length_at_age_sd_at_ref_ages.size() != 2) {
      Rcpp::stop("length_at_age_sd_at_ref_ages must have two values");
    }

    if (has_structured_delta &&
        (this->log_sd_length_at_ref_age_1.size() != 1 ||
         this->log_sd_length_at_ref_age_2.size() != 1 ||
         this->log_sd_growth_coefficient_K.size() != 1 ||
         this->logit_corr_length_at_ref_age_1_length_at_ref_age_2.size() != 1 ||
         this->logit_corr_length_at_ref_age_1_k.size() != 1 ||
         this->logit_corr_length_at_ref_age_2_k.size() != 1)) {
      Rcpp::stop(
          "VonBertalanffyGrowth currently supports a single structured "
          "delta-method variability parameter set; expected size 1 for "
          "each structured uncertainty input");
    }

    const double a1 = this->reference_age_for_length_1[0].initial_value_m;
    const double a2 = this->reference_age_for_length_2[0].initial_value_m;
    if (a2 <= a1) {
      Rcpp::stop(
          "VonBertalanffyGrowth reference_age_for_length_2 must be > "
          "reference_age_for_length_1");
    }

    auto check_positive = [](VariableVector& pv,
                             const std::string& base_name) {
      for (size_t i = 0; i < pv.size(); i++) {
        if (pv[i].initial_value_m <= 0.0) {
          Rcpp::stop((base_name + " must be > 0").c_str());
        }
      }
    };

    check_positive(this->length_at_ref_age_1, "length_at_ref_age_1");
    check_positive(this->length_at_ref_age_2, "length_at_ref_age_2");
    check_positive(this->growth_coefficient_K, "growth_coefficient_K");
    check_positive(this->length_weight_a, "length_weight_a");
    check_positive(this->length_weight_b, "length_weight_b");

    if (this->length_at_ref_age_2[0].initial_value_m <=
        this->length_at_ref_age_1[0].initial_value_m) {
      Rcpp::stop(
          "VonBertalanffyGrowth length_at_ref_age_2 must be > "
          "length_at_ref_age_1");
    }

    if (has_sd) {
      check_positive(this->length_at_age_sd_at_ref_ages,
                     "length_at_age_sd_at_ref_ages");
    }
  }

 public:

virtual std::string to_json() {
  std::stringstream ss;

  ss << "{\n";
  ss << " \"module_name\":\"Growth\",\n";
  ss << " \"module_type\": \"VonBertalanffy\",\n";
  ss << " \"module_id\": " << this->id << ",\n";

  ss << " \"parameters\": [\n";

  // length_at_ref_age_1
  ss << "{\n";
  ss << "   \"name\": \"length_at_ref_age_1\",\n";
  ss << "   \"id\":" << this->length_at_ref_age_1.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->length_at_ref_age_1.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->length_at_ref_age_1 << "\n";
  ss << "},\n";

  // growth_coefficient_K
  ss << "{\n";
  ss << "   \"name\": \"growth_coefficient_K\",\n";
  ss << "   \"id\":" << this->growth_coefficient_K.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->growth_coefficient_K.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->growth_coefficient_K << "\n";
  ss << "},\n";

  // length_at_ref_age_2
  ss << "{\n";
  ss << "   \"name\": \"length_at_ref_age_2\",\n";
  ss << "   \"id\":" << this->length_at_ref_age_2.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->length_at_ref_age_2.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->length_at_ref_age_2 << "\n";
  ss << "},\n";

  // reference_age_for_length_1
  ss << "{\n";
  ss << "   \"name\": \"reference_age_for_length_1\",\n";
  ss << "   \"id\":" << this->reference_age_for_length_1.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->reference_age_for_length_1.size()
     << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->reference_age_for_length_1 << "\n";
  ss << "},\n";

  // reference_age_for_length_2
  ss << "{\n";
  ss << "   \"name\": \"reference_age_for_length_2\",\n";
  ss << "   \"id\":" << this->reference_age_for_length_2.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->reference_age_for_length_2.size()
     << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->reference_age_for_length_2 << "\n";
  ss << "},\n";

  // length_weight_a
  ss << "{\n";
  ss << "   \"name\": \"length_weight_a\",\n";
  ss << "   \"id\":" << this->length_weight_a.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->length_weight_a.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->length_weight_a << "\n";
  ss << "},\n";

  // length_weight_b (last)
  ss << "{\n";
  ss << "   \"name\": \"length_weight_b\",\n";
  ss << "   \"id\":" << this->length_weight_b.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->length_weight_b.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->length_weight_b << "\n";
  ss << "}";

  // Optional variability parameters
  auto append_optional_parameter = [&](const std::string& name,
                                       VariableVector& pv) {
    if (pv.size() == 0) {
      return;
    }

    ss << ",\n";
    ss << "{\n";
    ss << "   \"name\": \"" << name << "\",\n";
    ss << "   \"id\":" << pv.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"dimensionality\": {\n";
    ss << "    \"header\": [null],\n";
    ss << "    \"dimensions\": [" << pv.size() << "]\n";
    ss << "   },\n";
    ss << "   \"values\":" << pv << "\n";
    ss << "}";
  };

  append_optional_parameter("length_at_age_sd_at_ref_ages",
                            this->length_at_age_sd_at_ref_ages);
  append_optional_parameter("log_sd_length_at_ref_age_1",
                            this->log_sd_length_at_ref_age_1);
  append_optional_parameter("log_sd_length_at_ref_age_2",
                            this->log_sd_length_at_ref_age_2);
  append_optional_parameter("log_sd_growth_coefficient_K",
                            this->log_sd_growth_coefficient_K);
  append_optional_parameter(
      "logit_corr_length_at_ref_age_1_length_at_ref_age_2",
      this->logit_corr_length_at_ref_age_1_length_at_ref_age_2);
  append_optional_parameter("logit_corr_length_at_ref_age_1_k",
                            this->logit_corr_length_at_ref_age_1_k);
  append_optional_parameter("logit_corr_length_at_ref_age_2_k",
                            this->logit_corr_length_at_ref_age_2_k);

  ss << "\n]\n";
  ss << "}";

  return ss.str();
}
#ifdef TMB_MODEL
template <typename Type>
bool add_to_fims_tmb_internal() {
  std::shared_ptr<fims_info::Information<Type>> info =
      fims_info::Information<Type>::GetInstance();

  std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>
      growth_observation =
          std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<Type>>();
  std::shared_ptr<fims_popdy::VonBertalanffyGrowthModelAdapter<Type>> vb =
      std::dynamic_pointer_cast<
          fims_popdy::VonBertalanffyGrowthModelAdapter<Type>>(
          growth_observation);
  if (!vb) {
    Rcpp::stop(
        "Failed to create VonBertalanffy growth-derived observation model");
  }

  vb->id = this->id;

  // Build parameter vectors and register if estimable. Growth variability
  // must be supplied through either the legacy interpolation SD anchors or
  // the transformed delta-method variability inputs.
  ValidateVonBInputs(true);

  std::stringstream ss;
  auto load_and_register = [&](VariableVector& pv,
                               fims::Vector<Type>& target,
                               const std::string& base_name,
                               bool log_scale) {
    target.resize(pv.size());
    for (size_t i = 0; i < pv.size(); i++) {
      double v = pv[i].initial_value_m;
      if (log_scale) {
        if (v <= 0.0) {
          Rcpp::stop((base_name + " must be > 0").c_str());
        }
        v = fims_math::log(v);
      }
      target[i] = static_cast<Type>(v);
      if (pv[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Growth." << this->id << "." << base_name << "."
           << pv[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(target[i]);
      }
      if (pv[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Growth." << this->id << "." << base_name << "."
           << pv[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(target[i]);
      }
    }
    info->variable_map[pv.id_m] = &target;
  };

  auto load_transformed_and_register = [&](VariableVector& pv,
                                           fims::Vector<Type>& target,
                                           const std::string& base_name,
                                           double transformed_value) {
    target.resize(pv.size());
    for (size_t i = 0; i < pv.size(); i++) {
      target[i] = static_cast<Type>(transformed_value);
      if (pv[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Growth." << this->id << "." << base_name << "."
           << pv[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(target[i]);
      }
      if (pv[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Growth." << this->id << "." << base_name << "."
           << pv[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(target[i]);
      }
    }
    info->variable_map[pv.id_m] = &target;
  };

  auto load_optional_and_register = [&](VariableVector& pv,
                                        fims::Vector<Type>& target,
                                        const std::string& base_name,
                                        bool log_scale) {
    if (pv.size() == 0) {
      target.resize(0);
      return;
    }
    load_and_register(pv, target, base_name, log_scale);
  };

  ConfigureLengthReferenceParameterization<Type>(vb);

  load_transformed_and_register(this->length_at_ref_age_1,
                                vb->LengthAtRefAge1Vector(),
                                "length_at_ref_age_1",
                                WorkingScaleInitialLengthAtRefAge1());
  load_transformed_and_register(this->length_at_ref_age_2,
                                vb->LengthAtRefAge2Vector(),
                                "length_at_ref_age_2",
                                WorkingScaleInitialLengthAtRefAge2());
  load_and_register(this->growth_coefficient_K, vb->GrowthCoefficientKVector(),
                    "growth_coefficient_K", true);
  load_and_register(this->reference_age_for_length_1,
                    vb->ReferenceAgeForLength1Vector(),
                    "reference_age_for_length_1", false);
  load_and_register(this->reference_age_for_length_2,
                    vb->ReferenceAgeForLength2Vector(),
                    "reference_age_for_length_2", false);
  load_and_register(this->length_weight_a, vb->LengthWeightAVector(),
                    "length_weight_a", true);
  load_and_register(this->length_weight_b, vb->LengthWeightBVector(),
                    "length_weight_b", true);
  load_optional_and_register(this->length_at_age_sd_at_ref_ages,
                             vb->LengthAtAgeSdAtRefAgesVector(),
                             "length_at_age_sd_at_ref_ages", true);
  load_optional_and_register(this->log_sd_length_at_ref_age_1,
                             vb->LogSdLengthAtRefAge1Vector(),
                             "log_sd_length_at_ref_age_1", false);
  load_optional_and_register(this->log_sd_length_at_ref_age_2,
                             vb->LogSdLengthAtRefAge2Vector(),
                             "log_sd_length_at_ref_age_2", false);
  load_optional_and_register(this->log_sd_growth_coefficient_K,
                             vb->LogSdGrowthCoefficientKVector(),
                             "log_sd_growth_coefficient_K", false);
  load_optional_and_register(
      this->logit_corr_length_at_ref_age_1_length_at_ref_age_2,
      vb->LogitCorrLengthAtRefAge1LengthAtRefAge2Vector(),
      "logit_corr_length_at_ref_age_1_length_at_ref_age_2", false);
  load_optional_and_register(this->logit_corr_length_at_ref_age_1_k,
                             vb->LogitCorrLengthAtRefAge1KVector(),
                             "logit_corr_length_at_ref_age_1_k", false);
  load_optional_and_register(this->logit_corr_length_at_ref_age_2_k,
                             vb->LogitCorrLengthAtRefAge2KVector(),
                             "logit_corr_length_at_ref_age_2_k", false);

  this->RegisterGrowthObservationInInfo<Type>(growth_observation);
  return true;
}

virtual bool add_to_fims_tmb() {
  this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
  this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
  return true;
}
#endif

};  // end class

#endif
