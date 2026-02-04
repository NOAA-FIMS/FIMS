/**
 * @file rcpp_interface.hpp
 * @brief The Rcpp interface to declare things.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP
#include "../../common/model.hpp"
#include "../../utilities/fims_json.hpp"
#include "rcpp_objects/rcpp_data.hpp"
#include "rcpp_objects/rcpp_distribution.hpp"
#include "rcpp_objects/rcpp_fleet.hpp"
#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_interface_base.hpp"
#include "rcpp_objects/rcpp_maturity.hpp"
#include "rcpp_objects/rcpp_models.hpp"
#include "rcpp_objects/rcpp_natural_mortality.hpp"
#include "rcpp_objects/rcpp_population.hpp"
#include "rcpp_objects/rcpp_recruitment.hpp"
#include "rcpp_objects/rcpp_selectivity.hpp"

/**
 * Initializes the logging system, setting all signal handling.
 */
void init_logging() {
  std::signal(SIGSEGV, &fims::WriteAtExit);
  std::signal(SIGINT, &fims::WriteAtExit);
  std::signal(SIGABRT, &fims::WriteAtExit);
  std::signal(SIGFPE, &fims::WriteAtExit);
  std::signal(SIGILL, &fims::WriteAtExit);
  std::signal(SIGTERM, &fims::WriteAtExit);
}

/**
 * @brief Creates the TMB model object and adds interface objects to it.
 *
 * @details
 * This function is called within `initialize_fims()` from R and is not
 * typically called by the user directly.
 */
bool CreateTMBModel() {
  init_logging();

  // clear first
  //  base model
#ifdef TMBAD_FRAMEWORK
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  info0->Clear();

  std::shared_ptr<fims_info::Information<TMBAD_FIMS_TYPE>> info =
      fims_info::Information<TMBAD_FIMS_TYPE>::GetInstance();
  info->Clear();

#else
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  info0->Clear()

      // first-order derivative
      std::shared_ptr<fims_info::Information<TMB_FIMS_FIRST_ORDER>>
          info1 = fims_info::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();
  info1->Clear();

  // second-order derivative
  std::shared_ptr<fims_info::Information<TMB_FIMS_SECOND_ORDER>> info2 =
      fims_info::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();
  info2->Clear();

  // third-order derivative
  std::shared_ptr<fims_info::Information<TMB_FIMS_THIRD_ORDER>> info3 =
      fims_info::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();
  info3->Clear();
#endif

  FIMS_INFO_LOG(
      "Adding FIMS objects to TMB, " +
      fims::to_string(FIMSRcppInterfaceBase::fims_interface_objects.size()) +
      " objects");
  for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
       i++) {
    FIMSRcppInterfaceBase::fims_interface_objects[i]->add_to_fims_tmb();
  }

  // base model
#ifdef TMBAD_FRAMEWORK

  info0->CreateModel();
  info0->CheckModel();

  info->CreateModel();

#else

  info0->CreateModel();
  info0->CheckModel();

  info1->CreateModel();

  // second-order derivative

  info2->CreateModel();

  // third-order derivative

  info3->CreateModel();
#endif

  // instantiate the model? TODO: Ask Matthew what this does
  std::shared_ptr<fims_model::Model<TMB_FIMS_REAL_TYPE>> m0 =
      fims_model::Model<TMB_FIMS_REAL_TYPE>::GetInstance();

  return true;
}

/**
 * @brief Sets the fixed parameters vector object.
 * Updates the internal parameter values for the model base
 * of type TMB_FIMS_REAL_TYPE. Typically called before
 * finalize or get_output to ensure the correct values are used.
 *
 * Usage example:
 * \code{.R}
 * set_fixed_parameters(c(1, 2, 3))
 * catch_at_age$get_output()
 * \endcode
 *
 * @param par A vector of parameter values.
 */
void set_fixed_parameters(Rcpp::NumericVector par) {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  for (size_t i = 0; i < info0->fixed_effects_parameters.size(); i++) {
    *info0->fixed_effects_parameters[i] = par[i];
  }
}

/**
 * @brief Gets the fixed parameters vector object.
 *
 * @return Rcpp::NumericVector
 */
Rcpp::NumericVector get_fixed_parameters_vector() {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < info0->fixed_effects_parameters.size(); i++) {
    p.push_back(*info0->fixed_effects_parameters[i]);
  }

  return p;
}

/**
 * @brief Sets the random parameters vector object.
 * Updates the internal random effects parameter values for
 * the model base of TMB_FIMS_REAL_TYPE. Typically called before
 * finalize or get_output to ensure the correct values are used.
 *
 * Usage example:
 * \code{.R}
 * set_random_parameters(c(1, 2, 3))
 * catch_at_age$get_output()
 * \endcode
 *
 * @param par A vector of parameter values.
 */
void set_random_parameters(Rcpp::NumericVector par) {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  for (size_t i = 0; i < info0->random_effects_parameters.size(); i++) {
    *info0->random_effects_parameters[i] = par[i];
  }
}

/**
 * @brief Gets the random parameters vector object.
 *
 * @return Rcpp::NumericVector
 */
