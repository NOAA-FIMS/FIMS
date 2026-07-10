/**
 * \file rcpp_data.cpp
 * \brief Implementation of Rcpp data interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_data.hpp"
// static id of the DataInterfaceBase object
uint32_t DataInterfaceBase::id_g = 1;
// local id of the DataInterfaceBase object map relating the ID of the
// DataInterfaceBase to the DataInterfaceBase objects
std::map<uint32_t, std::shared_ptr<DataInterfaceBase>>
    DataInterfaceBase::live_objects;

/**
 * Function to register data classes with the Rcpp module system.
 */
void register_data(Rcpp::Module& m) {
  Rcpp::class_<DataInterfaceBase>("DataBase",
                                     "See "
                                     "https://noaa-fims.github.io/FIMS/doxygen/"
                                     "classDataInterfaceBase.html.")
      .constructor<>()
      .field("name", &DataInterfaceBase::name)
      .field("dims", &DataInterfaceBase::dims)
      .field("dim_names", &DataInterfaceBase::dim_names)
      .field("observed_data", &DataInterfaceBase::observed_data)
      .field("uncertainty", &DataInterfaceBase::uncertainty)
      .method("get_id", &DataInterfaceBase::get_id);
      
  Rcpp::class_<DataInterface>("Data",
                                     "See "
                                     "https://noaa-fims.github.io/FIMS/doxygen/"
                                     "classDataInterface.html.")
      .constructor<std::string, Rcpp::IntegerVector, Rcpp::StringVector>()
      .derives<DataInterfaceBase>("DataBase")
      .method("get_id", &DataInterface::get_id);

  Rcpp::class_<AgeCompDataInterface>("AgeComp",
                                     "See "
                                     "https://noaa-fims.github.io/FIMS/doxygen/"
                                     "classAgeCompDataInterface.html.")
      .constructor<int, int>()
      .method("get_id", &AgeCompDataInterface::get_id);

  Rcpp::class_<LengthCompDataInterface>(
      "LengthComp",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLengthCompDataInterface.html.")
      .constructor<int, int>()
      .derives<DataInterfaceBase>("DataBase")
      .method("get_id", &LengthCompDataInterface::get_id);

  Rcpp::class_<LandingsDataInterface>(
      "Landings",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLandingsDataInterface.html.")
      .constructor<int>()
      .derives<DataInterfaceBase>("DataBase")
      .method("get_id", &LandingsDataInterface::get_id);

  Rcpp::class_<IndexDataInterface>(
      "Index",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classIndexDataInterface.html.")
      .constructor<int>()
      .derives<DataInterfaceBase>("DataBase")
      .method("get_id", &IndexDataInterface::get_id);
}