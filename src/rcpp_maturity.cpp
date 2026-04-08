 #include "../inst/include/interface/rcpp/rcpp_objects/rcpp_maturity.hpp"

 // static id of the MaturityInterfaceBase object
uint32_t MaturityInterfaceBase::id_g = 1;
// local id of the MaturityInterfaceBase object map relating the ID of the
// MaturityInterfaceBase to the MaturityInterfaceBase objects
std::map<uint32_t, std::shared_ptr<MaturityInterfaceBase>>
    MaturityInterfaceBase::live_objects;