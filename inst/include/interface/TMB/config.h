// RTMB configuration to include by TMB.h

#ifndef R_NO_REMAP
#define R_NO_REMAP
#endif
#include <Rinternals.h>
// Any failed internal assertion raises an R error.
#define TMB_ABORT Rf_error("TMB unexpected")
// Catch *all* std exceptions (not just bad_alloc)
#define TMB_CATCH catch(std::exception& excpt)
// Do not include TMB's thread-safe workarounds
#ifdef _OPENMP
#define TMB_HAVE_THREAD_SAFE_R
#endif
// Use TMBad
#define TMBAD_FRAMEWORK
// Use 64 bit integers to ensure sizeof(ad)=16 (128 bit)
#define TMBAD_INDEX_TYPE uint64_t
// Enable out-of-bounds checking
#define TMB_SAFEBOUNDS
// TMB FIXME: Some occurrences of ASSERT and ASSERT2
#undef  ASSERT
#define ASSERT(x) TMBAD_ASSERT(x)
#undef  ASSERT2
#define ASSERT2(x, msg) TMBAD_ASSERT2(x, msg)
