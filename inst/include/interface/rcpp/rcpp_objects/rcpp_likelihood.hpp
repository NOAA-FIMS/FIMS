/**
 * @file rcpp_likelihood.hpp
 * @brief Rcpp interfaces for likelihood components.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_LIKELIHOOD_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_LIKELIHOOD_HPP

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "rcpp_interface_base.hpp"
#include "../../../likelihood/likelihood.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for likelihood
 * interfaces.
 */
class LikelihoodInterfaceBase : public FIMSRcppInterfaceBase {
 protected:
  fims::Vector<double> fims_input_values;
  fims::Vector<double> fims_observed_values;
  fims::Vector<double> fims_expected_values;
  fims::Vector<double> fims_expected_input_values;

  /**
   * @brief Convert an R-facing RealVector to a backend fims::Vector.
   */
  fims::Vector<double> to_fims_vector(RealVector values) {
    fims::Vector<double> converted;
    converted.resize(values.size());
    for (size_t i = 0; i < values.size(); i++) {
      converted[i] = values[i];
    }
    return converted;
  }

  /**
   * @brief Convert an R-facing ParameterVector to a backend fims::Vector.
   */
  fims::Vector<double> to_fims_vector(ParameterVector values) {
    fims::Vector<double> converted;
    converted.resize(values.size());
    for (size_t i = 0; i < values.size(); i++) {
      converted[i] = values[i].initial_value_m;
    }
    return converted;
  }

  /**
   * @brief Convert a role string to the backend likelihood role.
   */
  fims_likelihood::LikelihoodRole parse_role() const {
    std::string role = this->role_m.get();
    if (role == "data") {
      return fims_likelihood::LikelihoodRole::Data;
    }
    if (role == "prior") {
      return fims_likelihood::LikelihoodRole::Prior;
    }
    if (role == "random_effect" || role == "random_effects" || role == "re") {
      return fims_likelihood::LikelihoodRole::RandomEffect;
    }
    if (role == "penalty") {
      return fims_likelihood::LikelihoodRole::Penalty;
    }
    throw std::invalid_argument("Unsupported likelihood role: " + role);
  }

  /**
   * @brief Set common values on a backend likelihood component.
   */
  void set_common_values(
      fims_likelihood::LikelihoodComponentBase<double>& likelihood) {
    this->fims_observed_values = this->to_fims_vector(this->observed_values);
    this->fims_expected_values = this->to_fims_vector(this->expected_values);
    this->fims_expected_input_values =
        this->to_fims_vector(this->expected_real_input);

    likelihood.observed_values = this->fims_observed_values;
    likelihood.expected_values = this->fims_expected_values;
    if (this->expected_source_m.get() == "real") {
      likelihood.SetExpected(&this->fims_expected_input_values);
    } else if (this->expected_source_m.get() == "parameter") {
      this->fims_expected_input_values =
          this->to_fims_vector(this->expected_parameter_input);
      likelihood.SetExpected(&this->fims_expected_input_values);
    }

    std::string input_source = this->input_source_m.get();
    if (input_source == "parameter") {
      this->fims_input_values = this->to_fims_vector(this->parameter_input);
      likelihood.SetInput(&this->fims_input_values, this->parse_role());
    } else if (input_source == "real") {
      this->fims_input_values = this->to_fims_vector(this->real_input);
      likelihood.SetInput(&this->fims_input_values, this->parse_role());
    } else if (input_source == "observed") {
      likelihood.role = this->parse_role();
    } else {
      throw std::invalid_argument("Unsupported likelihood input source: " +
                                  input_source);
    }
  }

  /**
   * @brief Store backend NLL components in the R-facing RealVector.
   */
  void set_nll_components(fims::Vector<double> values) {
    this->nll_components.resize(values.size());
    for (size_t i = 0; i < values.size(); i++) {
      this->nll_components[i] = values[i];
    }
  }

  /**
   * @brief Convert an R-facing RealVector to a backend fims::Vector.
   */
  template <typename Type>
  fims::Vector<Type> to_fims_vector_t(RealVector values) {
    fims::Vector<Type> converted;
    converted.resize(values.size());
    for (size_t i = 0; i < values.size(); i++) {
      converted[i] = values[i];
    }
    return converted;
  }

