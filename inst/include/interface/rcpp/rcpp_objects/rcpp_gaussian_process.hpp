/**
 * @file rcpp_gaussian_process.hpp
 * @brief Rcpp interface for IID, AR(1), and initialized random-walk densities.
 */
#ifndef FIMS_RCPP_GAUSSIAN_PROCESS_HPP
#define FIMS_RCPP_GAUSSIAN_PROCESS_HPP

#include <algorithm>
#include <cmath>
#include <memory>
#include <iomanip>
#include <limits>
#include "rcpp_distribution.hpp"

/** @brief A process density bound to an existing parameter and mean vector. */
class GaussianProcessDistributionsInterface
    : public DistributionsInterfaceBase {
 public:
  VariableVector observed_values;   /*!< Standalone target values. */
  VariableVector expected_values;   /*!< Standalone or unlinked means. */
  VariableVector log_sd;            /*!< Scalar innovation log-SD. */
  VariableVector unconstrained_rho; /*!< Scalar atanh correlation. */
  RealVector
      lpdf_vec; /*!< Initial and conditional log-density contributions. */
  std::shared_ptr<size_t> n_series_m = std::make_shared<size_t>(1);
  SharedString process_m = fims::to_string("iid");
  SharedString initial_rule_m = fims::to_string("none");
  std::shared_ptr<double> initial_mean_m = std::make_shared<double>(0.0);
  std::shared_ptr<double> initial_sd_m = std::make_shared<double>(1.0);

  GaussianProcessDistributionsInterface() : DistributionsInterfaceBase() {
    log_sd.resize(1);
    unconstrained_rho.resize(1);
    log_sd[0].initial_value_m = 0.0;
    unconstrained_rho[0].initial_value_m = 0.0;
    log_sd[0].estimation_type_m.set("constant");
    unconstrained_rho[0].estimation_type_m.set("constant");
    DistributionsInterfaceBase::live_objects[this->id_m] =
        std::make_shared<GaussianProcessDistributionsInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DistributionsInterfaceBase::live_objects[this->id_m]);
  }

  uint32_t get_id() override { return this->id_m; }

  /** @brief Select IID or stationary AR(1) without changing target ownership.
   */
  bool set_process(std::string process) {
    if (process != "iid" && process != "ar1") {
      Rcpp::stop("GaussianProcess type must be iid or ar1");
    }
    process_m.set(process);
    initial_rule_m.set("none");
    return true;
  }

  /** @brief Declare a random walk with a fixed anchor or a proper initial
   * density. */
  bool set_random_walk(std::string initial_rule, double mean, double sd) {
    if ((initial_rule != "fixed" && initial_rule != "normal") ||
        !std::isfinite(mean) || !std::isfinite(sd) || sd <= 0) {
      Rcpp::stop(
          "Random walk requires fixed or normal initialization and finite "
          "mean/positive SD");
    }
    initial_rule_m.set(initial_rule);
    *initial_mean_m = mean;
    *initial_sd_m = sd;
    process_m.set("rw1");
    return true;
  }

  /** @brief Declare interleaved independent series in year-major order. */
  bool set_series_count(int count) {
    if (count < 1) Rcpp::stop("GaussianProcess series count must be positive");
    *n_series_m = static_cast<size_t>(count);
    return true;
  }

  template <typename Type>
  void Configure(fims_distributions::GaussianProcessLPDF<Type>& density) {
    density.n_series = *n_series_m;
    density.ar1 = process_m.get() == "ar1";
    density.rw1 = process_m.get() == "rw1";
    density.initial_fixed = initial_rule_m.get() == "fixed";
    density.initial_mean = static_cast<Type>((*initial_mean_m));
    density.log_initial_sd = static_cast<Type>(std::log((*initial_sd_m)));
  }

  bool set_distribution_links(std::string input_type,
                              Rcpp::IntegerVector ids) override {
    if (input_type != "random_effects" || ids.size() < 1 || ids.size() > 2) {
      Rcpp::stop(
          "GaussianProcess requires a process target ID and optional mean ID");
    }
    for (auto id : ids) {
      if (id < 0) Rcpp::stop("GaussianProcess link IDs must be nonnegative");
    }
    this->input_type_m.set(input_type);
    this->key_m->assign(ids.begin(), ids.end());
    return true;
  }

  /** @brief Validate fixed metadata and initial hyperparameter values. */
  void ValidateParameters() {
    if (log_sd.size() != 1 || unconstrained_rho.size() != 1 ||
        !std::isfinite(log_sd[0].initial_value_m) ||
        !std::isfinite(unconstrained_rho[0].initial_value_m)) {
      Rcpp::stop(
          "GaussianProcess requires finite scalar log_sd and "
          "unconstrained_rho");
    }
    for (auto* parameter : {&log_sd, &unconstrained_rho}) {
      const auto type = (*parameter)[0].estimation_type_m.get();
      if (type != "constant" && type != "fixed_effects") {
        Rcpp::stop(
            "GaussianProcess hyperparameters must be constant or "
            "fixed_effects");
      }
    }
    if ((process_m.get() == "iid" || process_m.get() == "rw1") &&
        (unconstrained_rho[0].initial_value_m != 0.0 ||
         unconstrained_rho[0].estimation_type_m.get() != "constant")) {
      Rcpp::stop(
          "IID/random-walk GaussianProcess requires constant zero correlation");
    }
  }

  /** @brief Copy standalone values for direct density evaluation or simulation.
   */
  void SetStandalone(fims_distributions::GaussianProcessLPDF<double>& density) {
    ValidateParameters();
    Configure(density);
    density.log_sd.resize(1);
    density.unconstrained_rho.resize(1);
    density.log_sd[0] = log_sd[0].initial_value_m;
    density.unconstrained_rho[0] = unconstrained_rho[0].initial_value_m;
    density.observed_values.resize(observed_values.size());
    density.expected_values.resize(expected_values.size());
    for (size_t i = 0; i < observed_values.size(); ++i) {
      if (!std::isfinite(observed_values[i].initial_value_m)) {
        Rcpp::stop("GaussianProcess standalone targets must be finite");
      }
      density.observed_values[i] = observed_values[i].initial_value_m;
    }
    for (size_t i = 0; i < expected_values.size(); ++i) {
      if (!std::isfinite(expected_values[i].initial_value_m)) {
        Rcpp::stop("GaussianProcess standalone means must be finite");
      }
      density.expected_values[i] = expected_values[i].initial_value_m;
    }
  }

  double evaluate() override {
    fims_distributions::GaussianProcessLPDF<double> density;
    SetStandalone(density);
    return density.evaluate();
  }

  /** @brief Draw about supplied means using R's RNG, without mutating targets.
   */
  Rcpp::NumericVector simulate() {
    Rcpp::RNGScope scope;
    fims_distributions::GaussianProcessLPDF<double> density;
    SetStandalone(density);
    auto draws = density.Simulate([]() { return R::rnorm(0.0, 1.0); });
    return Rcpp::NumericVector(draws.begin(), draws.end());
  }

  /** @brief Continue supplied targets before a zero-based index, optionally
   * without noise. */
  Rcpp::NumericVector simulate_from(int first, bool stochastic) {
    if (first < 0) Rcpp::stop("Continuation index must be nonnegative");
    Rcpp::RNGScope scope;
    fims_distributions::GaussianProcessLPDF<double> density;
    SetStandalone(density);
    auto draws = density.SimulateFrom(static_cast<size_t>(first), [&]() {
      return stochastic ? R::rnorm(0.0, 1.0) : 0.0;
    });
    return Rcpp::NumericVector(draws.begin(), draws.end());
  }

