 /**
 * \file rcpp_models.cpp
 * \brief Implementation of Rcpp fishery model interfaces for the FIMS framework.
 */
 #include "../inst/include/interface/rcpp/rcpp_objects/rcpp_models.hpp"
 // static id of the FleetInterfaceBase object
uint32_t FisheryModelInterfaceBase::id_g = 1;

// FleetInterfaceBase to the FleetInterfaceBase objects
std::map<uint32_t, std::shared_ptr<FisheryModelInterfaceBase>>
    FisheryModelInterfaceBase::live_objects;

    #include <Rcpp.h>



/**
 * Function to register fishery model classes with the Rcpp module system.
 * 
 */    
void register_fishery_models(Rcpp::Module& m) {
         Rcpp::class_<CatchAtAgeInterface>(
      "CatchAtAge",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classCatchAtAgeInterface.html.")
      .constructor()
      .method("AddPopulation", &CatchAtAgeInterface::AddPopulation)
      .method("get_output", &CatchAtAgeInterface::to_json)
      .method("GetId", &CatchAtAgeInterface::get_id)
      .method("DoReporting", &CatchAtAgeInterface::DoReporting)
      .method("IsReporting", &CatchAtAgeInterface::IsReporting);
}