  /**
   * @brief Convert an R-facing ParameterVector to a backend fims::Vector.
   */
  template <typename Type>
  fims::Vector<Type> to_fims_vector_t(ParameterVector values) {
    fims::Vector<Type> converted;
    converted.resize(values.size());
    for (size_t i = 0; i < values.size(); i++) {
      converted[i] = values[i].initial_value_m;
    }
    return converted;
  }

  /**
   * @brief Register linked parameter inputs with the Information object.
   */
  template <typename Type, typename LikelihoodType>
  fims::Vector<Type>* register_parameter_input(
      std::shared_ptr<LikelihoodType> likelihood,
      const std::string& component_name) {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();
    typename fims_info::Information<Type>::variable_map_iterator vmit =
        info->variable_map.find(this->parameter_input.id_m);
    if (vmit != info->variable_map.end()) {
      return (*vmit).second.variable;
    }

    std::stringstream ss;
    for (size_t i = 0; i < this->parameter_input.size(); i++) {
      if (this->parameter_input[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << component_name << "." << this->id_m << ".input."
           << this->parameter_input[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(likelihood->input_storage[i]);
      }
      if (this->parameter_input[i].estimation_type_m.get() ==
          "random_effects") {
        ss.str("");
        ss << component_name << "." << this->id_m << ".input."
           << this->parameter_input[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(likelihood->input_storage[i]);
      }
    }

    info->variable_map[this->parameter_input.id_m] =
        typename fims_info::Information<Type>::VariableMapEntry(
            &likelihood->input_storage,
            *this->parameter_input.input_transformation_m,
            *this->parameter_input.prior_transformation_m);
    return &likelihood->input_storage;
  }

  /**
   * @brief Register or resolve a backend vector for an R-facing RealVector id.
   */
  template <typename Type>
  fims::Vector<Type>* register_or_get_real_vector(fims::Vector<Type>* values,
                                                  uint32_t id) {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();
    typename fims_info::Information<Type>::variable_map_iterator vmit =
        info->variable_map.find(id);
    if (vmit != info->variable_map.end()) {
      return (*vmit).second.variable;
    }

    fims::Transformation identity;
    identity.label = fims::Transformation::Label::identity;
    info->variable_map[id] =
        typename fims_info::Information<Type>::VariableMapEntry(
            values, identity, identity);
    return values;
  }

  /**
   * @brief Fill common backend likelihood fields and register linked inputs.
   */
  template <typename Type, typename LikelihoodType>
  void set_common_tmb_values(std::shared_ptr<LikelihoodType> likelihood,
                             const std::string& component_name) {
    likelihood->observed_values =
        this->to_fims_vector_t<Type>(this->observed_values);
    likelihood->expected_values =
        this->to_fims_vector_t<Type>(this->expected_values);
    std::string expected_source = this->expected_source_m.get();
    if (expected_source == "real") {
      likelihood->expected_storage =
          this->to_fims_vector_t<Type>(this->expected_real_input);
      likelihood->expected_values_id = this->expected_real_input.id_m;
      likelihood->use_expected_values_id = true;
    } else if (expected_source == "parameter") {
      likelihood->expected_storage =
          this->to_fims_vector_t<Type>(this->expected_parameter_input);
      likelihood->expected_values_id = this->expected_parameter_input.id_m;
      likelihood->use_expected_values_id = true;
    } else if (expected_source != "local") {
      throw std::invalid_argument("Unsupported likelihood expected source: " +
                                  expected_source);
    }
    likelihood->role = this->parse_role();

    std::string input_source = this->input_source_m.get();
    if (input_source == "parameter") {
      likelihood->input_storage =
          this->to_fims_vector_t<Type>(this->parameter_input);
      likelihood->SetInput(this->template register_parameter_input<Type>(
                               likelihood, component_name),
                           this->parse_role());
    } else if (input_source == "real") {
      likelihood->input_storage =
          this->to_fims_vector_t<Type>(this->real_input);
      likelihood->SetInput(
          this->register_or_get_real_vector(&likelihood->input_storage,
                                            this->real_input.id_m),
          this->parse_role());
    } else if (input_source != "observed") {
      throw std::invalid_argument("Unsupported likelihood input source: " +
                                  input_source);
    }
  }

  /**
   * @brief Register a backend likelihood with the Information object.
   */
  template <typename Type, typename LikelihoodType>
  void register_likelihood(std::shared_ptr<LikelihoodType> likelihood) {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();
    info->likelihood_components[this->id_m] = likelihood;
  }

 public:
  static uint32_t id_g;
  uint32_t id_m;
  RealVector observed_values;
  RealVector expected_values;
  RealVector real_input;
  RealVector expected_real_input;
  ParameterVector parameter_input;
  ParameterVector expected_parameter_input;
  RealVector log_sd;
  RealVector log_shape;
  RealVector log_scale;
  RealVector dims;
  RealVector nll_components;
  SharedString role_m = SharedString("data");
  SharedString input_source_m = SharedString("observed");
  SharedString expected_source_m = SharedString("local");
  double nll_value = 0.0;

  static std::vector<std::shared_ptr<LikelihoodInterfaceBase>> live_objects;

  /**
   * @brief The constructor.
   */
  LikelihoodInterfaceBase() { this->id_m = LikelihoodInterfaceBase::id_g++; }

  /**
   * @brief Construct a new Likelihood Interface Base object.
   *
   * @param other Object to copy.
   */
  LikelihoodInterfaceBase(const LikelihoodInterfaceBase& other)
      : id_m(other.id_m),
        observed_values(other.observed_values),
        expected_values(other.expected_values),
        real_input(other.real_input),
        expected_real_input(other.expected_real_input),
        parameter_input(other.parameter_input),
        expected_parameter_input(other.expected_parameter_input),
        log_sd(other.log_sd),
        log_shape(other.log_shape),
        log_scale(other.log_scale),
        dims(other.dims),
        nll_components(other.nll_components),
        role_m(other.role_m),
        input_source_m(other.input_source_m),
        expected_source_m(other.expected_source_m),
        nll_value(other.nll_value) {}

  virtual ~LikelihoodInterfaceBase() {}

  /**
   * @brief Get the ID for the child likelihood interface objects.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief Set the input role without linking an external input vector.
   */
  virtual bool set_role(std::string role) {
    this->role_m.set(role);
    this->parse_role();
    this->sync_live_input_state();
    return true;
  }

  /**
   * @brief Link a RealVector as the likelihood input.
   */
  virtual bool set_real_input(RealVector input, std::string role) {
    this->real_input = input;
    this->role_m.set(role);
    this->input_source_m.set("real");
    this->parse_role();
    this->sync_live_input_state();
    return true;
  }

  /**
   * @brief Link a RealVector as the likelihood expected value.
   */
  virtual bool set_real_expected_input(RealVector expected) {
    this->expected_real_input = expected;
    this->expected_source_m.set("real");
    this->sync_live_input_state();
    return true;
  }

  /**
   * @brief Link a ParameterVector as the likelihood expected value.
   */
  virtual bool set_parameter_expected_input(ParameterVector expected) {
    this->expected_parameter_input = expected;
    this->expected_source_m.set("parameter");
    this->sync_live_input_state();
    return true;
  }

  /**
   * @brief Link a ParameterVector as the likelihood input.
   */
  virtual bool set_parameter_input(ParameterVector input, std::string role) {
    this->parameter_input = input;
    this->role_m.set(role);
    this->input_source_m.set("parameter");
    this->parse_role();
    this->sync_live_input_state();
    return true;
  }

  /**
   * @brief Evaluate the likelihood.
   */
  virtual double evaluate() = 0;

 protected:
  /**
   * @brief Keep the live copy used by CreateTMBModel synchronized with R-side
   * setter calls.
   */
  void sync_live_input_state() {
    for (size_t i = 0; i < LikelihoodInterfaceBase::live_objects.size(); i++) {
      std::shared_ptr<LikelihoodInterfaceBase> object =
          LikelihoodInterfaceBase::live_objects[i];
      if (object->id_m == this->id_m && object.get() != this) {
        object->real_input = this->real_input;
        object->expected_real_input = this->expected_real_input;
        object->parameter_input = this->parameter_input;
        object->expected_parameter_input = this->expected_parameter_input;
        object->role_m = this->role_m;
        object->input_source_m = this->input_source_m;
        object->expected_source_m = this->expected_source_m;
      }
    }
  }
};

uint32_t LikelihoodInterfaceBase::id_g = 1;
std::vector<std::shared_ptr<LikelihoodInterfaceBase>>
    LikelihoodInterfaceBase::live_objects;

/**
 * @brief The Rcpp interface for a normal likelihood.
 */
class NormalLikelihoodInterface : public LikelihoodInterfaceBase {
 public:
  static std::vector<std::shared_ptr<NormalLikelihoodInterface>> live_objects;

  NormalLikelihoodInterface() : LikelihoodInterfaceBase() {
    std::shared_ptr<NormalLikelihoodInterface> object =
        std::make_shared<NormalLikelihoodInterface>(*this);
    NormalLikelihoodInterface::live_objects.push_back(object);
    LikelihoodInterfaceBase::live_objects.push_back(object);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(object);
  }

  NormalLikelihoodInterface(const NormalLikelihoodInterface& other)
      : LikelihoodInterfaceBase(other) {}

  virtual ~NormalLikelihoodInterface() {}

  virtual uint32_t get_id() { return this->id_m; }

  virtual double evaluate() {
    fims_likelihood::NormalLikelihood<double> likelihood;
    this->set_common_values(likelihood);
    likelihood.log_sd = this->to_fims_vector(this->log_sd);
    this->nll_value = likelihood.Evaluate();
    this->set_nll_components(likelihood.nll_components);
    return this->nll_value;
  }

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_likelihood::NormalLikelihood<Type>> likelihood =
        std::make_shared<fims_likelihood::NormalLikelihood<Type>>();
    this->template set_common_tmb_values<Type>(likelihood, "normal_likelihood");
    likelihood->log_sd = this->to_fims_vector_t<Type>(this->log_sd);
    this->template register_likelihood<Type>(likelihood);
    return true;
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }
};

std::vector<std::shared_ptr<NormalLikelihoodInterface>>
    NormalLikelihoodInterface::live_objects;

/**
 * @brief The Rcpp interface for a lognormal likelihood.
 */
class LognormalLikelihoodInterface : public LikelihoodInterfaceBase {
 public:
  static std::vector<std::shared_ptr<LognormalLikelihoodInterface>>
      live_objects;

  LognormalLikelihoodInterface() : LikelihoodInterfaceBase() {
    std::shared_ptr<LognormalLikelihoodInterface> object =
        std::make_shared<LognormalLikelihoodInterface>(*this);
    LognormalLikelihoodInterface::live_objects.push_back(object);
    LikelihoodInterfaceBase::live_objects.push_back(object);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(object);
  }

  LognormalLikelihoodInterface(const LognormalLikelihoodInterface& other)
      : LikelihoodInterfaceBase(other) {}

  virtual ~LognormalLikelihoodInterface() {}

  virtual uint32_t get_id() { return this->id_m; }

  virtual double evaluate() {
    fims_likelihood::LognormalLikelihood<double> likelihood;
    this->set_common_values(likelihood);
    likelihood.log_sd = this->to_fims_vector(this->log_sd);
    this->nll_value = likelihood.Evaluate();
    this->set_nll_components(likelihood.nll_components);
    return this->nll_value;
  }

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_likelihood::LognormalLikelihood<Type>> likelihood =
        std::make_shared<fims_likelihood::LognormalLikelihood<Type>>();
    this->template set_common_tmb_values<Type>(likelihood,
                                               "lognormal_likelihood");
    likelihood->log_sd = this->to_fims_vector_t<Type>(this->log_sd);
    this->template register_likelihood<Type>(likelihood);
    return true;
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }
};

std::vector<std::shared_ptr<LognormalLikelihoodInterface>>
    LognormalLikelihoodInterface::live_objects;

/**
 * @brief The Rcpp interface for a gamma likelihood.
 */
class GammaLikelihoodInterface : public LikelihoodInterfaceBase {
 public:
  static std::vector<std::shared_ptr<GammaLikelihoodInterface>> live_objects;

