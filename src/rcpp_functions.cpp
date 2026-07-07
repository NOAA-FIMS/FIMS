
/**
 * \file rcpp_functions.cpp
 * \brief Implementation of Rcpp function interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_math.hpp"
#include "../inst/include/interface/TMB/tmb_derived_quantity_uncertainty.hpp"
#include <Rcpp.h>
#include <cmath>
#include <limits>

namespace {

std::vector<double> NumericMatrixToRowMajor(const Rcpp::NumericMatrix& matrix) {
  std::vector<double> output;
  output.reserve(static_cast<size_t>(matrix.nrow() * matrix.ncol()));
  for (int i = 0; i < matrix.nrow(); i++) {
    for (int j = 0; j < matrix.ncol(); j++) {
      output.push_back(matrix(i, j));
    }
  }
  return output;
}

Rcpp::NumericVector DerivedQuantitySEToNumericVector(
    const fims_report::DerivedQuantityEstimate& output) {
  Rcpp::NumericVector se(output.se.size());
  for (size_t i = 0; i < output.se.size(); i++) {
    se[i] = output.se[i];
  }
  return se;
}

Rcpp::NumericVector FimsVectorToNumericVector(
    const fims::Vector<double>& values) {
  Rcpp::NumericVector output(values.size());
  for (size_t i = 0; i < values.size(); i++) {
    output[i] = values[i];
  }
  return output;
}

Rcpp::IntegerVector FimsVectorToIntegerVector(
    const fims::Vector<int>& values, bool one_based = false) {
  Rcpp::IntegerVector output(values.size());
  for (size_t i = 0; i < values.size(); i++) {
    output[i] = values[i] + (one_based ? 1 : 0);
  }
  return output;
}

Rcpp::NumericMatrix FimsVectorToNumericMatrix(
    const fims::Vector<double>& values, size_t n_rows, size_t n_cols) {
  Rcpp::NumericMatrix output(n_rows, n_cols);
  if (values.size() == 0) {
    return output;
  }
  if (values.size() != n_rows * n_cols) {
    Rcpp::stop("Cannot convert vector to matrix: dimensions do not match");
  }
  for (size_t i = 0; i < n_rows; i++) {
    for (size_t j = 0; j < n_cols; j++) {
      output(i, j) = values[(i * n_cols) + j];
    }
  }
  return output;
}

fims::Vector<double> InvertSquareMatrix(
    const fims::Vector<double>& matrix, size_t n,
    const std::string& caller_name) {
  if (matrix.size() != n * n) {
    Rcpp::stop("%s: matrix size does not match dimensions",
               caller_name.c_str());
  }

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
      Rcpp::stop("%s: matrix is singular", caller_name.c_str());
    }

    if (pivot_row != pivot_col) {
      for (size_t col = 0; col < 2 * n; col++) {
        const double tmp = work[(pivot_col * 2 * n) + col];
        work[(pivot_col * 2 * n) + col] =
            work[(pivot_row * 2 * n) + col];
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

fims::Vector<int> RandomIndicesFromRcppList(Rcpp::List payload) {
  fims::Vector<int> output;
  if (!payload.containsElementNamed("random_indices")) {
    return output;
  }

  Rcpp::IntegerVector random_indices = payload["random_indices"];
  for (int i = 0; i < random_indices.size(); i++) {
    output.push_back(static_cast<int>(random_indices[i]));
  }
  return output;
}

Rcpp::List TMBADFunEvaluationControl(int order) {
  return Rcpp::List::create(
      Rcpp::Named("order") = order,
      Rcpp::Named("data_changed") = 0,
      Rcpp::Named("set_tail") = 0,
      Rcpp::Named("rangecomponent") = 1,
      Rcpp::Named("hessiancols") = Rcpp::IntegerVector(),
      Rcpp::Named("hessianrows") = Rcpp::IntegerVector(),
      Rcpp::Named("sparsitypattern") = 0,
      Rcpp::Named("dumpstack") = 0);
}

Rcpp::List TMBADFunReverseSweepControl(
    const fims::Vector<double>& weights) {
  return Rcpp::List::create(
      Rcpp::Named("order") = 1,
      Rcpp::Named("data_changed") = 0,
      Rcpp::Named("set_tail") = 0,
      Rcpp::Named("rangecomponent") = 1,
      Rcpp::Named("hessiancols") = Rcpp::IntegerVector(),
      Rcpp::Named("hessianrows") = Rcpp::IntegerVector(),
      Rcpp::Named("rangeweight") = FimsVectorToNumericVector(weights),
      Rcpp::Named("doforward") = 0);
}

Rcpp::List ADReportPayloadToRcppList(
    const fims_tmb::ADReportPayload& payload) {
  const size_t n_estimates = payload.estimate.size();

  Rcpp::List output = Rcpp::List::create(
      Rcpp::Named("backend") = "TMB",
      Rcpp::Named("method") = payload.method,
      Rcpp::Named("estimate") = FimsVectorToNumericVector(payload.estimate),
      Rcpp::Named("fixed_covariance") = FimsVectorToNumericMatrix(
          payload.fixed_effect_covariance, payload.n_fixed_effects,
          payload.n_fixed_effects),
      Rcpp::Named("fixed_indices") =
          FimsVectorToIntegerVector(payload.fixed_indices, true),
      Rcpp::Named("random_indices") =
          FimsVectorToIntegerVector(payload.random_indices, true),
      Rcpp::Named("n_fixed_effects") =
          static_cast<int>(payload.n_fixed_effects),
      Rcpp::Named("n_random_effects") =
          static_cast<int>(payload.n_random_effects));

  if (payload.method == "fixed") {
    output["jacobian"] = FimsVectorToNumericMatrix(
        payload.jacobian, n_estimates, payload.n_fixed_effects);
  } else {
    output["fixed_jacobian"] = FimsVectorToNumericMatrix(
        payload.fixed_jacobian, n_estimates, payload.n_fixed_effects);
    output["random_jacobian"] = FimsVectorToNumericMatrix(
        payload.random_jacobian, n_estimates, payload.n_random_effects);

    if (payload.method == "laplace") {
      output["adjusted_fixed_jacobian"] = FimsVectorToNumericMatrix(
          payload.adjusted_fixed_jacobian, n_estimates,
          payload.n_fixed_effects);
      output["random_hessian"] = FimsVectorToNumericMatrix(
          payload.random_effect_hessian, payload.n_random_effects,
          payload.n_random_effects);
      output["random_covariance"] = FimsVectorToNumericMatrix(
          payload.random_effect_covariance, payload.n_random_effects,
          payload.n_random_effects);
    }
  }

  return output;
}

class RFunctionLaplaceReverseSweepProvider
    : public fims_tmb::LaplaceReverseSweepProvider {
 public:
  explicit RFunctionLaplaceReverseSweepProvider(
      Rcpp::Function reverse_sweep_function)
      : reverse_sweep_function_m(reverse_sweep_function) {}

  fims::Vector<double> GetFixedEffectReverseSweep(
      const fims::Vector<double>& weights,
      const fims::Vector<int>& fixed_indices) const override {
    Rcpp::NumericVector r_weights = FimsVectorToNumericVector(weights);
    Rcpp::IntegerVector r_fixed_indices =
        FimsVectorToIntegerVector(fixed_indices, true);
    Rcpp::NumericVector fixed_reverse =
        reverse_sweep_function_m(r_weights, r_fixed_indices);
    return fims::Vector<double>(
        Rcpp::as<std::vector<double>>(fixed_reverse));
  }

 private:
  Rcpp::Function reverse_sweep_function_m;
};

class TMBADFunLaplaceReverseSweepProvider
    : public fims_tmb::LaplaceReverseSweepProvider {
 public:
  TMBADFunLaplaceReverseSweepProvider(
      SEXP adgrad_ptr, Rcpp::NumericVector parameters)
      : adgrad_ptr_m(adgrad_ptr), parameters_m(parameters) {}

  fims::Vector<double> GetFixedEffectReverseSweep(
      const fims::Vector<double>& weights,
      const fims::Vector<int>& fixed_indices) const override {
#ifdef TMBAD_FRAMEWORK
    Rcpp::NumericVector full_reverse = Rcpp::as<Rcpp::NumericVector>(
        EvalADFunObject(adgrad_ptr_m, parameters_m,
                        TMBADFunReverseSweepControl(weights)));
    fims::Vector<double> output;
    output.reserve(fixed_indices.size());
    for (size_t i = 0; i < fixed_indices.size(); i++) {
      output.push_back(static_cast<double>(
          full_reverse[static_cast<size_t>(fixed_indices[i])]));
    }
    return output;
#else
    throw std::logic_error(
        "TMBADFunLaplaceReverseSweepProvider requires TMBAD_FRAMEWORK");
#endif
  }

 private:
  SEXP adgrad_ptr_m;
  Rcpp::NumericVector parameters_m;
};

fims_tmb::ADReportPayload ADReportPayloadFromRcppList(Rcpp::List payload) {
  if (!payload.containsElementNamed("method")) {
    Rcpp::stop("payload must contain a method element");
  }

  fims_tmb::ADReportPayload output;
  output.method = Rcpp::as<std::string>(payload["method"]);

  Rcpp::NumericVector estimate = payload["estimate"];
  Rcpp::NumericMatrix fixed_covariance = payload["fixed_covariance"];
  output.estimate =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(estimate));
  output.fixed_effect_covariance =
      fims::Vector<double>(NumericMatrixToRowMajor(fixed_covariance));
  output.n_fixed_effects = static_cast<size_t>(fixed_covariance.nrow());

  if (output.method == "laplace") {
    Rcpp::NumericMatrix adjusted_fixed_jacobian =
        payload["adjusted_fixed_jacobian"];
    Rcpp::NumericMatrix random_jacobian = payload["random_jacobian"];
    Rcpp::NumericMatrix random_covariance = payload["random_covariance"];

    output.adjusted_fixed_jacobian =
        fims::Vector<double>(NumericMatrixToRowMajor(adjusted_fixed_jacobian));
    output.random_jacobian =
        fims::Vector<double>(NumericMatrixToRowMajor(random_jacobian));
    output.random_effect_covariance =
        fims::Vector<double>(NumericMatrixToRowMajor(random_covariance));
    output.n_random_effects = static_cast<size_t>(random_covariance.nrow());

    return output;
  }

  if (output.method == "fixed_after_laplace") {
    Rcpp::NumericMatrix fixed_jacobian =
        Rcpp::as<Rcpp::NumericMatrix>(payload["fixed_jacobian"]);
    output.fixed_jacobian =
        fims::Vector<double>(NumericMatrixToRowMajor(fixed_jacobian));
    return output;
  }

  if (output.method == "fixed") {
    Rcpp::NumericMatrix jacobian =
        Rcpp::as<Rcpp::NumericMatrix>(payload["jacobian"]);
    output.jacobian = fims::Vector<double>(NumericMatrixToRowMajor(jacobian));
    return output;
  }

  Rcpp::stop("Unsupported ADREPORT payload method: %s", output.method.c_str());
}

}  // namespace

/**
 * @brief Calculate derived quantity standard errors using the FIMS backend
 * delta-method calculator.
 *
 * @param estimate Derived quantity estimates.
 * @param jacobian Flattened row-major Jacobian.
 * @param covariance Flattened row-major parameter covariance matrix.
 * @param n_parameters Number of parameters.
 * @return Rcpp::NumericVector Standard errors.
 */
