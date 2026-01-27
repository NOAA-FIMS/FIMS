#ifndef TMBTYPES_HPP
#define TMBTYPES_HPP    

#include <RcppCommon.h>

using namespace Rcpp;


#define TMBAD_ASSERT2(x,msg) (void) (x);
#define TMBAD_ASSERT(x) (void) (x);


#include <TMBad/global.hpp>

namespace tmbutils{
// Utilities used by the core
using namespace Eigen;
#include <tmbutils/vector.hpp>
} // namespace tmbutils


#endif