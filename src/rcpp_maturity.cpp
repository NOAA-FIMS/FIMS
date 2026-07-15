/**
 * \file rcpp_maturity.cpp
 * \brief Implementation of Rcpp maturity interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_maturity.hpp"
// static id of the MaturityInterfaceBase object
uint32_t MaturityInterfaceBase::id_g = 1;
// local id of the MaturityInterfaceBase object map relating the ID of the
// MaturityInterfaceBase to the MaturityInterfaceBase objects
std::map<uint32_t, std::shared_ptr<MaturityInterfaceBase>>
    MaturityInterfaceBase::live_objects;

#include <Rcpp.h>

/**
 * Function to register maturity classes with the Rcpp module system.
 *
 */
void register_maturity(Rcpp::Module& m) {
  Rcpp::class_<LogisticMaturityInterface>(
      "LogisticMaturity",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLogisticMaturityInterface.html.")
      .constructor()
      .field("inflection_point", &LogisticMaturityInterface::inflection_point)
      .field("slope", &LogisticMaturityInterface::slope)
      .method("get_id", &LogisticMaturityInterface::get_id)
      .method("evaluate", &LogisticMaturityInterface::evaluate);
}