Rcpp::NumericVector calculate_derived_quantity_se(
    Rcpp::NumericVector estimate, Rcpp::NumericVector jacobian,
    Rcpp::NumericVector covariance, int n_parameters) {
  if (n_parameters < 1) {
    Rcpp::stop("n_parameters must be greater than zero");
  }

  fims_tmb::FixedEffectADReport report;
  report.estimate =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(estimate));
  report.jacobian =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(jacobian));
  report.fixed_effect_covariance =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(covariance));
  report.n_fixed_effects = static_cast<size_t>(n_parameters);

  fims_tmb::FixedEffectADReportUncertaintyAdapter adapter;
  fims_report::DerivedQuantityEstimate output = adapter.Calculate(report);

  return DerivedQuantitySEToNumericVector(output);
}

/**
 * @brief Calculate derived quantity standard errors using TMB Laplace
 * random-effect ingredients.
 *
 * @param estimate Derived quantity estimates.
 * @param adjusted_fixed_jacobian Flattened row-major adjusted fixed Jacobian.
 * @param fixed_covariance Flattened row-major fixed-effect covariance matrix.
 * @param random_jacobian Flattened row-major random-effect Jacobian.
 * @param random_covariance Flattened row-major inverse random Hessian.
 * @param n_fixed_effects Number of fixed-effect parameters.
 * @param n_random_effects Number of random-effect parameters.
 * @return Rcpp::NumericVector Standard errors.
 */
