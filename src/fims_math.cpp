// // Explicit instantiation translation unit for fims_math templates
// #include "../inst/include/common/fims_math.hpp"

// // Instantiate for double
// template const double fims_math::exp<double>(const double &);
// template const double fims_math::log<double>(const double &);
// template const double fims_math::cos<double>(const double &);
// template const double fims_math::sqrt<double>(const double &);
// template const double fims_math::pow<double>(const double &, const double &);
// template const double fims_math::lgamma<double>(const double &);

// template const double fims_math::logistic<double>(const double &, const double &, const double &);
// template const double fims_math::logit<double>(const double &, const double &, const double &);
// template const double fims_math::inv_logit<double>(const double &, const double &, const double &);
// template const double fims_math::double_logistic<double>(const double &, const double &, const double &, const double &, const double &);

// template const double fims_math::ad_fabs<double>(const double &, double);
// template const double fims_math::ad_min<double>(const double &, const double &, double);
// template const double fims_math::ad_max<double>(const double &, const double &, double);

// template double fims_math::sum<double>(const std::vector<double> &);
// template double fims_math::sum<double>(const fims::Vector<double> &);

// #ifdef TMB_MODEL
// // Instantiate for TMB AD augmented type
// template const TMBad::ad_aug fims_math::exp<TMBad::ad_aug>(const TMBad::ad_aug &);
// template const TMBad::ad_aug fims_math::log<TMBad::ad_aug>(const TMBad::ad_aug &);
// template const TMBad::ad_aug fims_math::cos<TMBad::ad_aug>(const TMBad::ad_aug &);
// template const TMBad::ad_aug fims_math::sqrt<TMBad::ad_aug>(const TMBad::ad_aug &);
// template const TMBad::ad_aug fims_math::pow<TMBad::ad_aug>(const TMBad::ad_aug &, const TMBad::ad_aug &);
// template const TMBad::ad_aug fims_math::lgamma<TMBad::ad_aug>(const TMBad::ad_aug &);

// template const TMBad::ad_aug fims_math::logistic<TMBad::ad_aug>(const TMBad::ad_aug &, const TMBad::ad_aug &, const TMBad::ad_aug &);
// template const TMBad::ad_aug fims_math::logit<TMBad::ad_aug>(const TMBad::ad_aug &, const TMBad::ad_aug &, const TMBad::ad_aug &);
// template const TMBad::ad_aug fims_math::inv_logit<TMBad::ad_aug>(const TMBad::ad_aug &, const TMBad::ad_aug &, const TMBad::ad_aug &);
// template const TMBad::ad_aug fims_math::double_logistic<TMBad::ad_aug>(const TMBad::ad_aug &, const TMBad::ad_aug &, const TMBad::ad_aug &, const TMBad::ad_aug &, const TMBad::ad_aug &);

// template const TMBad::ad_aug fims_math::ad_fabs<TMBad::ad_aug>(const TMBad::ad_aug &, TMBad::ad_aug);
// template const TMBad::ad_aug fims_math::ad_min<TMBad::ad_aug>(const TMBad::ad_aug &, const TMBad::ad_aug &, TMBad::ad_aug);
// template const TMBad::ad_aug fims_math::ad_max<TMBad::ad_aug>(const TMBad::ad_aug &, const TMBad::ad_aug &, TMBad::ad_aug);

// template TMBad::ad_aug fims_math::sum<TMBad::ad_aug>(const std::vector<TMBad::ad_aug> &);
// template TMBad::ad_aug fims_math::sum<TMBad::ad_aug>(const fims::Vector<TMBad::ad_aug> &);
// #endif
