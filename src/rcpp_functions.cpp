
/**
 * \file rcpp_functions.cpp
 * \brief Implementation of Rcpp function interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_math.hpp"
#include <Rcpp.h>

/**
 * Function to register functions with the Rcpp module system.
 *
 */
void register_functions(Rcpp::Module &m) {
#ifdef TMB_MODEL
  Rcpp::function(
      "CreateTMBModel", &CreateTMBModel,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
#endif
#ifdef QUADRA_MODEL
  Rcpp::function(
      "CreateQuadraModel", &CreateQuadraModel,
      "Construct the configured FIMS model using the Quadra AD backend.");
  Rcpp::function(
      "EvaluateQuadraModel", &EvaluateQuadraModel,
      "Evaluate the joint FIMS objective and gradient using Quadra.");
  Rcpp::function(
      "BenchmarkQuadraModel", &BenchmarkQuadraModel,
      "Time repeated replay of the most recently evaluated Quadra model.");
  Rcpp::function(
      "BenchmarkQuadraRestrictedHessian", &BenchmarkQuadraRestrictedHessian,
      "Compare full and partition-restricted random Hessian propagation.");
  Rcpp::function(
      "fit_fims_quadra", &fit_fims_quadra,
      "Fit the Laplace-profiled FIMS objective with Quadra.");
  Rcpp::function(
      "EvaluateQuadraLaplaceModel", &EvaluateQuadraLaplaceModel,
      "Evaluate the model-aware Quadra Laplace objective and backend.");
  Rcpp::function(
      "EvaluateQuadraDenseHessian", &EvaluateQuadraDenseHessian,
      "Diagnose numerical sparsity in the random-effect Hessian.");
  Rcpp::function(
      "quadra_model_diagnostics", &quadra_model_diagnostics,
      "Return Quadra functional diagnostics for a configured FIMS model.");
  Rcpp::function(
      "quadra_model_diagnostics_md", &quadra_model_diagnostics_md,
      "Render Quadra model diagnostics as a Markdown string.");
  Rcpp::function(
      "fit_fims_quadra_joint", &fit_fims_quadra_joint,
      "Fit the unprofiled FIMS joint objective with L-BFGS.");
#endif
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
  Rcpp::function("test_clear_with_leak_check", test_clear_with_leak_check, "Test-only variant of clear(). Not part of the public API.");
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
}