  GammaLikelihoodInterface() : LikelihoodInterfaceBase() {
    std::shared_ptr<GammaLikelihoodInterface> object =
        std::make_shared<GammaLikelihoodInterface>(*this);
    GammaLikelihoodInterface::live_objects.push_back(object);
    LikelihoodInterfaceBase::live_objects.push_back(object);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(object);
  }

  GammaLikelihoodInterface(const GammaLikelihoodInterface& other)
      : LikelihoodInterfaceBase(other) {}

  virtual ~GammaLikelihoodInterface() {}

  virtual uint32_t get_id() { return this->id_m; }

  virtual double evaluate() {
    fims_likelihood::GammaLikelihood<double> likelihood;
    this->set_common_values(likelihood);
    likelihood.log_sd = this->to_fims_vector(this->log_sd);
    this->nll_value = likelihood.Evaluate();
    this->set_nll_components(likelihood.nll_components);
    return this->nll_value;
  }

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_likelihood::GammaLikelihood<Type>> likelihood =
        std::make_shared<fims_likelihood::GammaLikelihood<Type>>();
    this->template set_common_tmb_values<Type>(likelihood, "gamma_likelihood");
    likelihood->log_sd = this->to_fims_vector_t<Type>(this->log_sd);
    this->template register_likelihood<Type>(likelihood);
    return true;
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }
};

