#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_recruitment.hpp"

// static id of the RecruitmentInterfaceBase object
uint32_t RecruitmentInterfaceBase::id_g = 1;
// local id of the RecruitmentInterfaceBase object map relating the ID of the
// RecruitmentInterfaceBase to the RecruitmentInterfaceBase objects
std::map<uint32_t, std::shared_ptr<RecruitmentInterfaceBase>>
    RecruitmentInterfaceBase::live_objects;