Rcpp::NumericVector get_random_parameters_vector() {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < d0->random_effects_parameters.size(); i++) {
    p.push_back(*d0->random_effects_parameters[i]);
  }

  return p;
}

/**
 * @brief Gets the parameter names object.
 *
 * @param pars
 * @return Rcpp::List
 */
Rcpp::List get_parameter_names(Rcpp::List pars) {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  pars.attr("names") = d0->parameter_names;

  return pars;
}

/**
 * @brief Gets the random effects names object.
 *
 * @param pars
 * @return Rcpp::List
 */
Rcpp::List get_random_names(Rcpp::List pars) {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  pars.attr("names") = d0->random_effects_names;

  return pars;
}

/**
* @brief Gets a vector of parameter minimum values.
* @return Rcpp::NumericVector
*/
Rcpp::NumericVector get_parameter_min_vector() {
  //loop over all parameters and get their minimum values
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();  
  Rcpp::NumericVector min_vector;

  for (size_t i = 0; i < d0->fixed_effects_parameters_min.size(); i++) {
    min_vector.push_back(d0->fixed_effects_parameters_min[i]);
  }

  return min_vector;
}

/**
* @brief Gets a vector of parameter maximum values.
* @return Rcpp::NumericVector
*/
Rcpp::NumericVector get_parameter_max_vector() {
  //loop over all parameters and get their maximum values
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();  
  Rcpp::NumericVector max_vector;

  for (size_t i = 0; i < d0->fixed_effects_parameters_max.size(); i++) {
    max_vector.push_back(d0->fixed_effects_parameters_max[i]);
  }

  return max_vector;
}

/**
* @brief Gets a vector of random effects minimum values.
* @return Rcpp::NumericVector
*/
Rcpp::NumericVector get_random_effects_min_vector() {
  //loop over all random effects and get their minimum values
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();  
  Rcpp::NumericVector min_vector;

  for (size_t i = 0; i < d0->random_effects_parameters_min.size(); i++) {
    min_vector.push_back(d0->random_effects_parameters_min[i]);
  }

  return min_vector;
}

/**
* @brief Gets a vector of random effects maximum values.
* @return Rcpp::NumericVector
*/
Rcpp::NumericVector get_random_effects_max_vector() {
  //loop over all random effects and get their maximum values
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();  
  Rcpp::NumericVector max_vector;

  for (size_t i = 0; i < d0->random_effects_parameters_max.size(); i++) {
    max_vector.push_back(d0->random_effects_parameters_max[i]);
  }

  return max_vector;
}

/**
 * @brief Clears the internal objects.
 *
 * @tparam Type
 */
template <typename Type>
void clear_internal() {
  std::shared_ptr<fims_info::Information<Type>> d0 =
      fims_info::Information<Type>::GetInstance();
  d0->Clear();
}

/**
 * @brief Clears the vector of independent variables.
 */
void clear() {
  FIMS_INFO_LOG("Clearing FIMS objects from interface stack");
  // rcpp_interface_base.hpp
  FIMSRcppInterfaceBase::fims_interface_objects.clear();

  // Parameter and ParameterVector
  Parameter::id_g = 1;
  ParameterVector::id_g = 1;
  // rcpp_data.hpp
  DataInterfaceBase::id_g = 1;
  DataInterfaceBase::live_objects.clear();

  AgeCompDataInterface::id_g = 1;
  AgeCompDataInterface::live_objects.clear();

  LengthCompDataInterface::id_g = 1;
  LengthCompDataInterface::live_objects.clear();

  LandingsDataInterface::id_g = 1;
  LandingsDataInterface::live_objects.clear();

  IndexDataInterface::id_g = 1;
  IndexDataInterface::live_objects.clear();

  // rcpp_fleets.hpp
  FleetInterfaceBase::id_g = 1;
  FleetInterfaceBase::live_objects.clear();

  FleetInterface::id_g = 1;
  FleetInterface::live_objects.clear();

  // rcpp_growth.hpp
  GrowthInterfaceBase::id_g = 1;
  GrowthInterfaceBase::live_objects.clear();

  EWAAGrowthInterface::id_g = 1;
  EWAAGrowthInterface::live_objects.clear();

  // rcpp_maturity.hpp
  MaturityInterfaceBase::id_g = 1;
  MaturityInterfaceBase::live_objects.clear();

  LogisticMaturityInterface::id_g = 1;
  LogisticMaturityInterface::live_objects.clear();

  // rcpp_population.hpp
  PopulationInterfaceBase::id_g = 1;
  PopulationInterfaceBase::live_objects.clear();

  PopulationInterface::id_g = 1;
  PopulationInterface::live_objects.clear();

  // rcpp_recruitment.hpp
  RecruitmentInterfaceBase::id_g = 1;
  RecruitmentInterfaceBase::live_objects.clear();

  BevertonHoltRecruitmentInterface::id_g = 1;
  BevertonHoltRecruitmentInterface::live_objects.clear();

  // rcpp_selectivity.hpp
  SelectivityInterfaceBase::id_g = 1;
  SelectivityInterfaceBase::live_objects.clear();

  LogisticSelectivityInterface::id_g = 1;
  LogisticSelectivityInterface::live_objects.clear();

  DoubleLogisticSelectivityInterface::id_g = 1;
  DoubleLogisticSelectivityInterface::live_objects.clear();

  // rcpp_distribution.hpp
  DistributionsInterfaceBase::id_g = 1;
  DistributionsInterfaceBase::live_objects.clear();

  DnormDistributionsInterface::id_g = 1;
  DnormDistributionsInterface::live_objects.clear();

  DlnormDistributionsInterface::id_g = 1;
  DlnormDistributionsInterface::live_objects.clear();

  DmultinomDistributionsInterface::id_g = 1;
  DmultinomDistributionsInterface::live_objects.clear();

  FisheryModelInterfaceBase::id_g = 1;
  FisheryModelInterfaceBase::live_objects.clear();

#ifdef TMBAD_FRAMEWORK
  clear_internal<TMB_FIMS_REAL_TYPE>();
  clear_internal<TMBAD_FIMS_TYPE>();
#else
  clear_internal<TMB_FIMS_REAL_TYPE>();
  clear_internal<TMB_FIMS_FIRST_ORDER>();
  clear_internal<TMB_FIMS_SECOND_ORDER>();
  clear_internal<TMB_FIMS_THIRD_ORDER>();
#endif

  fims::FIMSLog::fims_log->clear();
}

