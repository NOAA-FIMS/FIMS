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
#include "../../common/model_object.hpp"
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
 * @brief Initialize and construct the FIMS model using TMB.
 *
 * @details
 * This function sets up the core C++ objects required for building the
 * objective function with TMB before optimizing a FIMS model. The main steps
 * of the function are as follows:
 * - The logging system is initialized and any existing model structures are
 *   cleared, ensuring a clean slate for a new model.
 * - It resets and prepares the main model information objects
 *   (fims_info::Information singletons), ensuring all internal data and
 *   settings are cleared and ready for a new model run. This step is essential
 *   for both initializing the model structure and avoiding conflicts from
 *   previous runs.
 * - It iterates over the list of module interface pointers passed in from R 
 *   and call's each interfaces' `add_to_fims_tmb()` function. 
 * - After all of the objects are registered, it calls
 *   fims_info::Information::CreateModel() and
 *   fims_info::Information::CheckModel() on the base fims_info::Information
 *   object.
 * - It instantiates the singleton fims_model::Model object which represents
 *   the constructed TMB model.
 *
 * Typically the average user does not interact with this function because it
 * is called within <a href =
 * "https://noaa-fims.github.io/FIMS/reference/initialize_fims.html">`initialize_fims`</a>.
 *
 * @see init_logging()
 * @see fims_info::Information::Clear()
 * @see fims_info::Information::CreateModel()
 * @see fims_info::Information::CheckModel()
 * @see fims_info::Information::GetInstance()
 * @see <a href =
 * "https://noaa-fims.github.io/FIMS/reference/initialize_fims.html"
 * target="_blank">`initialize_fims()`</a>
 *
 * @param xptr_list List of Rcpp::XPtrs that point to the modules being used
 * in a particular model run, each an Rcpp::XPtr<SharedBase> as produced by the 
 * `*_to_fims_xptr_()` functions. The R wrapper `CreateTMBModel()` passes the 
 * package registry here, so users do not assemble this list by hand.
 * @return A handle identifying this build of the model. It is a plain number
 * that increases with every successful call, so two handles are equal only if
 * they came from the same build. Failure is reported by throwing, not by the
 * return value, so a returned handle always means the model was created.
 */
uint32_t CreateTMBModel(Rcpp::List xptr_list) {
  // Identifies this build of the model. Incremented on every successful call
  // and never rewound, so a handle taken from one build never matches a later
  // one -- which is what lets R tell that a TMB objective function belongs to
  // a model that has since been rebuilt. 
  static uint32_t model_handle = 0;

  init_logging();

  // clear first
  //  base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  info0->Clear();

  std::shared_ptr<fims_info::Information<TMBAD_FIMS_TYPE>> info =
      fims_info::Information<TMBAD_FIMS_TYPE>::GetInstance();
  info->Clear();

  // Every entry is an XPtr<SharedBase>, so add_to_fims_tmb() reaches the right
  // derived implementation without this loop knowing which module family it is
  // holding. The list comes from the R-side registry, which collects each
  // module's base pointer as it is created.
  for (int i = 0; i < xptr_list.size(); i++) {
    SEXP element = xptr_list[i];
    if (Rf_isNull(element) || TYPEOF(element) != EXTPTRSXP) {
      Rcpp::stop("Model component " + std::to_string(i + 1) +
                 " is not a module pointer.");
    }
    Rcpp::XPtr<SharedBase> xp(element);
    if (!xp || !(*xp)) {
      Rcpp::stop("Model component " + std::to_string(i + 1) +
                 " is an empty pointer. This usually means clear() was called "
                 "while the module was still held in R.");
    }
    (*xp)->add_to_fims_tmb();
  }

  // base model
  info0->CreateModel();
  info0->CheckModel();

  info->CreateModel();

  // instantiate the model? TODO: Ask Matthew what this does
  std::shared_ptr<fims_model::Model<TMB_FIMS_REAL_TYPE>> m0 =
      fims_model::Model<TMB_FIMS_REAL_TYPE>::GetInstance();

  return ++model_handle;
}

