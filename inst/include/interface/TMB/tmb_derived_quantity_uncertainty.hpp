/**
 * @file tmb_derived_quantity_uncertainty.hpp
 * @brief TMB-facing adapter for derived quantity uncertainty calculations.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_TMB_DERIVED_QUANTITY_UNCERTAINTY_HPP
#define FIMS_INTERFACE_TMB_DERIVED_QUANTITY_UNCERTAINTY_HPP

#include "../../common/derived_quantity_uncertainty.hpp"
#include <sstream>
#include <stdexcept>
#include <vector>

namespace fims_tmb {

/**
 * @brief Fixed-effect ADREPORT information extracted from TMB.
 *
 * @details The Jacobian is expected in the same row-major layout returned by
 * TMBad and used internally by TMB before conversion to an R matrix.
 */
struct FixedEffectADReport {
  fims_report::DerivedQuantityReportRequest request;
  fims::Vector<double> estimate;
  fims::Vector<double> jacobian;
  fims::Vector<double> fixed_effect_covariance;
  fims::Vector<int> dims;
  fims::Vector<std::string> dim_names;
  size_t n_fixed_effects = 0;
};

/**
 * @brief Random-effect ADREPORT information after TMB Laplace reduction.
 *
 * @details The adjusted fixed-effect Jacobian includes the reverse-sweep
 * correction used by TMB's sdreport. The random-effect covariance is the
 * inverse random-effect Hessian. All matrices are stored row-major.
 */
struct LaplaceADReport {
  fims_report::DerivedQuantityReportRequest request;
  fims::Vector<double> estimate;
  fims::Vector<double> adjusted_fixed_jacobian;
  fims::Vector<double> fixed_effect_covariance;
  fims::Vector<double> random_jacobian;
  fims::Vector<double> random_effect_covariance;
  fims::Vector<int> dims;
  fims::Vector<std::string> dim_names;
  size_t n_fixed_effects = 0;
  size_t n_random_effects = 0;
};

/**
 * @brief Structured ADREPORT payload extracted from a TMB model.
 *
 * @details This type is the backend boundary for TMB-derived uncertainty. Rcpp
 * can translate an R list into this payload today, and a future TMB API
 * extractor can populate the same payload directly from C++.
 */
struct ADReportPayload {
  std::string method;
  fims_report::DerivedQuantityReportRequest request;
  fims::Vector<double> estimate;
  fims::Vector<double> jacobian;
  fims::Vector<double> fixed_jacobian;
  fims::Vector<double> adjusted_fixed_jacobian;
  fims::Vector<double> fixed_effect_covariance;
  fims::Vector<double> random_jacobian;
  fims::Vector<double> random_effect_hessian;
  fims::Vector<double> random_effect_covariance;
  fims::Vector<int> fixed_indices;
  fims::Vector<int> random_indices;
  fims::Vector<int> dims;
  fims::Vector<std::string> dim_names;
  size_t n_fixed_effects = 0;
  size_t n_random_effects = 0;
};

/**
 * @brief Raw ingredients needed to assemble an ADREPORT payload.
 *
 * @details Index vectors use zero-based C++ positions. The full Jacobian is
 * stored row-major with one row per ADREPORT element and one column per full
 * parameter vector element.
 */
struct ADReportPayloadExtractionInput {
  fims_report::DerivedQuantityReportRequest request;
  fims::Vector<double> estimate;
  fims::Vector<double> jacobian;
  fims::Vector<double> fixed_effect_covariance;
  fims::Vector<double> random_effect_hessian;
  fims::Vector<double> fixed_jacobian_adjustment;
  fims::Vector<int> random_indices;
  fims::Vector<int> dims;
  fims::Vector<std::string> dim_names;
  size_t n_parameters = 0;
};

/**
 * @brief Interface for objects that provide ADREPORT derivative ingredients.
 *
 * @details The current Rcpp implementation can populate a static provider from
 * TMB's R-facing API. A future native TMB provider can implement the same
 * boundary without changing payload extraction or uncertainty calculation.
 */
class ADReportDerivativeProvider {
 public:
  virtual ~ADReportDerivativeProvider() {}

