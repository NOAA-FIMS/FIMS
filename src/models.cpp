// Translation unit to explicitly instantiate model functors (CatchAtAge)

#include "../inst/include/models/functors/catch_at_age.hpp"

#include <RcppCommon.h>

using namespace Rcpp;

#define TMBAD_ASSERT2(x,msg) (void) (x);
#define TMBAD_ASSERT(x) (void) (x);

#include <TMBad/global.hpp>

template class fims_popdy::CatchAtAge<double>;
template class fims_popdy::CatchAtAge<TMBad::ad_aug>;
