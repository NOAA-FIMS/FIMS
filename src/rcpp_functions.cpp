
/**
 * \file rcpp_functions.cpp
 * \brief Implementation of Rcpp function interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_math.hpp"
#include "../inst/include/interface/TMB/tmb_derived_quantity_uncertainty.hpp"
#include <Rcpp.h>

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

  Rcpp::NumericVector se(output.se.size());
  for (size_t i = 0; i < output.se.size(); i++) {
    se[i] = output.se[i];
  }
  return se;
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

  Rcpp::NumericVector se(output.se.size());
  for (size_t i = 0; i < output.se.size(); i++) {
    se[i] = output.se[i];
  }
  return se;
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
}
