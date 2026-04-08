#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_interface_base.hpp"


/**
 * @brief The unique ID for the variable map that points to a fims::Vector.
 */
uint32_t Parameter::id_g = 0;

uint32_t ParameterVector::id_g = 0;

uint32_t RealVector::id_g = 0;

std::vector<std::shared_ptr<FIMSRcppInterfaceBase>>
    FIMSRcppInterfaceBase::fims_interface_objects;