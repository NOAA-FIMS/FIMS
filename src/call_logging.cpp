#include "../inst/include/common/def.hpp"
#include "../inst/include/interface/call/logging.hpp"
#define R_NO_REMAP
#include <Rinternals.h>

extern "C" SEXP fims_call_logging(SEXP operation, SEXP value) {
  if (TYPEOF(operation) != STRSXP || XLENGTH(operation) != 1 ||
      STRING_ELT(operation, 0) == NA_STRING) {
    Rf_error("operation must be a single non-missing string");
  }
  // Copy exceptions before raising an R error, outside the C++ catch scope.
  char error[4096] = {};
  try {
    const std::string op = CHAR(STRING_ELT(operation, 0));
    auto& log = *fims::FIMSLog::fims_log;
    if (op == "get_log") return Rf_mkString(log.get_log().c_str());
    if (op == "get_log_errors") return Rf_mkString(log.get_errors().c_str());
    if (op == "get_log_warnings") return Rf_mkString(log.get_warnings().c_str());
    if (op == "get_log_info") return Rf_mkString(log.get_info().c_str());
    if (op == "get_log_path") return Rf_mkString(log.get_path().c_str());
    if (op == "write_log" || op == "set_log_throw_on_error") {
      if (TYPEOF(value) != LGLSXP || XLENGTH(value) != 1 ||
          LOGICAL(value)[0] == NA_LOGICAL) {
        throw std::invalid_argument("value must be a single non-missing logical");
      }
      if (op == "write_log") log.write_on_exit = LOGICAL(value)[0];
      else log.throw_on_error = LOGICAL(value)[0];
    } else {
      if (TYPEOF(value) != STRSXP || XLENGTH(value) != 1 ||
          STRING_ELT(value, 0) == NA_STRING) {
        throw std::invalid_argument("value must be a single non-missing string");
      }
      const std::string entry = CHAR(STRING_ELT(value, 0));
      if (op == "set_log_path") log.set_path(entry);
      else if (op == "log_info") log.info_message(entry, -1, "R_env", "R_script_entry");
      else if (op == "log_warning") log.warning_message(entry, -1, "R_env", "R_script_entry");
      else if (op == "log_error") log.error_message(entry, -1, "R_env", "R_script_entry");
      else throw std::invalid_argument("Unknown logging operation");
    }
  } catch (const std::exception& e) {
    snprintf(error, sizeof(error), "%s", e.what());
  }
  if (error[0]) Rf_error("%s", error);
  return R_NilValue;
}