std::vector<std::shared_ptr<GammaLikelihoodInterface>>
    GammaLikelihoodInterface::live_objects;

/**
 * @brief The Rcpp interface for an inverse-gamma likelihood.
 */
class InvGammaLikelihoodInterface : public LikelihoodInterfaceBase {
 public:
  static std::vector<std::shared_ptr<InvGammaLikelihoodInterface>> live_objects;

  InvGammaLikelihoodInterface() : LikelihoodInterfaceBase() {
    std::shared_ptr<InvGammaLikelihoodInterface> object =
        std::make_shared<InvGammaLikelihoodInterface>(*this);
    InvGammaLikelihoodInterface::live_objects.push_back(object);
    LikelihoodInterfaceBase::live_objects.push_back(object);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(object);
  }

  InvGammaLikelihoodInterface(const InvGammaLikelihoodInterface& other)
      : LikelihoodInterfaceBase(other) {}

  virtual ~InvGammaLikelihoodInterface() {}

  virtual uint32_t get_id() { return this->id_m; }

  virtual double evaluate() {
    fims_likelihood::InvGammaLikelihood<double> likelihood;
    this->set_common_values(likelihood);
    likelihood.log_shape = this->to_fims_vector(this->log_shape);
    likelihood.log_scale = this->to_fims_vector(this->log_scale);
    this->nll_value = likelihood.Evaluate();
    this->set_nll_components(likelihood.nll_components);
    return this->nll_value;
  }

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_likelihood::InvGammaLikelihood<Type>> likelihood =
        std::make_shared<fims_likelihood::InvGammaLikelihood<Type>>();
    this->template set_common_tmb_values<Type>(likelihood,
                                               "invgamma_likelihood");
    likelihood->log_shape = this->to_fims_vector_t<Type>(this->log_shape);
    likelihood->log_scale = this->to_fims_vector_t<Type>(this->log_scale);
    this->template register_likelihood<Type>(likelihood);
    return true;
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }
};