/**
 * @brief Gets the log entries as a string in JSON format.
 */
std::string get_log() { return fims::FIMSLog::fims_log->get_log(); }

/**
 * @brief Gets the error entries from the log as a string in JSON format.
 */
std::string get_log_errors() { return fims::FIMSLog::fims_log->get_errors(); }

/**
 * @brief Gets the warning entries from the log as a string in JSON format.
 */
std::string get_log_warnings() {
  return fims::FIMSLog::fims_log->get_warnings();
}

/**
 * @brief Gets the info entries from the log as a string in JSON format.
 */
std::string get_log_info() { return fims::FIMSLog::fims_log->get_info(); }

/**
 * @brief Gets log entries by module as a string in JSON format.
 */
std::string get_log_module(const std::string &module) {
  return fims::FIMSLog::fims_log->get_module(module);
}

/**
 * @brief If true, writes the log on exit.
 */
void write_log(bool write) {
  FIMS_INFO_LOG("Setting FIMS write log: " + fims::to_string(write));
  fims::FIMSLog::fims_log->write_on_exit = write;
}

/**
 * @brief Sets the path for the log file to be written to.
 */
void set_log_path(const std::string &path) {
  FIMS_INFO_LOG("Setting FIMS log path: " + path);
  fims::FIMSLog::fims_log->set_path(path);
}

/**
 * @brief If true, throws a runtime exception when an error is logged.
 */
void set_log_throw_on_error(bool throw_on_error) {
  fims::FIMSLog::fims_log->throw_on_error = throw_on_error;
}

/**
 * @brief Adds an info entry to the log from the R environment.
 */
void log_info(std::string log_entry) {
  fims::FIMSLog::fims_log->info_message(log_entry, -1, "R_env",
                                        "R_script_entry");
}

/**
 * @brief Adds a warning entry to the log from the R environment.
 */
void log_warning(std::string log_entry) {
  fims::FIMSLog::fims_log->warning_message(log_entry, -1, "R_env",
                                           "R_script_entry");
}

/**
 * @brief Escapes quotations.
 *
 * @param input A string.
 * @return std::string
 */
std::string escapeQuotes(const std::string &input) {
  std::string result = input;
  std::string search = "\"";
  std::string replace = "\\\"";

  // Find each occurrence of `"` and replace it with `\"`
  size_t pos = result.find(search);
  while (pos != std::string::npos) {
    result.replace(pos, search.size(), replace);
    pos = result.find(search,
                      pos + replace.size());  // Move past the replaced position
  }
  return result;
}

/**
 * @brief Adds a error entry to the log from the R environment.
 */
void log_error(std::string log_entry) {
  std::stringstream ss;
  ss << "capture.output(traceback(4))";
  SEXP expression, result;
  ParseStatus status;

  PROTECT(expression = R_ParseVector(Rf_mkString(ss.str().c_str()), 1, &status,
                                     R_NilValue));
  if (status != PARSE_OK) {
    Rcpp::Rcout << "Error parsing expression" << std::endl;
    UNPROTECT(1);
  }
  Rcpp::Rcout << "before call.";
  PROTECT(result = Rf_eval(VECTOR_ELT(expression, 0), R_GlobalEnv));
  Rcpp::Rcout << "after call.";
  UNPROTECT(2);
  std::stringstream ss_ret;
  ss_ret << "traceback: ";
  for (int j = 0; j < LENGTH(result); j++) {
    std::string str(CHAR(STRING_ELT(result, j)));
    ss_ret << escapeQuotes(str) << "\\n";
  }

  std::string ret =
      ss_ret.str();  //"find error";//Rcpp::as<std::string>(result);

  fims::FIMSLog::fims_log->error_message(log_entry, -1, "R_env", ret.c_str());
}
#endif  // FIMS_INTERFACE_RCPP_INTERFACE_HPP
