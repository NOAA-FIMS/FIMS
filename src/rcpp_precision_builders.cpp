/**
 * \file rcpp_precision_builders.cpp
 * \brief Implementation of Rcpp precision builder interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_precision_builders.hpp"

#include <Rcpp.h>

// static id of the DSEMInterface object
uint32_t DSEMInterface::id_g = 1;

// live object map for DSEMInterface
std::map<uint32_t, std::shared_ptr<DSEMInterface>>
    DSEMInterface::live_objects;

uint32_t UnstructuredInterface::id_g = 1;

std::map<uint32_t, std::shared_ptr<UnstructuredInterface>>
    UnstructuredInterface::live_objects;

/**
 * Function to register precision builder classes with the Rcpp module system.
 */
void register_precision_builders(Rcpp::Module& m) {
  Rcpp::class_<DSEMInterface>(
      "DSEM",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classDSEMInterface.html.")
      .constructor()
      .field("ram_matrix", &DSEMInterface::ram_matrix)
      .field("beta_z", &DSEMInterface::beta_z)
      .field("n_time", &DSEMInterface::n_time)
      .field("n_variables", &DSEMInterface::n_variables)
      .method("get_id", &DSEMInterface::get_id)
      .method("register_self", &DSEMInterface::register_self)
      .method("add_to_fims_tmb", &DSEMInterface::add_to_fims_tmb)
      .method("finalize", &DSEMInterface::finalize);

    Rcpp::class_<UnstructuredInterface>(
            "Unstructured",
            "Default identity precision builder for an unstructured GMRF.")
            .constructor()
            .field("n", &UnstructuredInterface::n)
            .method("get_id", &UnstructuredInterface::get_id)
            .method("register_self", &UnstructuredInterface::register_self)
            .method("add_to_fims_tmb", &UnstructuredInterface::add_to_fims_tmb);
}