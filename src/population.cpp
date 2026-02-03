// Translation unit to explicitly instantiate Population and Fleet templates

#include "../inst/include/population_dynamics/population/population.hpp"
#include "../inst/include/population_dynamics/fleet/fleet.hpp"

#include <RcppCommon.h>

using namespace Rcpp;

#define TMBAD_ASSERT2(x,msg) (void) (x);
#define TMBAD_ASSERT(x) (void) (x);

#include <TMBad/global.hpp>

template struct fims_popdy::Population<double>;
template struct fims_popdy::Population<TMBad::ad_aug>;

template struct fims_popdy::Fleet<double>;
template struct fims_popdy::Fleet<TMBad::ad_aug>;