/* Dictionary block for shared documentation.
  [details_set_x_parameters]
  Updates the internal parameter values for the model base of type
  TMB_FIMS_REAL_TYPE. It is typically called before finalize() or
  @ref CatchAtAgeInterface::to_json "`get_output()`" to ensure the correct
  values are used because TMB doesn't always keep the updated parameters in
  the "double" version of the tape. So we need to update those first.
  \n\n
  Usage example in R:
  \code{.R}
  set_fixed_parameters(c(1, 2, 3))
  set_random_parameters(c(1, 2, 3))
  catch_at_age$get_output()
  \endcode
  [details_set_x_parameters]
*/
/* Dictionary block for shared documentation.
  [param_par]
  @param par A vector of parameter values.
  [param_par]
 */

/**
 * @brief Update fixed parameters in the tape, so the output is correct.
 * @details @snippet{doc} this details_set_x_parameters
 * @snippet{doc} this param_par
 * @see set_random_parameters()
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
 * @brief Update random effect parameters in the tape, so the output is correct.
 * @details @snippet{doc} this details_set_x_parameters
 * @snippet{doc} this param_par
 * @see set_fixed_parameters()
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
 * @brief Reset the model so the next model starts from a cleared memory state.
 *
 * @details Rewind ID counters to one and reset the TMB information singletons
 * so the next model is not built on top of the previous one. This function is
 * called by the R facing clean() wrapper, which is also responsible for clearing
 * the interface modules. 
 *  
 * @param get_error_msg If true, retains a lingering pointer to trigger
 * dangling pointer diagnostics. Should only be set to true via
 * test_clear_with_leak_check().
 */
void clear_impl(bool get_error_msg) {

  // Check that the R wrapper released every interface module, which it does
  // before calling here, so this count should be zero. A count above zero
  // means something bypassed the wrapper: clear_() called directly, or a
  // module that never reached the registry. That module keeps its ID, the
  // reset below hands the same ID out again, and two modules end up sharing
  // it -- which the model reads as one module, silently.
  if (FIMSRcppInterfaceBase::live_module_count > 0) {
    std::ostringstream msg;
    msg << "\nclear() was called while "
        << FIMSRcppInterfaceBase::live_module_count
        << " module(s) are still in use.\n"
        << "Those modules hold IDs that will be issued again to modules "
           "created after this\ncall, which would silently produce a wrong "
           "model. Release them before calling\nclear().\n";
    Rcpp::warning(msg.str());
  }

  // Each family adds its id_g to this list where the counter is defined, in
  // the matching src/rcpp_*.cpp, so a new family cannot be left out here.
  for (uint32_t *counter : FIMSRcppInterfaceBase::id_counters()) {
    *counter = 1;
  }

  clear_internal<TMB_FIMS_REAL_TYPE>();
  clear_internal<TMBAD_FIMS_TYPE>();

  fims::FIMSLog::fims_log->clear();

  std::unique_ptr<fims_popdy::LogisticSelectivity<double>> test_obj;
  if (get_error_msg) {
    test_obj = std::make_unique<fims_popdy::LogisticSelectivity<double>>();
  }

  // --- AUTOMATED DANGLING POINTER DIAGNOSTIC PRINT ---
  if (fims_model_object::FIMSMemoryTracker::total_active_objects > 0) {
    std::ostringstream msg;
    msg << "\n⚠️  WARNING: FIMS Dangling Pointer or Module Detected after "
           "clear()!\n";
    msg << "--------------------------------------------------\n";
    msg << "A total of "
        << fims_model_object::FIMSMemoryTracker::total_active_objects
        << " pointer(s) NOT cleared\n";
    msg << "--------------------------------------------------\n";
    msg << "Ensure all pointers are being reset.\n\n";

    Rcpp::warning(msg.str());
  }
}

/**
 * @brief Calls the default `clear_impl` function.
 *
 * @details Registered by Rcpp as `clear_()`. The R wrapper `clear()` releases
 * the interface modules and empties the R-side registry before calling this.
 */
void clear() { clear_impl(false); }

/**
 * @brief Test-only variant of `clear_impl()` that retains a lingering pointer
 * to validate dangling pointer diagnostics.
 * @note Not exposed to users. Use only in tests.
 */
void test_clear_with_leak_check() { clear_impl(true); }

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
 * @brief If true, writes the log on exit.
 */
void write_log(bool write) { fims::FIMSLog::fims_log->write_on_exit = write; }

/**
 * @brief Sets the path for the log file to be written to.
 */
void set_log_path(const std::string &path) {
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