Rcpp::NumericVector calculate_derived_quantity_laplace_se(
    Rcpp::NumericVector estimate, Rcpp::NumericVector adjusted_fixed_jacobian,
    Rcpp::NumericVector fixed_covariance, Rcpp::NumericVector random_jacobian,
    Rcpp::NumericVector random_covariance, int n_fixed_effects,
    int n_random_effects) {
  if (n_fixed_effects < 1) {
    Rcpp::stop("n_fixed_effects must be greater than zero");
  }
  if (n_random_effects < 1) {
    Rcpp::stop("n_random_effects must be greater than zero");
  }

  fims_tmb::LaplaceADReport report;
  report.estimate =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(estimate));
  report.adjusted_fixed_jacobian =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(
          adjusted_fixed_jacobian));
  report.fixed_effect_covariance =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(fixed_covariance));
  report.random_jacobian =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(random_jacobian));
  report.random_effect_covariance =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(random_covariance));
  report.n_fixed_effects = static_cast<size_t>(n_fixed_effects);
  report.n_random_effects = static_cast<size_t>(n_random_effects);

  fims_tmb::LaplaceADReportUncertaintyAdapter adapter;
  fims_report::DerivedQuantityEstimate output = adapter.Calculate(report);

  return DerivedQuantitySEToNumericVector(output);
}

