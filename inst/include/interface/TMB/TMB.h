/**
 * TMB already included => skip!
 */
#ifndef TMB_OBJECTIVE_PTR

#include "config.h"

/**
 * \file TMB.h
 * \brief Include this file to extract declarations, definitions and selected
 * code for pre-compilation
 */
#ifndef TMB_H
/** @brief header guard*/
#define TMB_H
/** \file
    \brief Include this file to extract declarations only
*/
#ifdef TMB_PRECOMPILE // Only include this file once, and only for
                      // pre-compilation. The TMB library itself is compiled
                      // with -DTMB_PRECOMPILE, and users should not define this
                      // macro when including TMB.h.
#undef WITH_LIBTMB    // Ensure that WITH_LIBTMB is not defined during
                   // pre-compilation to avoid conflicts with the TMB library's
                   // own compilation process.
#undef TMB_PRECOMPILE // Undefine TMB_PRECOMPILE to prevent it from affecting
                      // the TMB library's compilation process.
#undef CSKIP // Undefine CSKIP to prevent it from affecting the TMB library's
             // compilation process.
#undef IF_TMB_PRECOMPILE // Undefine IF_TMB_PRECOMPILE to prevent it from
                         // affecting the TMB library's compilation process.
#undef TMB_EXTERN // Undefine TMB_EXTERN to prevent it from affecting the TMB
                  // library's compilation process.
// Redefine
#undef WITH_LIBTMB
#define TMB_PRECOMPILE
#define CSKIP(...) __VA_ARGS__
#define IF_TMB_PRECOMPILE(...) __VA_ARGS__
#define TMB_EXTERN
#else
/** \file
    \brief Include this file to extract declarations only
*/
#undef WITH_LIBTMB /** @brief Ensure that WITH_LIBTMB is not defined in user   \
                      code to avoid conflicts with the TMB library's own       \
                      compilation process.*/
#undef TMB_PRECOMPILE // Ensure that TMB_PRECOMPILE is not defined in user code
                      // to avoid conflicts with the TMB library's own
                      // compilation process.
#undef CSKIP          // Undefine CSKIP to prevent it from affecting user code.
#undef IF_TMB_PRECOMPILE // Undefine IF_TMB_PRECOMPILE to prevent it from
                         // affecting user code.
#undef TMB_EXTERN // Undefine TMB_EXTERN to prevent it from affecting user code.
/**
 * Redefine
 */
#define WITH_LIBTMB //  Define WITH_LIBTMB to indicate that the TMB library is
                    //  being used in user code. This can be used to
                    //  conditionally compile code that depends on the TMB
                    //  library.
/*Ensure that TMB_PRECOMPILE
is not defined in user code to
avoid conflicts with the TMB
library's own compilation process.*/
#undef TMB_PRECOMPILE
/** Redefine CSKIP to expand to a
 * semicolon, effectively skipping
 *  the code in user code contexts.
 */
#define CSKIP(...) ;
/**Redefine IF_TMB_PRECOMPILE to
 * expand to nothing, effectively
 * excluding the code in user code
 * contexts.
 */
#define IF_TMB_PRECOMPILE(...) //
/**Redefine TMB_EXTERN to expand to
 * extern, which is appropriate for
 * declarations in user code contexts.
 */
#define TMB_EXTERN extern      //
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wmacro-redefined"
#endif
/**
 * if TMB_PRECOMPILE is defined, then the
 * TMB library itself is being compiled,
 * and we should not include the TMB library's
 * own header files to avoid conflicts.
 * If TMB_PRECOMPILE is not defined,
 * then we are in a user code context
 * and we can safely include the TMB
 * library's header files.
 */
#include <TMB.hpp>

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif
