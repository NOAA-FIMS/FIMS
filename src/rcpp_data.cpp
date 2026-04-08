 #include "../inst/include/interface/rcpp/rcpp_objects/rcpp_data.hpp"

 // static id of the DataInterfaceBase object
uint32_t DataInterfaceBase::id_g = 1;
// local id of the DataInterfaceBase object map relating the ID of the
// DataInterfaceBase to the DataInterfaceBase objects
std::map<uint32_t, std::shared_ptr<DataInterfaceBase>>
    DataInterfaceBase::live_objects;
