/**
 * \file call_log.cpp
 * \brief Native .Call logging interface for FIMS.
 */

#include "../inst/include/interface/call/log.hpp"

#include <exception>
#include <string>

#include "../inst/include/common/def.hpp"

#include <Rinternals.h>

namespace {

std::shared_ptr<fims::FIMSLog> logger() { return fims::FIMSLog::fims_log; }

std::string require_scalar_string(SEXP value, const char *argument_name) {
  if (TYPEOF(value) != STRSXP || XLENGTH(value) != 1 ||
      STRING_ELT(value, 0) == NA_STRING) {
    Rf_error("`%s` must be one non-missing string.", argument_name);
  }
  return CHAR(STRING_ELT(value, 0));
}

bool require_scalar_logical(SEXP value, const char *argument_name) {
  if (TYPEOF(value) != LGLSXP || XLENGTH(value) != 1 ||
      LOGICAL(value)[0] == NA_LOGICAL) {
    Rf_error("`%s` must be TRUE or FALSE.", argument_name);
  }
  return LOGICAL(value)[0] == TRUE;
}

SEXP scalar_string(const std::string &value) {
  return Rf_mkString(value.c_str());
}

} // namespace

extern "C" SEXP fims_call_log_get() {
  return scalar_string(logger()->get_log());
}

extern "C" SEXP fims_call_log_get_errors() {
  return scalar_string(logger()->get_errors());
}

extern "C" SEXP fims_call_log_get_warnings() {
  return scalar_string(logger()->get_warnings());
}

extern "C" SEXP fims_call_log_get_info() {
  return scalar_string(logger()->get_info());
}

extern "C" SEXP fims_call_log_info(SEXP message_sexp) {
  const std::string message = require_scalar_string(message_sexp, "message");
  logger()->info_message(message, -1, "R_env", "R_script_entry");
  return R_NilValue;
}

extern "C" SEXP fims_call_log_warning(SEXP message_sexp) {
  const std::string message = require_scalar_string(message_sexp, "message");
  logger()->warning_message(message, -1, "R_env", "R_script_entry");
  return R_NilValue;
}

extern "C" SEXP fims_call_log_error(SEXP message_sexp) {
  const std::string message = require_scalar_string(message_sexp, "message");
  try {
    logger()->error_message(message, -1, "R_env", "R_script_entry");
  } catch (const std::exception &exception) {
    Rf_error("%s", exception.what());
  }
  return R_NilValue;
}

extern "C" SEXP fims_call_log_write_on_exit(SEXP write_sexp) {
  logger()->write_on_exit = require_scalar_logical(write_sexp, "write");
  return R_NilValue;
}

extern "C" SEXP fims_call_log_set_path(SEXP path_sexp) {
  logger()->set_path(require_scalar_string(path_sexp, "path"));
  return R_NilValue;
}

extern "C" SEXP fims_call_log_set_throw_on_error(SEXP throw_sexp) {
  logger()->throw_on_error =
      require_scalar_logical(throw_sexp, "throw_on_error");
  return R_NilValue;
}