std::vector<std::shared_ptr<InvGammaLikelihoodInterface>>
    InvGammaLikelihoodInterface::live_objects;

/**
 * @brief The Rcpp interface for a multinomial likelihood.
 */
class MultinomialLikelihoodInterface : public LikelihoodInterfaceBase {
 public:
  static std::vector<std::shared_ptr<MultinomialLikelihoodInterface>>
      live_objects;

  MultinomialLikelihoodInterface() : LikelihoodInterfaceBase() {
    std::shared_ptr<MultinomialLikelihoodInterface> object =
        std::make_shared<MultinomialLikelihoodInterface>(*this);
    MultinomialLikelihoodInterface::live_objects.push_back(object);
    LikelihoodInterfaceBase::live_objects.push_back(object);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(object);
  }

  MultinomialLikelihoodInterface(const MultinomialLikelihoodInterface& other)
      : LikelihoodInterfaceBase(other) {}

  virtual ~MultinomialLikelihoodInterface() {}

  virtual uint32_t get_id() { return this->id_m; }

  virtual double evaluate() {
    fims_likelihood::MultinomialLikelihood<double> likelihood;
    this->set_common_values(likelihood);
    likelihood.dims.resize(this->dims.size());
    for (size_t i = 0; i < this->dims.size(); i++) {
      likelihood.dims[i] = static_cast<size_t>(this->dims[i]);
    }
    this->nll_value = likelihood.Evaluate();
    this->set_nll_components(likelihood.nll_components);
    return this->nll_value;
  }

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_likelihood::MultinomialLikelihood<Type>> likelihood =
        std::make_shared<fims_likelihood::MultinomialLikelihood<Type>>();
    this->template set_common_tmb_values<Type>(likelihood,
                                               "multinomial_likelihood");
    likelihood->dims.resize(this->dims.size());
    for (size_t i = 0; i < this->dims.size(); i++) {
      likelihood->dims[i] = static_cast<size_t>(this->dims[i]);
    }
    this->template register_likelihood<Type>(likelihood);
    return true;
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }
};

std::vector<std::shared_ptr<MultinomialLikelihoodInterface>>
    MultinomialLikelihoodInterface::live_objects;

#endif
