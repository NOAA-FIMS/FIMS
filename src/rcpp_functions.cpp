
/**
 * \file rcpp_functions.cpp
 * \brief Implementation of Rcpp function interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_math.hpp"
#include "../inst/include/interface/TMB/tmb_derived_quantity_uncertainty.hpp"
#include <Rcpp.h>

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
      "calculate_adreport_payload_se", &calculate_adreport_payload_se,
      "Calculate derived quantity standard errors from a structured ADREPORT "
      "payload.");
  Rcpp::function(
      "assemble_adreport_payload", &assemble_adreport_payload,
      "Assemble a structured ADREPORT payload from raw TMB derivative pieces.");
}
