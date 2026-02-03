// Translation unit to explicitly instantiate maturity functors

#include "../inst/include/population_dynamics/maturity/functors/logistic.hpp"
#include "../inst/include/population_dynamics/maturity/functors/maturity_base.hpp"

#include <RcppCommon.h>

using namespace Rcpp;

#define TMBAD_ASSERT2(x,msg) (void) (x);
#define TMBAD_ASSERT(x) (void) (x);

#include <TMBad/global.hpp>

template struct fims_popdy::LogisticMaturity<double>;
template struct fims_popdy::LogisticMaturity<TMBad::ad_aug>;

template struct fims_popdy::MaturityBase<double>;
template struct fims_popdy::MaturityBase<TMBad::ad_aug>;