  /**
   * @brief Get raw derivative ingredients for ADREPORT payload extraction.
   *
   * @return ADReportPayloadExtractionInput Raw derivative pieces.
   */
  virtual ADReportPayloadExtractionInput GetExtractionInput() const = 0;
};

/**
 * @brief Provider backed by an already assembled derivative bundle.
 */
class StaticADReportDerivativeProvider : public ADReportDerivativeProvider {
 public:
  /**
   * @brief Construct a provider from raw extraction input.
   *
   * @param input Raw derivative pieces.
   */
  explicit StaticADReportDerivativeProvider(
      const ADReportPayloadExtractionInput& input)
      : input_m(input) {}

  /**
   * @brief Get raw derivative ingredients for ADREPORT payload extraction.
   *
   * @return ADReportPayloadExtractionInput Raw derivative pieces.
   */
  ADReportPayloadExtractionInput GetExtractionInput() const override {
    return input_m;
  }

 private:
  ADReportPayloadExtractionInput input_m;
};

/**
 * @brief Interface for native TMB objects that can expose ADREPORT derivatives.
 */
class NativeTMBADReportHandle {
 public:
  virtual ~NativeTMBADReportHandle() {}

  /**
   * @brief Get raw derivative ingredients from a native TMB object.
   *
   * @return ADReportPayloadExtractionInput Raw derivative pieces.
   */
  virtual ADReportPayloadExtractionInput GetADReportExtractionInput() const = 0;
};

/**
 * @brief Native handle backed by a TMB ADREPORT ADFun-like object.
 *
 * @details The AD function is expected to expose `Domain()`, `operator()`, and
 * `Jacobian()` with row-major Jacobian storage. This matches TMBad ADFun and
 * mirrors the current R-facing ADREPORT extractor without owning the TMB
 * function object's lifetime.
 *
 * @tparam ADFunType TMB-compatible AD function type.
 */
template <class ADFunType>
class TMBADFunADReportHandle : public NativeTMBADReportHandle {
 public:
  /**
   * @brief Construct an ADREPORT handle from an ADFun-like object.
   *
   * @param adreport_function Native ADREPORT function.
   * @param parameters Full parameter vector used to evaluate the report.
   * @param fixed_effect_covariance Fixed-effect covariance matrix, row-major.
   */
  TMBADFunADReportHandle(
      ADFunType& adreport_function,
      const fims::Vector<double>& parameters,
      const fims::Vector<double>& fixed_effect_covariance)
      : adreport_function_m(&adreport_function),
        parameters_m(parameters),
        fixed_effect_covariance_m(fixed_effect_covariance) {}

  /**
   * @brief Add random-effect ingredients for Laplace ADREPORT payloads.
   *
   * @param random_indices Zero-based random-effect positions.
   * @param random_effect_hessian Random-effect Hessian matrix, row-major.
   * @param fixed_jacobian_adjustment Reverse-sweep fixed Jacobian adjustment,
   * row-major.
   */
  void SetRandomEffectInputs(
      const fims::Vector<int>& random_indices,
      const fims::Vector<double>& random_effect_hessian,
      const fims::Vector<double>& fixed_jacobian_adjustment) {
    random_indices_m = random_indices;
    random_effect_hessian_m = random_effect_hessian;
    fixed_jacobian_adjustment_m = fixed_jacobian_adjustment;
  }

