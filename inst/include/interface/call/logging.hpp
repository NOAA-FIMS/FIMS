#ifndef FIMS_INTERFACE_CALL_LOGGING_HPP
#define FIMS_INTERFACE_CALL_LOGGING_HPP
typedef struct SEXPREC *SEXP;
extern "C" SEXP fims_call_logging(SEXP operation, SEXP value);
#endif
