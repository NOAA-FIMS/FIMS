
/**
 * \file rcpp_functions.cpp
 * \brief Implementation of Rcpp function interfaces for the FIMS framework.
 */
#include "../inst/include/distributions/functors/lognormal_lpdf.hpp"
#include "../inst/include/distributions/functors/normal_lpdf.hpp"
#include "../inst/include/interface/rcpp/rcpp_include.hpp"
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_math.hpp"
#include "../inst/include/population_dynamics/fleet/fleet.hpp"
#include "../inst/include/population_dynamics/growth/functors/vonb_schnute.hpp"
#include "../inst/include/population_dynamics/population/population.hpp"
#include "../inst/include/population_dynamics/recruitment/functors/sr_beverton_holt.hpp"
#include "../inst/include/population_dynamics/selectivity/functors/double_logistic.hpp"
#include "../inst/include/population_dynamics/selectivity/functors/logistic.hpp"

/**
 * @brief Expose shared defaults without constructing or registering modules.
 */
Rcpp::List get_parameter_defaults() {
  using namespace fims_defaults;
  return Rcpp::List::create(
      Rcpp::Named("Logistic") = Rcpp::NumericVector::create(
          Rcpp::Named("inflection_point") = logistic::kDefaultInflectionPoint,
          Rcpp::Named("slope") = logistic::kDefaultSlope),
      Rcpp::Named("DoubleLogistic") = Rcpp::NumericVector::create(
          Rcpp::Named("inflection_point_asc") =
              double_logistic::kDefaultInflectionPointAsc,
          Rcpp::Named("slope_asc") = double_logistic::kDefaultSlopeAsc,
          Rcpp::Named("inflection_point_desc") =
              double_logistic::kDefaultInflectionPointDesc,
          Rcpp::Named("slope_desc") = double_logistic::kDefaultSlopeDesc),
      Rcpp::Named("BevertonHolt") = Rcpp::NumericVector::create(
          Rcpp::Named("log_rzero") = recruitment::kDefaultLogRzero,
          Rcpp::Named("logit_steep") = recruitment::kDefaultLogitSteep,
          Rcpp::Named("log_devs") = recruitment::kDefaultLogDevs),
      Rcpp::Named("VonBertalanffySchnute") = Rcpp::NumericVector::create(
          Rcpp::Named("mean_length_young") = growth::kDefaultMeanLengthYoung,
          Rcpp::Named("mean_length_old") = growth::kDefaultMeanLengthOld,
          Rcpp::Named("growth_coefficient") = growth::kDefaultGrowthCoefficient,
          Rcpp::Named("length_weight_a") = growth::kDefaultLengthWeightA,
          Rcpp::Named("length_weight_b") = growth::kDefaultLengthWeightB,
          Rcpp::Named("length_sd_young") = growth::kDefaultLengthSdYoung,
          Rcpp::Named("length_sd_old") = growth::kDefaultLengthSdOld),
      Rcpp::Named("Population") = Rcpp::NumericVector::create(
          Rcpp::Named("log_M") = population::kDefaultLogM,
          Rcpp::Named("proportion_female") =
              population::kDefaultProportionFemale),
      Rcpp::Named("Fleet") = Rcpp::NumericVector::create(
          Rcpp::Named("log_q") = fleet::kDefaultLogQ,
          Rcpp::Named("log_Fmort") = fleet::kDefaultLogFmort,
          Rcpp::Named("survey_log_Fmort") = fleet::kDefaultSurveyLogFmort),
      Rcpp::Named("Dnorm") = Rcpp::NumericVector::create(
          Rcpp::Named("log_sd") = distribution::kDefaultNormalLogSd),
      Rcpp::Named("Dlnorm") = Rcpp::NumericVector::create(
          Rcpp::Named("sd") = distribution::kDefaultLognormalSd,
          Rcpp::Named("log_sd") = distribution::kDefaultLognormalLogSd));
}

/**
 * Function to register functions with the Rcpp module system.
 *
 */
void register_functions(Rcpp::Module &m) {
  Rcpp::function("get_parameter_defaults", &get_parameter_defaults,
                 "Get shared, data-independent parameter starting values.");
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
  Rcpp::function("test_clear_with_leak_check", test_clear_with_leak_check,
                 "Test-only variant of clear(). Not part of the public API.");
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