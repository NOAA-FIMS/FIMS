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
#include <Rinternals.h>
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
#endif