#ifdef TMB_MODEL
  /** @brief Register covariance parameters once, with the usual FIMS names. */
  template <typename Type>
  void RegisterHyperparameter(
      std::shared_ptr<fims_info::Information<Type>> info,
      VariableVector& source, fims::Vector<Type>& target,
      const std::string& name) {
    target.resize(1);
    target[0] = source[0].initial_value_m;
    if (source[0].estimation_type_m.get() == "fixed_effects") {
      info->RegisterParameterName("gaussian_process." +
                                  fims::to_string(this->id_m) + "." + name +
                                  "." + fims::to_string(source[0].id_m));
      info->RegisterParameter(target[0]);
    }
    info->variable_map[source.id_m] = &target;
  }

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    ValidateParameters();
    auto info = fims_info::Information<Type>::GetInstance();
    if (this->input_type_m.get() != "random_effects" || this->key_m->empty() ||
        this->key_m->size() > 2) {
      Rcpp::stop(
          "GaussianProcess must be linked to a process target before "
          "model creation");
    }
    // Create the owning module before binding its process distribution.
    auto target = info->variable_map.find(this->key_m->at(0));
    if (target == info->variable_map.end() || target->second == nullptr ||
        target->second->size() == 0) {
      Rcpp::stop(
          "GaussianProcess target ID is not a nonempty registered vector");
    }
    if (*n_series_m == 0 || target->second->size() % *n_series_m != 0) {
      Rcpp::stop("GaussianProcess series count must divide target length");
    }
    if (process_m.get() == "rw1" && initial_rule_m.get() == "fixed") {
      if (*n_series_m > target->second->size())
        Rcpp::stop("Invalid process series count");
      for (size_t j = 0; j < *n_series_m; ++j) {
        auto address = &(*target->second)[j];
        if (std::find(info->fixed_effects_parameters.begin(),
                      info->fixed_effects_parameters.end(),
                      address) != info->fixed_effects_parameters.end() ||
            std::find(info->random_effects_parameters.begin(),
                      info->random_effects_parameters.end(),
                      address) != info->random_effects_parameters.end()) {
          Rcpp::stop(
              "A fixed random-walk initial state cannot be registered for "
              "estimation");
        }
      }
    }
    if (this->key_m->size() == 2) {
      auto mean = info->variable_map.find(this->key_m->at(1));
      if (mean == info->variable_map.end() || mean->second == nullptr ||
          (mean->second->size() != 1 &&
           mean->second->size() != target->second->size())) {
        Rcpp::stop(
            "GaussianProcess mean ID must resolve to a scalar or "
            "matching vector");
      }
    } else if (expected_values.size() != 1 &&
               expected_values.size() != target->second->size()) {
      Rcpp::stop(
          "GaussianProcess requires a scalar or matching "
          "expected_values vector");
    }
    auto density =
        std::make_shared<fims_distributions::GaussianProcessLPDF<Type>>();
    density->id = this->id_m;
    Configure(*density);
    density->input_type = "random_effects";
    density->key = *this->key_m;
    density->expected_values.resize(expected_values.size());
    for (size_t i = 0; i < expected_values.size(); ++i) {
      density->expected_values[i] = expected_values[i].initial_value_m;
    }
    RegisterHyperparameter(info, log_sd, density->log_sd, "log_sd");
    RegisterHyperparameter(info, unconstrained_rho, density->unconstrained_rho,
                           "unconstrained_rho");
    info->density_components[density->id] = density;
    return true;
  }

  bool add_to_fims_tmb() override {
    add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }
