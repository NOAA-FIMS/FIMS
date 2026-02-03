// Translation unit to explicitly instantiate distribution templates

#include "../inst/include/distributions/functors/normal_lpdf.hpp"
#include "../inst/include/distributions/functors/lognormal_lpdf.hpp"
#include "../inst/include/distributions/functors/multinomial_lpmf.hpp"

#include <RcppCommon.h>

using namespace Rcpp;

#define TMBAD_ASSERT2(x,msg) (void) (x);
#define TMBAD_ASSERT(x) (void) (x);


#include <TMBad/global.hpp>

// Explicit instantiations for distribution functors
template struct fims_distributions::NormalLPDF<double>;
template struct fims_distributions::NormalLPDF<TMBad::ad_aug>;

template struct fims_distributions::LogNormalLPDF<double>;
template struct fims_distributions::LogNormalLPDF<TMBad::ad_aug>;

template struct fims_distributions::MultinomialLPMF<double>;
template struct fims_distributions::MultinomialLPMF<TMBad::ad_aug>;
