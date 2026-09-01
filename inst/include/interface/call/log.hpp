/**
 * \file log.hpp
 * \brief Native .Call logging interface for FIMS.
 */
#ifndef FIMS_INTERFACE_CALL_LOG_HPP
#define FIMS_INTERFACE_CALL_LOG_HPP

typedef struct SEXPREC *SEXP;

extern "C" SEXP fims_call_log_get();
extern "C" SEXP fims_call_log_get_errors();
extern "C" SEXP fims_call_log_get_warnings();
extern "C" SEXP fims_call_log_get_info();
extern "C" SEXP fims_call_log_info(SEXP message_sexp);
extern "C" SEXP fims_call_log_warning(SEXP message_sexp);
extern "C" SEXP fims_call_log_error(SEXP message_sexp);
extern "C" SEXP fims_call_log_write_on_exit(SEXP write_sexp);
extern "C" SEXP fims_call_log_set_path(SEXP path_sexp);
extern "C" SEXP fims_call_log_set_throw_on_error(SEXP throw_sexp);

#endif
