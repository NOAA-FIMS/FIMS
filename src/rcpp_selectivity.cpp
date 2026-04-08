 #include "../inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp"


 // static id of the SelectivityInterfaceBase object
uint32_t SelectivityInterfaceBase::id_g = 1;
// local id of the SelectivityInterfaceBase object map relating the ID of the
// SelectivityInterfaceBase to the SelectivityInterfaceBase objects
std::map<uint32_t, std::shared_ptr<SelectivityInterfaceBase>>
    SelectivityInterfaceBase::live_objects;
