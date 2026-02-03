// Translation unit to explicitly instantiate growth functors

#include "../inst/include/population_dynamics/growth/functors/ewaa.hpp"
#include "../inst/include/population_dynamics/growth/functors/growth_base.hpp"

#include <RcppCommon.h>

using namespace Rcpp;

#define TMBAD_ASSERT2(x,msg) (void) (x);
#define TMBAD_ASSERT(x) (void) (x);

#include <TMBad/global.hpp>

template struct fims_popdy::EWAAGrowth<double>;
template struct fims_popdy::EWAAGrowth<TMBad::ad_aug>;

template struct fims_popdy::GrowthBase<double>;
template struct fims_popdy::GrowthBase<TMBad::ad_aug>;
