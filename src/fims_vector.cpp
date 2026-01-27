

#include "../inst/include/common/fims_vector.hpp"
#include "TMBTypes.hpp"
/**
 * @brief Explicit instantiation translation unit for fims::Vector templates
 */
#ifdef TMB_MODEL
#ifdef TMBAD_FRAMEWORK
template class fims::Vector<TMBad::ad_aug>;
#else
template class fims::Vector<TMBad::ad_aug>;
#endif
#endif
template class fims::Vector<double>;