  /**
   * @brief Get raw derivative ingredients from the native ADREPORT function.
   *
   * @return ADReportPayloadExtractionInput Raw derivative pieces.
   */
  ADReportPayloadExtractionInput GetADReportExtractionInput()
      const override {
    if (adreport_function_m == nullptr) {
      throw std::invalid_argument(
          "TMBADFunADReportHandle requires a non-null ADREPORT function");
    }

    std::vector<double> parameters(parameters_m.size());
    for (size_t i = 0; i < parameters_m.size(); i++) {
      parameters[i] = parameters_m[i];
    }
    if (parameters.size() != adreport_function_m->Domain()) {
      std::ostringstream error;
      error << "TMBADFunADReportHandle parameter vector size ("
            << parameters.size()
            << ") does not match ADREPORT function domain ("
            << adreport_function_m->Domain() << ")";
      throw std::invalid_argument(error.str());
    }

    ADReportPayloadExtractionInput input;
    input.estimate =
        fims::Vector<double>((*adreport_function_m)(parameters));
    input.jacobian =
        fims::Vector<double>(adreport_function_m->Jacobian(parameters));
    input.fixed_effect_covariance = fixed_effect_covariance_m;
    input.random_indices = random_indices_m;
    input.random_effect_hessian = random_effect_hessian_m;
    input.fixed_jacobian_adjustment = fixed_jacobian_adjustment_m;
    input.n_parameters = parameters.size();
    return input;
  }

 private:
  ADFunType* adreport_function_m = nullptr;
  fims::Vector<double> parameters_m;
  fims::Vector<double> fixed_effect_covariance_m;
  fims::Vector<int> random_indices_m;
  fims::Vector<double> random_effect_hessian_m;
  fims::Vector<double> fixed_jacobian_adjustment_m;
};

/**
 * @brief Placeholder for a future provider backed by native TMB handles.
 *
 * @details TMB currently exposes the pieces used here through the R-facing
 * MakeADFun/sdreport path. This provider marks the C++ API FIMS will use once
 * native TMB handle access is available.
 */
class NativeTMBADReportDerivativeProvider : public ADReportDerivativeProvider {
 public:
  /**
   * @brief Construct a native TMB provider stub.
   *
   * @param model_handle Pointer to a future native TMB model or ADFun handle.
   */
  explicit NativeTMBADReportDerivativeProvider(
      void* model_handle = nullptr)
      : model_handle_m(model_handle), native_handle_m(nullptr) {}

  /**
   * @brief Construct a native TMB provider from a typed native handle.
   *
   * @param native_handle Native object exposing ADREPORT derivative ingredients.
   */
  explicit NativeTMBADReportDerivativeProvider(
      const NativeTMBADReportHandle& native_handle)
      : model_handle_m(nullptr), native_handle_m(&native_handle) {}

  /**
   * @brief Get raw derivative ingredients for ADREPORT payload extraction.
   *
   * @return ADReportPayloadExtractionInput Raw derivative pieces.
   */
  ADReportPayloadExtractionInput GetExtractionInput() const override {
    if (native_handle_m != nullptr) {
      return native_handle_m->GetADReportExtractionInput();
    }

    throw std::logic_error(
        "NativeTMBADReportDerivativeProvider::GetExtractionInput is not "
        "implemented until native TMB ADREPORT handle access is available");
  }

  /**
   * @brief Check whether a raw native TMB model handle was provided.
   *
   * @return bool True when a non-null raw model handle is stored.
   */
  bool HasModelHandle() const { return model_handle_m != nullptr; }

  /**
   * @brief Check whether a typed native handle is stored.
   *
   * @return bool True when a typed native handle is available.
   */
  bool HasNativeHandle() const { return native_handle_m != nullptr; }

  /**
   * @brief Check whether any native handle is stored.
   *
   * @return bool True when a raw or typed native handle is available.
   */
  bool HasAnyHandle() const {
    return model_handle_m != nullptr || native_handle_m != nullptr;
  }

 private:
  void* model_handle_m = nullptr;
  const NativeTMBADReportHandle* native_handle_m = nullptr;
};

/**
 * @brief Assemble backend ADREPORT payloads from raw TMB derivative pieces.
 */
class ADReportPayloadExtractor {
 public:
  /**
   * @brief Extract an ADREPORT payload from a derivative provider.
   *
   * @param provider ADREPORT derivative provider.
   * @return ADReportPayload Structured payload for backend uncertainty.
   */
  ADReportPayload Extract(const ADReportDerivativeProvider& provider) const {
    return this->Extract(provider.GetExtractionInput());
  }