/**
 * @brief Calculate fixed-effect covariance from a fixed-effect Hessian.
 *
 * @param hessian Fixed-effect Hessian matrix.
 * @return Rcpp::NumericMatrix Fixed-effect covariance matrix.
 */
Rcpp::NumericMatrix calculate_fixed_effect_covariance(
    Rcpp::NumericMatrix hessian) {
  if (hessian.nrow() != hessian.ncol()) {
    Rcpp::stop("hessian must be square");
  }
  if (hessian.nrow() == 0) {
    return Rcpp::NumericMatrix(0, 0);
  }

  fims::Vector<double> covariance = InvertSquareMatrix(
      fims::Vector<double>(NumericMatrixToRowMajor(hessian)),
      static_cast<size_t>(hessian.nrow()),
      "calculate_fixed_effect_covariance");
  return FimsVectorToNumericMatrix(
      covariance, static_cast<size_t>(hessian.nrow()),
      static_cast<size_t>(hessian.ncol()));
}

/**
 * @brief Calculate a fixed-effect Hessian by central differencing a gradient.
 *
 * @param parameters Fixed-effect parameter vector.
 * @param gradient_function Function returning the fixed-effect gradient.
 * @param relative_step Relative finite-difference step size.
 * @return Rcpp::NumericMatrix Fixed-effect Hessian matrix.
 */
