// Translation unit to explicitly instantiate recruitment functors

#include "../inst/include/population_dynamics/recruitment/functors/log_r.hpp"
#include "../inst/include/population_dynamics/recruitment/functors/log_devs.hpp"
#include "../inst/include/population_dynamics/recruitment/functors/sr_beverton_holt.hpp"
#include "../inst/include/population_dynamics/recruitment/functors/recruitment_base.hpp"

#include <RcppCommon.h>

using namespace Rcpp;

#define TMBAD_ASSERT2(x,msg) (void) (x);
#define TMBAD_ASSERT(x) (void) (x);

#include <TMBad/global.hpp>

template struct fims_popdy::LogR<double>;
template struct fims_popdy::LogR<TMBad::ad_aug>;

template struct fims_popdy::LogDevs<double>;
template struct fims_popdy::LogDevs<TMBad::ad_aug>;

template struct fims_popdy::SRBevertonHolt<double>;
template struct fims_popdy::SRBevertonHolt<TMBad::ad_aug>;

template struct fims_popdy::RecruitmentBase<double>;
template struct fims_popdy::RecruitmentBase<TMBad::ad_aug>;