  /**
   * @brief Extract fixed and random-effect pieces from raw ADREPORT derivatives.
   *
   * @param input Raw ADREPORT derivative pieces.
   * @return ADReportPayload Structured payload for backend uncertainty.
   */
  ADReportPayload Extract(const ADReportPayloadExtractionInput& input) const {
    this->Validate(input);

    ADReportPayload output;
    output.request = input.request;
    output.estimate = input.estimate;
    output.fixed_effect_covariance = input.fixed_effect_covariance;
    output.dims = input.dims;
    output.dim_names = input.dim_names;

    if (input.random_indices.size() == 0) {
      output.method = "fixed";
      output.jacobian = input.jacobian;
      output.n_fixed_effects = input.n_parameters;
      output.n_random_effects = 0;
      for (size_t i = 0; i < input.n_parameters; i++) {
        output.fixed_indices.push_back(static_cast<int>(i));
      }
      return output;
    }

    fims::Vector<int> fixed_indices = this->FixedIndices(
        input.n_parameters, input.random_indices);
    output.fixed_jacobian = this->SelectColumns(
        input.jacobian, input.estimate.size(), input.n_parameters,
        fixed_indices);
    output.random_jacobian = this->SelectColumns(
        input.jacobian, input.estimate.size(), input.n_parameters,
        input.random_indices);
    output.n_fixed_effects = fixed_indices.size();
    output.n_random_effects = input.random_indices.size();
    output.fixed_indices = fixed_indices;
    output.random_indices = input.random_indices;

    if (this->AllNearZero(output.random_jacobian)) {
      output.method = "fixed_after_laplace";
      return output;
    }

    output.method = "laplace";
    output.random_effect_hessian = input.random_effect_hessian;
    output.random_effect_covariance = this->InvertSquareMatrix(
        input.random_effect_hessian, output.n_random_effects);
    output.adjusted_fixed_jacobian = output.fixed_jacobian;
    for (size_t i = 0; i < output.adjusted_fixed_jacobian.size(); i++) {
      output.adjusted_fixed_jacobian[i] += input.fixed_jacobian_adjustment[i];
    }

    return output;
  }

 private:
  /**
   * @brief Validate extraction inputs.
   *
   * @param input Raw extraction inputs.
   */
  void Validate(const ADReportPayloadExtractionInput& input) const {
    if (input.estimate.size() == 0) {
      throw std::invalid_argument(
          "ADReportPayloadExtractor::Validate: estimate cannot be empty");
    }
    if (input.n_parameters == 0) {
      throw std::invalid_argument(
          "ADReportPayloadExtractor::Validate: n_parameters must be greater "
          "than zero");
    }
    if (input.jacobian.size() != input.estimate.size() * input.n_parameters) {
      std::ostringstream ss;
      ss << "ADReportPayloadExtractor::Validate: jacobian size "
         << input.jacobian.size() << " does not match estimate size "
         << input.estimate.size() << " times n_parameters "
         << input.n_parameters;
      throw std::invalid_argument(ss.str());
    }

    std::vector<bool> is_random(input.n_parameters, false);
    for (size_t i = 0; i < input.random_indices.size(); i++) {
      if (input.random_indices[i] < 0 ||
          static_cast<size_t>(input.random_indices[i]) >= input.n_parameters) {
        throw std::invalid_argument(
            "ADReportPayloadExtractor::Validate: random index is out of "
            "bounds");
      }
      if (is_random[static_cast<size_t>(input.random_indices[i])]) {
        throw std::invalid_argument(
            "ADReportPayloadExtractor::Validate: random indices cannot contain "
            "duplicates");
      }
      is_random[static_cast<size_t>(input.random_indices[i])] = true;
    }

    const size_t n_fixed_effects =
        input.n_parameters - input.random_indices.size();
    if (n_fixed_effects == 0) {
      throw std::invalid_argument(
          "ADReportPayloadExtractor::Validate: at least one fixed effect is "
          "required");
    }
    if (input.fixed_effect_covariance.size() !=
        n_fixed_effects * n_fixed_effects) {
      std::ostringstream ss;
      ss << "ADReportPayloadExtractor::Validate: fixed covariance size "
         << input.fixed_effect_covariance.size()
         << " does not match n_fixed_effects squared "
         << n_fixed_effects * n_fixed_effects;
      throw std::invalid_argument(ss.str());
    }

    if (input.random_indices.size() == 0) {
      return;
    }

    if (input.random_effect_hessian.size() !=
        input.random_indices.size() * input.random_indices.size()) {
      throw std::invalid_argument(
          "ADReportPayloadExtractor::Validate: random Hessian size is invalid");
    }
    if (input.fixed_jacobian_adjustment.size() !=
        input.estimate.size() * n_fixed_effects) {
      throw std::invalid_argument(
          "ADReportPayloadExtractor::Validate: fixed Jacobian adjustment size "
          "is invalid");
    }
  }

