/**
 * TMB already included => skip!
 */
#ifndef TMB_OBJECTIVE_PTR

#include "config.h"
/**
 * \file TMB.h
 * \brief Include this file to extract declarations, definitions and selected code for pre-compilation
 */
#ifndef TMB_H
#define TMB_H
/** \file
    \brief Include this file to extract declarations only
*/
#ifdef TMB_PRECOMPILE // Only include this file once, and only for pre-compilation. The TMB library itself is compiled with -DTMB_PRECOMPILE, and users should not define this macro when including TMB.h.
#undef WITH_LIBTMB // Ensure that WITH_LIBTMB is not defined during pre-compilation to avoid conflicts with the TMB library's own compilation process.
#undef TMB_PRECOMPILE // Undefine TMB_PRECOMPILE to prevent it from affecting the TMB library's compilation process.
#undef CSKIP // Undefine CSKIP to prevent it from affecting the TMB library's compilation process.  
#undef IF_TMB_PRECOMPILE // Undefine IF_TMB_PRECOMPILE to prevent it from affecting the TMB library's compilation process.
#undef TMB_EXTERN // Undefine TMB_EXTERN to prevent it from affecting the TMB library's compilation process.
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
#define WITH_LIBTMB
#undef  TMB_PRECOMPILE
#define CSKIP(...) ;
#define IF_TMB_PRECOMPILE(...)
#define TMB_EXTERN extern
#endif
/**
 * \file TMB.hpp
 * \brief Include this file to extract declarations, definitions and selected code for pre-compilation
 */
#include <TMB.hpp>
#endif

#endif
