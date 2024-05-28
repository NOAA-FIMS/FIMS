/**
 * @file init.hpp
 * @brief An interface to dynamically load the functions.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef INTERFACE_INIT_HPP
#define INTERFACE_INIT_HPP
#include <R_ext/Rdynload.h>
#include <stdlib.h>

/**
 * @brief Callback definition for TMB C++ functions.
 */
#ifndef TMB_CALLDEFS
#define TMB_CALLDEFS                                             \
  {"MakeADFunObject", (DL_FUNC)&MakeADFunObject, 4},             \
      {"InfoADFunObject", (DL_FUNC)&InfoADFunObject, 1},         \
      {"EvalADFunObject", (DL_FUNC)&EvalADFunObject, 3},         \
      {"MakeDoubleFunObject", (DL_FUNC)&MakeDoubleFunObject, 3}, \
      {"EvalDoubleFunObject", (DL_FUNC)&EvalDoubleFunObject, 3}, \
      {"getParameterOrder", (DL_FUNC)&getParameterOrder, 3},     \
      {"MakeADGradObject", (DL_FUNC)&MakeADGradObject, 3},       \
      {"MakeADHessObject2", (DL_FUNC)&MakeADHessObject2, 4},     \
      {"usingAtomics", (DL_FUNC)&usingAtomics, 0}, {             \
    "TMBconfig", (DL_FUNC)&TMBconfig, 2                          \
  }
#endif

/**
 * @brief TODO: provide a brief description.
 * 
 */
#define CALLDEF(name, n) \
  { #name, (DL_FUNC)&name, n }

extern "C" {

/**
 * @brief TODO: provide a brief description.
 * 
 * @param mean 
 * @param nu 
 * @return SEXP 
 */
SEXP compois_calc_var(SEXP mean, SEXP nu);
/**
 * @brief TODO: provide a brief description.
 * 
 * @return SEXP 
 */
SEXP omp_check();
/**
 * @brief TODO: provide a brief description.
 * 
 * @return SEXP 
 */
SEXP omp_num_threads(SEXP);
/**
 * @brief TODO: provide a brief description.
 * 
 * @return SEXP 
 */
SEXP _rcpp_module_boot_fims();

/**
 * @brief Callback definition to load the FIMS module.
 */
static const R_CallMethodDef CallEntries[] = {
    TMB_CALLDEFS,
    {"_rcpp_module_boot_fims", (DL_FUNC)&_rcpp_module_boot_fims, 0},
    {NULL, NULL, 0}};

/**
 * @brief FIMS shared object initializer.
 * @param dll TODO: provide a brief description.
 *
 */
void R_init_FIMS(DllInfo *dll) {
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
#ifdef TMB_CCALLABLES
  TMB_CCALLABLES("FIMS");
#endif
}
}

#endif
