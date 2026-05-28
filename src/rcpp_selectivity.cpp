/**
 * \file rcpp_selectivity.cpp
 * \brief Implementation of Rcpp selectivity interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp"

// static id of the SelectivityInterfaceBase object
uint32_t SelectivityInterfaceBase::id_g = 1;
// local id of the SelectivityInterfaceBase object map relating the ID of the
// SelectivityInterfaceBase to the SelectivityInterfaceBase objects
std::map<uint32_t, std::shared_ptr<SelectivityInterfaceBase>>
    SelectivityInterfaceBase::live_objects;
#include <Rcpp.h>

/**
 * Function to register selectivity classes with the Rcpp module system.
 *
 */
void register_selectivity(Rcpp::Module& m) {
  Rcpp::class_<LogisticSelectivityInterface>(
      "LogisticSelectivity",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLogisticSelectivityInterface.html.")
      .constructor()
      .field("inflection_point",
             &LogisticSelectivityInterface::inflection_point)
      .field("slope", &LogisticSelectivityInterface::slope)
      .method("get_id", &LogisticSelectivityInterface::get_id)
      .method("evaluate", &LogisticSelectivityInterface::evaluate);

  Rcpp::class_<DoubleLogisticSelectivityInterface>(
      "DoubleLogisticSelectivity",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDoubleLogisticSelectivityInterface.html.")
      .constructor()
      .field("inflection_point_asc",
             &DoubleLogisticSelectivityInterface::inflection_point_asc)
      .field("slope_asc", &DoubleLogisticSelectivityInterface::slope_asc)
      .field("inflection_point_desc",
             &DoubleLogisticSelectivityInterface::inflection_point_desc)
      .field("slope_desc", &DoubleLogisticSelectivityInterface::slope_desc)
      .method("get_id", &DoubleLogisticSelectivityInterface::get_id)
      .method("evaluate", &DoubleLogisticSelectivityInterface::evaluate);
}