Rcpp::NumericMatrix calculate_fixed_effect_hessian(
    Rcpp::NumericVector parameters, Rcpp::Function gradient_function,
    double relative_step = std::pow(std::numeric_limits<double>::epsilon(),
                                    1.0 / 3.0)) {
  if (parameters.size() == 0) {
    return Rcpp::NumericMatrix(0, 0);
  }
  if (relative_step <= 0.0) {
    Rcpp::stop("relative_step must be positive");
  }

  const int n_parameters = parameters.size();
  Rcpp::NumericMatrix hessian(n_parameters, n_parameters);
  Rcpp::NumericVector forward_parameters = Rcpp::clone(parameters);
  Rcpp::NumericVector backward_parameters = Rcpp::clone(parameters);

  for (int col = 0; col < n_parameters; col++) {
    const double step =
        relative_step * (std::fabs(parameters[col]) + 1.0);
    forward_parameters[col] = parameters[col] + step;
    backward_parameters[col] = parameters[col] - step;

    Rcpp::NumericVector forward_gradient =
        gradient_function(forward_parameters);
    Rcpp::NumericVector backward_gradient =
        gradient_function(backward_parameters);
    if (forward_gradient.size() != n_parameters ||
        backward_gradient.size() != n_parameters) {
      Rcpp::stop(
          "calculate_fixed_effect_hessian: gradient size does not match "
          "parameter size");
    }

    for (int row = 0; row < n_parameters; row++) {
      hessian(row, col) =
          (forward_gradient[row] - backward_gradient[row]) / (2.0 * step);
    }

    forward_parameters[col] = parameters[col];
    backward_parameters[col] = parameters[col];
  }

  for (int row = 0; row < n_parameters; row++) {
    for (int col = row + 1; col < n_parameters; col++) {
      const double symmetric_value =
          0.5 * (hessian(row, col) + hessian(col, row));
      hessian(row, col) = symmetric_value;
      hessian(col, row) = symmetric_value;
    }
  }

  return hessian;
}

/**
 * @brief Calculate the Laplace fixed-Jacobian adjustment for ADREPORT SEs.
 *
 * @param random_hessian Random-effect Hessian matrix.
 * @param random_jacobian ADREPORT-by-random-effect Jacobian matrix.
 * @param random_indices Zero-based random-effect indices in the full parameter
 * vector.
 * @param fixed_indices Zero-based fixed-effect indices in the full parameter
 * vector.
 * @param n_parameters Full parameter vector size.
 * @param reverse_sweep_function R callback returning fixed-effect reverse
 * sweep values for a full-parameter-length weight vector.
 * @return Rcpp::NumericMatrix ADREPORT-by-fixed-effect adjustment matrix.
 */
Rcpp::NumericMatrix calculate_laplace_fixed_jacobian_adjustment(
    Rcpp::NumericMatrix random_hessian, Rcpp::NumericMatrix random_jacobian,
    Rcpp::IntegerVector random_indices, Rcpp::IntegerVector fixed_indices,
    int n_parameters, Rcpp::Function reverse_sweep_function) {
  if (n_parameters < 1) {
    Rcpp::stop("n_parameters must be greater than zero");
  }

  fims::Vector<int> random_indices_zero_based;
  for (int i = 0; i < random_indices.size(); i++) {
    random_indices_zero_based.push_back(
        static_cast<int>(random_indices[i]));
  }
  fims::Vector<int> fixed_indices_zero_based;
  for (int i = 0; i < fixed_indices.size(); i++) {
    fixed_indices_zero_based.push_back(static_cast<int>(fixed_indices[i]));
  }

  RFunctionLaplaceReverseSweepProvider reverse_provider(
      reverse_sweep_function);
  fims_tmb::LaplaceFixedJacobianAdjustmentCalculator calculator;
  fims::Vector<double> adjustment = calculator.Calculate(
      fims::Vector<double>(NumericMatrixToRowMajor(random_hessian)),
      fims::Vector<double>(NumericMatrixToRowMajor(random_jacobian)),
      random_indices_zero_based, fixed_indices_zero_based,
      static_cast<size_t>(n_parameters),
      static_cast<size_t>(random_jacobian.nrow()), reverse_provider);

  return FimsVectorToNumericMatrix(
      adjustment, static_cast<size_t>(random_jacobian.nrow()),
      static_cast<size_t>(fixed_indices.size()));
}

