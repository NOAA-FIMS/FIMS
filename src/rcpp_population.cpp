#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp"


// static id of the PopulationInterfaceBase object
uint32_t PopulationInterfaceBase::id_g = 1;
// local id of the PopulationInterfaceBase object map relating the ID of the
// PopulationInterfaceBase to the PopulationInterfaceBase objects
std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>
    PopulationInterfaceBase::live_objects;
