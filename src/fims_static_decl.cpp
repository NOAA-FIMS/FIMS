

#include "F_common_def.h"
#include "F_interface_rcpp_rcpp_objects_rcpp_math.h"

namespace fims {
std::shared_ptr<FIMSLog> FIMSLog::fims_log = std::make_shared<FIMSLog>();
}


double logit_rcpp(double a, double b, double x) {
  return fims_math::logit<double>(a, b, x);
}

double inv_logit_rcpp(double a, double b, double logit_x) {
  return fims_math::inv_logit<double>(a, b, logit_x);
}
