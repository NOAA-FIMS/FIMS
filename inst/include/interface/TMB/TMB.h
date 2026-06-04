// TMB already included => skip!
#ifndef TMB_OBJECTIVE_PTR

#include "config.h"
#ifndef TMB_H
/** 
 * @brief A header guard for this file.
*/
#define TMB_H
#ifdef TMB_PRECOMPILE
/** \file
    \brief Include this file to extract declarations, definitions and selected code for pre-compilation
*/
#undef WITH_LIBTMB
#undef TMB_PRECOMPILE
#undef CSKIP
#undef IF_TMB_PRECOMPILE
#undef TMB_EXTERN
// Redefine
#undef  WITH_LIBTMB
#define TMB_PRECOMPILE
#define CSKIP(...) __VA_ARGS__
#define IF_TMB_PRECOMPILE(...) __VA_ARGS__
#define TMB_EXTERN
#else
/** \file
    \brief Include this file to extract declarations only
*/
#undef WITH_LIBTMB
#undef TMB_PRECOMPILE
#undef CSKIP
#undef IF_TMB_PRECOMPILE
#undef TMB_EXTERN
// Redefine
/** 
 * @brief Define WITH_LIBTMB to indicate that the TMB library is being used in user code. This can be used to conditionally compile code that depends on the TMB library.
 */
#define WITH_LIBTMB
#undef  TMB_PRECOMPILE
/**
 * @brief semicolon, effectively skipping  the code in user code contexts.
 */ 
#define CSKIP(...) ;
/**
 * @brief Redefine to expand to nothing.
 * 
 */
#define IF_TMB_PRECOMPILE(...)
/**
 * @brief Redefine TMB_EXTERN to expand to extern.
 * 
 */
#define TMB_EXTERN extern
#endif
#include <TMB.hpp>
#endif

#endif
