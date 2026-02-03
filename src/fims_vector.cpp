#include "../inst/include/common/fims_vector.hpp"
#include "../inst/include/common/detail/fims_vector_impl.hpp"


// #include <RcppCommon.h>

// using namespace Rcpp;


#define TMBAD_ASSERT2(x,msg) (void) (x);
#define TMBAD_ASSERT(x) (void) (x);


#include <TMBad/global.hpp>

template class fims::Vector<double>;
template class fims::Vector<TMBad::ad_aug>; 