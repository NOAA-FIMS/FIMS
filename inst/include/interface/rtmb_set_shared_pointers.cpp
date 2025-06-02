// Pass shared pointers RTMB => Other Package
#include <Rinternals.h>
#include <R_ext/Error.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Visibility.h>


extern "C" SEXP _rtmb_set_shared_pointers() {
  typedef SEXP(*funptr)(SEXP);
  funptr fun = (funptr) R_FindSymbol("getSetGlobalPtr", "RTMB", NULL);
  if (fun == NULL) {
      Rf_error("Failed to find symbol 'getSetGlobalPtr' in package 'RTMB'");
  }

  SEXP ptr = fun(R_NilValue);
  if (TYPEOF(ptr) != EXTPTRSXP) {
      Rf_error("Invalid external pointer returned by 'getSetGlobalPtr'");
  }

  TMBad::global_ptr = (TMBad::global**) R_ExternalPtrAddr(ptr);
  if (TMBad::global_ptr == NULL) {
      Rf_error("Failed to retrieve a valid global pointer from 'getSetGlobalPtr'");
  }

  Rprintf("Global pointer successfully set: %p\n", (void*)TMBad::global_ptr);
  return R_NilValue;
}