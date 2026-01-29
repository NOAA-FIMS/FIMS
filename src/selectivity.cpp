

#include "../inst/include/population_dynamics/selectivity/functors/logistic.hpp"
#include "../inst/include/population_dynamics/selectivity/functors/detail/logistic_impl.hpp"

#include <RcppCommon.h>

using namespace Rcpp;


#define TMBAD_ASSERT2(x,msg) (void) (x);
#define TMBAD_ASSERT(x) (void) (x);


#include <TMBad/global.hpp>



template struct fims_popdy::LogisticSelectivity<double>;
template struct fims_popdy::LogisticSelectivity<TMBad::ad_aug>; 