/**
 * @brief Calculate the Laplace fixed-Jacobian adjustment from a native ADGrad
 * ADFun pointer.
 *
 * @param random_hessian Random-effect Hessian matrix.
 * @param random_jacobian ADREPORT-by-random-effect Jacobian matrix.
 * @param random_indices Zero-based random-effect indices in the full parameter
 * vector.
 * @param fixed_indices Zero-based fixed-effect indices in the full parameter
 * vector.
 * @param parameters Full parameter vector used by the ADGrad tape.
 * @param adgrad_ptr External pointer to a native TMBad ADGrad ADFun.
 * @return Rcpp::NumericMatrix ADREPORT-by-fixed-effect adjustment matrix.
 */
Rcpp::NumericMatrix calculate_laplace_fixed_jacobian_adjustment_native(
    Rcpp::NumericMatrix random_hessian, Rcpp::NumericMatrix random_jacobian,
    Rcpp::IntegerVector random_indices, Rcpp::IntegerVector fixed_indices,
    Rcpp::NumericVector parameters, SEXP adgrad_ptr) {
  if (TYPEOF(adgrad_ptr) != EXTPTRSXP) {
    Rcpp::stop("adgrad_ptr must be an external pointer");
  }
  if (R_ExternalPtrAddr(adgrad_ptr) == nullptr) {
    Rcpp::stop("adgrad_ptr is null");
  }

  fims::Vector<int> random_indices_zero_based;
  for (int i = 0; i < random_indices.size(); i++) {
    random_indices_zero_based.push_back(
        static_cast<int>(random_indices[i]));
  }
  fims::Vector<int> fixed_indices_zero_based;
  for (int i = 0; i < fixed_indices.size(); i++) {
    fixed_indices_zero_based.push_back(static_cast<int>(fixed_indices[i]));
  }

  TMBADFunLaplaceReverseSweepProvider reverse_provider(
      adgrad_ptr, parameters);
  fims_tmb::LaplaceFixedJacobianAdjustmentCalculator calculator;
  fims::Vector<double> adjustment = calculator.Calculate(
      fims::Vector<double>(NumericMatrixToRowMajor(random_hessian)),
      fims::Vector<double>(NumericMatrixToRowMajor(random_jacobian)),
      random_indices_zero_based, fixed_indices_zero_based,
      static_cast<size_t>(parameters.size()),
      static_cast<size_t>(random_jacobian.nrow()), reverse_provider);

  return FimsVectorToNumericMatrix(
      adjustment, static_cast<size_t>(random_jacobian.nrow()),
      static_cast<size_t>(fixed_indices.size()));
}

/**
 * @brief Calculate derived quantity SEs from a structured ADREPORT payload.
 *
 * @param payload Structured payload extracted from TMB ADREPORT output.
 * @return Rcpp::NumericVector Standard errors.
 */
Rcpp::NumericVector calculate_adreport_payload_se(Rcpp::List payload) {
  fims_tmb::ADReportPayload adreport_payload =
      ADReportPayloadFromRcppList(payload);
  fims_tmb::ADReportPayloadUncertaintyCalculator calculator;
  fims_report::DerivedQuantityEstimate output =
      calculator.Calculate(adreport_payload);
  return DerivedQuantitySEToNumericVector(output);
}

/**
 * @brief Assemble a structured ADREPORT payload from raw TMB derivative pieces.
 *
 * @param payload Raw ADREPORT extraction pieces.
 * @return Rcpp::List Structured ADREPORT payload.
 */