  /**
   * @brief Get fixed-effect indices as complement of random-effect indices.
   */
  fims::Vector<int> FixedIndices(
      size_t n_parameters, const fims::Vector<int>& random_indices) const {
    std::vector<bool> is_random(n_parameters, false);
    for (size_t i = 0; i < random_indices.size(); i++) {
      is_random[static_cast<size_t>(random_indices[i])] = true;
    }

    fims::Vector<int> fixed_indices;
    for (size_t i = 0; i < n_parameters; i++) {
      if (!is_random[i]) {
        fixed_indices.push_back(static_cast<int>(i));
      }
    }
    return fixed_indices;
  }

  /**
   * @brief Select matrix columns from a row-major matrix.
   */
  fims::Vector<double> SelectColumns(
      const fims::Vector<double>& matrix, size_t n_rows, size_t n_cols,
      const fims::Vector<int>& columns) const {
    fims::Vector<double> output;
    output.reserve(n_rows * columns.size());
    for (size_t i = 0; i < n_rows; i++) {
      for (size_t j = 0; j < columns.size(); j++) {
        output.push_back(
            static_cast<double>(
                matrix[(i * n_cols) + static_cast<size_t>(columns[j])]));
      }
    }
    return output;
  }

  /**
   * @brief Check whether all values are approximately zero.
   */
  bool AllNearZero(const fims::Vector<double>& values) const {
    for (size_t i = 0; i < values.size(); i++) {
      if (std::fabs(values[i]) > 1.0e-14) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Invert a dense row-major square matrix.
   */
  fims::Vector<double> InvertSquareMatrix(
      const fims::Vector<double>& matrix, size_t n) const {
    fims::Vector<double> work(n * 2 * n, 0.0);
    for (size_t i = 0; i < n; i++) {
      for (size_t j = 0; j < n; j++) {
        work[(i * 2 * n) + j] = matrix[(i * n) + j];
      }
      work[(i * 2 * n) + n + i] = 1.0;
    }

    for (size_t pivot_col = 0; pivot_col < n; pivot_col++) {
      size_t pivot_row = pivot_col;
      double pivot_abs = std::fabs(work[(pivot_col * 2 * n) + pivot_col]);
      for (size_t row = pivot_col + 1; row < n; row++) {
        const double candidate_abs =
            std::fabs(work[(row * 2 * n) + pivot_col]);
        if (candidate_abs > pivot_abs) {
          pivot_abs = candidate_abs;
          pivot_row = row;
        }
      }

      if (pivot_abs <= 1.0e-14) {
        throw std::runtime_error(
            "ADReportPayloadExtractor::InvertSquareMatrix: matrix is "
            "singular");
      }

      if (pivot_row != pivot_col) {
        for (size_t col = 0; col < 2 * n; col++) {
          const double tmp = work[(pivot_col * 2 * n) + col];
          work[(pivot_col * 2 * n) + col] = work[(pivot_row * 2 * n) + col];
          work[(pivot_row * 2 * n) + col] = tmp;
        }
      }

      const double pivot = work[(pivot_col * 2 * n) + pivot_col];
      for (size_t col = 0; col < 2 * n; col++) {
        work[(pivot_col * 2 * n) + col] /= pivot;
      }

      for (size_t row = 0; row < n; row++) {
        if (row == pivot_col) {
          continue;
        }
        const double factor = work[(row * 2 * n) + pivot_col];
        for (size_t col = 0; col < 2 * n; col++) {
          work[(row * 2 * n) + col] -=
              factor * work[(pivot_col * 2 * n) + col];
        }
      }
    }

    fims::Vector<double> inverse;
    inverse.reserve(n * n);
    for (size_t i = 0; i < n; i++) {
      for (size_t j = 0; j < n; j++) {
        inverse.push_back(static_cast<double>(work[(i * 2 * n) + n + j]));
      }
    }
    return inverse;
  }
};

/**
 * @brief Adapter from TMB fixed-effect ADREPORT output to FIMS uncertainty.
 */
class FixedEffectADReportUncertaintyAdapter {
 public:
  typedef std::vector<FixedEffectADReport> report_vector;
  typedef std::vector<fims_report::DerivedQuantityEstimate> estimate_vector;

  /**
   * @brief Calculate SEs from one TMB fixed-effect ADREPORT payload.
   *
   * @param report ADREPORT values, Jacobian, and fixed-effect covariance.
   * @return fims_report::DerivedQuantityEstimate Estimate and SEs.
   */
  fims_report::DerivedQuantityEstimate Calculate(
      const FixedEffectADReport& report) const {
    fims_report::DeltaMethodInput input;
    input.request = report.request;
    input.estimate = report.estimate;
    input.jacobian = report.jacobian;
    input.covariance = report.fixed_effect_covariance;
    input.dims = report.dims;
    input.dim_names = report.dim_names;
    input.n_parameters = report.n_fixed_effects;

    return this->calculator_m.Calculate(input);
  }

  /**
   * @brief Calculate SEs from multiple TMB fixed-effect ADREPORT payloads.
   *
   * @param reports ADREPORT payloads.
   * @return estimate_vector Estimate and SEs for each payload.
   */
  estimate_vector Calculate(const report_vector& reports) const {
    estimate_vector estimates;
    estimates.reserve(reports.size());
    for (size_t i = 0; i < reports.size(); i++) {
      estimates.push_back(this->Calculate(reports[i]));
    }
    return estimates;
  }

 private:
  fims_report::DeltaMethodUncertaintyCalculator calculator_m;
};

/**
 * @brief Adapter from TMB Laplace ADREPORT output to FIMS uncertainty.
 */
class LaplaceADReportUncertaintyAdapter {
 public:
  typedef std::vector<LaplaceADReport> report_vector;
  typedef std::vector<fims_report::DerivedQuantityEstimate> estimate_vector;

  /**
   * @brief Calculate SEs from one TMB Laplace ADREPORT payload.
   *
   * @param report ADREPORT values and Laplace delta-method ingredients.
   * @return fims_report::DerivedQuantityEstimate Estimate and SEs.
   */
  fims_report::DerivedQuantityEstimate Calculate(
      const LaplaceADReport& report) const {
    this->Validate(report);

    fims_report::DerivedQuantityEstimate output;
    output.request = report.request;
    output.estimate = report.estimate;
    output.dims = report.dims;
    output.dim_names = report.dim_names;
    output.se.resize(report.estimate.size());

    for (size_t i = 0; i < report.estimate.size(); i++) {
      double variance = 0.0;
      for (size_t j = 0; j < report.n_random_effects; j++) {
        const double d_i_j =
            report.random_jacobian[(i * report.n_random_effects) + j];
        for (size_t k = 0; k < report.n_random_effects; k++) {
          variance += d_i_j *
                      report.random_effect_covariance
                          [(j * report.n_random_effects) + k] *
                      report.random_jacobian
                          [(i * report.n_random_effects) + k];
        }
      }

      for (size_t j = 0; j < report.n_fixed_effects; j++) {
        const double d_i_j =
            report.adjusted_fixed_jacobian
                [(i * report.n_fixed_effects) + j];
        for (size_t k = 0; k < report.n_fixed_effects; k++) {
          variance += d_i_j *
                      report.fixed_effect_covariance
                          [(j * report.n_fixed_effects) + k] *
                      report.adjusted_fixed_jacobian
                          [(i * report.n_fixed_effects) + k];
        }
      }

      if (variance < 0.0 && variance > -1.0e-12) {
        variance = 0.0;
      }
      if (variance < 0.0) {
        std::ostringstream ss;
        ss << "LaplaceADReportUncertaintyAdapter::Calculate: negative "
           << "variance for element " << i << ": " << variance;
        throw std::runtime_error(ss.str());
      }
      output.se[i] = std::sqrt(variance);
    }

    return output;
  }

  /**
   * @brief Calculate SEs from multiple TMB Laplace ADREPORT payloads.
   *
   * @param reports ADREPORT payloads.
   * @return estimate_vector Estimate and SEs for each payload.
   */
  estimate_vector Calculate(const report_vector& reports) const {
    estimate_vector estimates;
    estimates.reserve(reports.size());
    for (size_t i = 0; i < reports.size(); i++) {
      estimates.push_back(this->Calculate(reports[i]));
    }
    return estimates;
  }

 private:
  /**
   * @brief Validate TMB Laplace ADREPORT inputs.
   *
   * @param report ADREPORT payload.
   */
  void Validate(const LaplaceADReport& report) const {
    if (report.estimate.size() == 0) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: estimate cannot be "
          "empty");
    }
    if (report.n_fixed_effects == 0) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: n_fixed_effects must "
          "be greater than zero");
    }
    if (report.n_random_effects == 0) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: n_random_effects must "
          "be greater than zero");
    }
    if (report.adjusted_fixed_jacobian.size() !=
        report.estimate.size() * report.n_fixed_effects) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: adjusted fixed "
          "Jacobian size is invalid");
    }
    if (report.fixed_effect_covariance.size() !=
        report.n_fixed_effects * report.n_fixed_effects) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: fixed covariance size "
          "is invalid");
    }
    if (report.random_jacobian.size() !=
        report.estimate.size() * report.n_random_effects) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: random Jacobian size "
          "is invalid");
    }
    if (report.random_effect_covariance.size() !=
        report.n_random_effects * report.n_random_effects) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: random covariance "
          "size is invalid");
    }
  }
};

