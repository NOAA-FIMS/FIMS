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
  Rcpp::class_<AgeCompDataInterface>("AgeComp",
                                     "See "
                                     "https://noaa-fims.github.io/FIMS/doxygen/"
                                     "classAgeCompDataInterface.html.")
      .constructor<int, int>()
      .constructor<int, int, std::vector<std::string>>()
      .field("age_comp_data", &AgeCompDataInterface::age_comp_data)
      .method("get_id", &AgeCompDataInterface::get_id);

  Rcpp::class_<LengthCompDataInterface>(
      "LengthComp",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLengthCompDataInterface.html.")
      .constructor<int, int>()
      .constructor<int, int, std::vector<std::string>>()
      .field("length_comp_data", &LengthCompDataInterface::length_comp_data)
      .method("get_id", &LengthCompDataInterface::get_id);

  Rcpp::class_<CatchDataInterface>("Catch",
                                   "See "
                                   "https://noaa-fims.github.io/FIMS/doxygen/"
                                   "classCatchDataInterface.html.")
      .constructor<int>()
      .constructor<int, std::vector<std::string>>()
      .field("catch_data", &CatchDataInterface::catch_data)
      .method("get_id", &CatchDataInterface::get_id);

  Rcpp::class_<IndexDataInterface>(
      "Index",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classIndexDataInterface.html.")
      .constructor<int>()
      .constructor<int, std::vector<std::string>>()
      .field("index_data", &IndexDataInterface::index_data)
      .method("get_id", &IndexDataInterface::get_id);
}