Rcpp::List assemble_adreport_payload(Rcpp::List payload) {
  fims_tmb::ADReportPayloadExtractionInput input;

  Rcpp::NumericVector estimate = payload["estimate"];
  Rcpp::NumericMatrix jacobian = payload["jacobian"];
  Rcpp::NumericMatrix fixed_covariance = payload["fixed_covariance"];

  input.estimate =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(estimate));
  input.jacobian = fims::Vector<double>(NumericMatrixToRowMajor(jacobian));
  input.fixed_effect_covariance =
      fims::Vector<double>(NumericMatrixToRowMajor(fixed_covariance));
  input.random_indices = RandomIndicesFromRcppList(payload);
  input.n_parameters = static_cast<size_t>(jacobian.ncol());

  if (input.random_indices.size() > 0) {
    Rcpp::NumericMatrix random_hessian = payload["random_hessian"];
    Rcpp::NumericMatrix fixed_jacobian_adjustment =
        payload["fixed_jacobian_adjustment"];

    input.random_effect_hessian =
        fims::Vector<double>(NumericMatrixToRowMajor(random_hessian));
    input.fixed_jacobian_adjustment =
        fims::Vector<double>(NumericMatrixToRowMajor(
            fixed_jacobian_adjustment));
  }

  fims_tmb::StaticADReportDerivativeProvider provider(input);
  fims_tmb::ADReportPayloadExtractor extractor;
  fims_tmb::ADReportPayload output = extractor.Extract(provider);
  return ADReportPayloadToRcppList(output);
}

/**
 * @brief Experimental internal bridge for assembling a structured ADREPORT
 * payload from a native TMBad ADFun pointer.
 *
 * @param adfun_ptr External pointer to a TMBad ADREPORT ADFun.
 * @param parameters Full parameter vector used to evaluate the report.
 * @param fixed_covariance Fixed-effect covariance matrix.
 * @param random_payload Optional list with random_indices, random_hessian, and
 * fixed_jacobian_adjustment.
 * @return Rcpp::List Structured ADREPORT payload.
 */
Rcpp::List assemble_adreport_payload_from_tmb_adfun_native(
    SEXP adfun_ptr, Rcpp::NumericVector parameters,
    Rcpp::NumericMatrix fixed_covariance, Rcpp::List random_payload) {
#ifdef TMBAD_FRAMEWORK
  if (TYPEOF(adfun_ptr) != EXTPTRSXP) {
    Rcpp::stop("adfun_ptr must be an external pointer");
  }

  void* raw_adfun = R_ExternalPtrAddr(adfun_ptr);
  if (raw_adfun == nullptr) {
    Rcpp::stop("adfun_ptr is null");
  }

  Rcpp::NumericVector estimate;
  Rcpp::NumericMatrix jacobian;
  try {
    estimate = Rcpp::as<Rcpp::NumericVector>(
        EvalADFunObject(adfun_ptr, parameters, TMBADFunEvaluationControl(0)));
    jacobian = Rcpp::as<Rcpp::NumericMatrix>(
        EvalADFunObject(adfun_ptr, parameters, TMBADFunEvaluationControl(1)));
  } catch (std::exception& e) {
    Rcpp::stop(
        "Native ADREPORT extraction failed with parameter length %i: %s",
        parameters.size(), e.what());
  }

  fims_tmb::ADReportPayloadExtractionInput input;
  input.estimate =
      fims::Vector<double>(Rcpp::as<std::vector<double>>(estimate));
  input.jacobian =
      fims::Vector<double>(NumericMatrixToRowMajor(jacobian));
  input.fixed_effect_covariance =
      fims::Vector<double>(NumericMatrixToRowMajor(fixed_covariance));
  input.n_parameters = static_cast<size_t>(jacobian.ncol());

  if (random_payload.size() > 0) {
    if (!random_payload.containsElementNamed("random_indices") ||
        !random_payload.containsElementNamed("random_hessian") ||
        !random_payload.containsElementNamed("fixed_jacobian_adjustment")) {
      Rcpp::stop(
          "random_payload must contain random_indices, random_hessian, and "
          "fixed_jacobian_adjustment");
    }

    Rcpp::IntegerVector random_indices =
        random_payload["random_indices"];
    Rcpp::NumericMatrix random_hessian =
        random_payload["random_hessian"];
    Rcpp::NumericMatrix fixed_jacobian_adjustment =
        random_payload["fixed_jacobian_adjustment"];
    fims::Vector<int> random_indices_zero_based;
    for (int i = 0; i < random_indices.size(); i++) {
      random_indices_zero_based.push_back(
          static_cast<int>(random_indices[i]));
    }
    input.random_indices = random_indices_zero_based;
    input.random_effect_hessian =
        fims::Vector<double>(NumericMatrixToRowMajor(random_hessian));
    input.fixed_jacobian_adjustment =
        fims::Vector<double>(
            NumericMatrixToRowMajor(fixed_jacobian_adjustment));
  }

  fims_tmb::StaticADReportDerivativeProvider provider(input);
  fims_tmb::ADReportPayloadExtractor extractor;
  fims_tmb::ADReportPayload output = extractor.Extract(provider);
  return ADReportPayloadToRcppList(output);
#else
  Rcpp::stop(
      "assemble_adreport_payload_from_tmb_adfun_native requires "
      "TMBAD_FRAMEWORK");
#endif
}

