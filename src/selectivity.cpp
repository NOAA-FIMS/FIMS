

#include "../inst/include/population_dynamics/selectivity/functors/logistic.hpp"
#include "../inst/include/population_dynamics/selectivity/functors/detail/logistic_impl.hpp"



template struct fims_popdy::LogisticSelectivity<double>;
template struct fims_popdy::LogisticSelectivity<TMBad::ad_aug>; 