#endif

  /** @brief Keep final covariance parameters and density values for JSON
   * output. */
  void finalize() override {
    auto info = fims_info::Information<double>::GetInstance();
    auto it = info->density_components.find(this->id_m);
    if (it == info->density_components.end()) return;
    auto density = std::dynamic_pointer_cast<
        fims_distributions::GaussianProcessLPDF<double>>(it->second);
    if (!density) Rcpp::stop("GaussianProcess density type mismatch");
    this->finalized = true;
    this->lpdf_value = density->lpdf;
    log_sd[0].final_value_m = density->log_sd[0];
    unconstrained_rho[0].final_value_m = density->unconstrained_rho[0];
    lpdf_vec = RealVector(density->lpdf_vec.size());
    observed_values.resize(density->lpdf_vec.size());
    expected_values.resize(density->lpdf_vec.size());
    for (size_t i = 0; i < density->lpdf_vec.size(); ++i) {
      lpdf_vec[i] = density->lpdf_vec[i];
      observed_values[i].final_value_m = density->get_observed(i);
      expected_values[i].final_value_m = density->Mean(i);
    }
  }

  std::string to_json() override {
    std::stringstream ss;
    ss << "{\n\"module_name\": \"density\",\n\"module_id\": " << this->id_m
       << ",\n\"module_type\": \"gaussian_process\",\n\"observed_data_id\": "
          "-999,\n\"input_type\": \"random_effects\",\n"
       << "\"process\": \"" << process_m.get() << "\",\n";
    if (process_m.get() == "rw1") {
      const auto precision = ss.precision();
      ss << std::setprecision(std::numeric_limits<double>::max_digits10);
      if (*n_series_m > 1) ss << "\"n_series\": " << *n_series_m << ",\n";
      ss << "\"initial_rule\": \"" << initial_rule_m.get() << "\",\n"
         << "\"initial_mean\": " << (*initial_mean_m) << ",\n"
         << "\"initial_sd\": " << (*initial_sd_m) << ",\n";
      ss.precision(precision);
    }
    ss << this->linked_ids_to_json()
       << this->parameters_to_json(
              {{"log_sd", &log_sd}, {"unconstrained_rho", &unconstrained_rho}})
       << "\"density_component\": {\"lpdf_value\": "
       << sanitize_val(this->lpdf_value) << ", \"value\": [";
    for (size_t i = 0; i < lpdf_vec.size(); ++i) {
      if (i) ss << ", ";
      ss << sanitize_val(lpdf_vec[i]);
    }
    ss << "], \"log_sd_values\": [" << sanitize_val(log_sd[0].final_value_m)
       << "], \"rho_values\": ["
       << (process_m.get() == "rw1" ? 1.0
           : process_m.get() == "ar1"
               ? std::tanh(unconstrained_rho[0].final_value_m)
               : 0.0)
       << "], \"expected_values\": [";
    for (size_t i = 0; i < expected_values.size(); ++i) {
      if (i) ss << ", ";
      ss << sanitize_val(expected_values[i].final_value_m);
    }
    ss << "], \"observed_values\": [";
    for (size_t i = 0; i < observed_values.size(); ++i) {
      if (i) ss << ", ";
      ss << sanitize_val(observed_values[i].final_value_m);
    }
    ss << "]}}\n";
    return ss.str();
  }
};
#endif