/**
 * Function to register functions with the Rcpp module system.
 *
 */
void register_functions(Rcpp::Module &m) {
  Rcpp::function(
      "CreateTMBModel", &CreateTMBModel,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "set_fixed", &set_fixed_parameters,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "get_fixed", &get_fixed_parameters_vector,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "set_random", &set_random_parameters,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "get_random", &get_random_parameters_vector,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_parameter_names", &get_parameter_names,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_random_names", &get_random_names,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "clear", clear,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_log", get_log,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_log_errors", get_log_errors,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_log_warnings", get_log_warnings,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_log_info", get_log_info,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "write_log", write_log,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "set_log_path", set_log_path,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "init_logging", init_logging,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "set_log_throw_on_error", set_log_throw_on_error,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "log_info", log_info,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "log_warning", log_warning,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "log_error", log_error,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "logit", logit_rcpp,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "inv_logit", inv_logit_rcpp,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "calculate_derived_quantity_se", &calculate_derived_quantity_se,
      "Calculate derived quantity standard errors using the FIMS backend "
      "delta-method calculator.");
  Rcpp::function(
      "calculate_derived_quantity_laplace_se",
      &calculate_derived_quantity_laplace_se,
      "Calculate derived quantity standard errors using the FIMS backend "
      "Laplace ADREPORT calculator.");
  Rcpp::function(
      "calculate_fixed_effect_covariance",
      &calculate_fixed_effect_covariance,
      "Calculate fixed-effect covariance from a fixed-effect Hessian.");
  Rcpp::function(
      "calculate_fixed_effect_hessian",
      &calculate_fixed_effect_hessian,
      "Calculate a fixed-effect Hessian by central differencing a gradient.");
  Rcpp::function(
      "calculate_laplace_fixed_jacobian_adjustment",
      &calculate_laplace_fixed_jacobian_adjustment,
      "Calculate the Laplace fixed-Jacobian adjustment for ADREPORT standard "
      "errors.");
  Rcpp::function(
      "calculate_laplace_fixed_jacobian_adjustment_native",
      &calculate_laplace_fixed_jacobian_adjustment_native,
      "Calculate the Laplace fixed-Jacobian adjustment for ADREPORT standard "
      "errors from a native TMBad ADGrad external pointer.");
  Rcpp::function(
      "calculate_adreport_payload_se", &calculate_adreport_payload_se,
      "Calculate derived quantity standard errors from a structured ADREPORT "
      "payload.");
  Rcpp::function(
      "assemble_adreport_payload", &assemble_adreport_payload,
      "Assemble a structured ADREPORT payload from raw TMB derivative pieces.");
  Rcpp::function(
      "assemble_adreport_payload_from_tmb_adfun_native",
      &assemble_adreport_payload_from_tmb_adfun_native,
      "Internal experimental bridge for assembling a structured ADREPORT "
      "payload from a native TMBad ADFun external pointer.");
}
