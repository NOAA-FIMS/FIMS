 #include "../inst/include/interface/rcpp/rcpp_objects/rcpp_distribution.hpp"


 // static id of the DistributionsInterfaceBase object
uint32_t DistributionsInterfaceBase::id_g = 1;
// local id of the DistributionsInterfaceBase object map relating the ID of the
// DistributionsInterfaceBase to the DistributionsInterfaceBase objects
std::map<uint32_t, std::shared_ptr<DistributionsInterfaceBase>>
    DistributionsInterfaceBase::live_objects;