/**
 * @brief Dispatch ADREPORT payloads to the correct backend uncertainty adapter.
 */
class ADReportPayloadUncertaintyCalculator {
 public:
  /**
   * @brief Calculate standard errors from a structured ADREPORT payload.
   *
   * @param payload ADREPORT payload.
   * @return fims_report::DerivedQuantityEstimate Estimate and SEs.
   */
  fims_report::DerivedQuantityEstimate Calculate(
      const ADReportPayload& payload) const {
    if (payload.method == "laplace") {
      LaplaceADReport report;
      report.request = payload.request;
      report.estimate = payload.estimate;
      report.adjusted_fixed_jacobian = payload.adjusted_fixed_jacobian;
      report.fixed_effect_covariance = payload.fixed_effect_covariance;
      report.random_jacobian = payload.random_jacobian;
      report.random_effect_covariance = payload.random_effect_covariance;
      report.dims = payload.dims;
      report.dim_names = payload.dim_names;
      report.n_fixed_effects = payload.n_fixed_effects;
      report.n_random_effects = payload.n_random_effects;

      LaplaceADReportUncertaintyAdapter adapter;
      return adapter.Calculate(report);
    }

    FixedEffectADReport report;
    report.request = payload.request;
    report.estimate = payload.estimate;
    if (payload.method == "fixed_after_laplace") {
      report.jacobian = payload.fixed_jacobian;
    } else if (payload.method == "fixed") {
      report.jacobian = payload.jacobian;
    } else {
      std::ostringstream ss;
      ss << "ADReportPayloadUncertaintyCalculator::Calculate: unsupported "
         << "method '" << payload.method << "'";
      throw std::invalid_argument(ss.str());
    }
    report.fixed_effect_covariance = payload.fixed_effect_covariance;
    report.dims = payload.dims;
    report.dim_names = payload.dim_names;
    report.n_fixed_effects = payload.n_fixed_effects;

    FixedEffectADReportUncertaintyAdapter adapter;
    return adapter.Calculate(report);
  }
};

}  // namespace fims_tmb

#endif
