/**
 * \file rcpp_edm.cpp
 * \brief Implementation of Rcpp EDM (Empirical Dynamic Modeling) interfaces
 * for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_edm.hpp"

// static id of the EDMInterfaceBase object
uint32_t EDMInterfaceBase::id_g = 1;
// map relating the ID of the EDMInterfaceBase to the EDMInterfaceBase objects
std::map<uint32_t, std::shared_ptr<EDMInterfaceBase>>
    EDMInterfaceBase::live_objects;

#include <Rcpp.h>
/**
 * Function to register EDM classes with the Rcpp module system.
 *
 */
void register_edm(Rcpp::Module& m) {
  Rcpp::class_<DelayEmbeddingInterface>(
      "DelayEmbedding",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDelayEmbeddingInterface.html.")
      .constructor()
      .field("embedding_dimension",
             &DelayEmbeddingInterface::embedding_dimension)
      .field("time_lag", &DelayEmbeddingInterface::time_lag)
      .field("n_rows", &DelayEmbeddingInterface::n_rows)
      .field("n_cols", &DelayEmbeddingInterface::n_cols)
      .field("embedded_values", &DelayEmbeddingInterface::embedded_values)
      .field("target_values", &DelayEmbeddingInterface::target_values)
      .field("embedded_uncertainty",
             &DelayEmbeddingInterface::embedded_uncertainty)
      .field("target_uncertainty",
             &DelayEmbeddingInterface::target_uncertainty)
      .method("get_id", &DelayEmbeddingInterface::get_id)
      .method("construct", &DelayEmbeddingInterface::construct)
      .method("construct_with_uncertainty",
              &DelayEmbeddingInterface::construct_with_uncertainty)
      .method("construct_drop_missing",
              &DelayEmbeddingInterface::construct_drop_missing)
      .method("construct_drop_missing_with_uncertainty",
              &DelayEmbeddingInterface::construct_drop_missing_with_uncertainty)
      .method("at", &DelayEmbeddingInterface::at);

  Rcpp::class_<SimplexProjectionInterface>(
      "SimplexProjection",
      "Exposes Simplex projection functor to R.")
      .constructor()
      .field("embedding_dimension", &SimplexProjectionInterface::embedding_dimension)
      .field("n_neighbors", &SimplexProjectionInterface::n_neighbors)
      .method("get_id", &SimplexProjectionInterface::get_id)
      .method("predict", &SimplexProjectionInterface::predict);

  Rcpp::class_<SMapProjectionInterface>(
      "SMapProjection",
      "Exposes S-Map projection functor to R.")
      .constructor()
      .field("embedding_dimension", &SMapProjectionInterface::embedding_dimension)
      .field("theta", &SMapProjectionInterface::theta)
      .field("kernel", &SMapProjectionInterface::kernel)
      .method("get_id", &SMapProjectionInterface::get_id)
      .method("predict", &SMapProjectionInterface::predict);

  Rcpp::class_<GPEdmProjectionInterface>(
      "GPEdmProjection",
      "Exposes GP-EDM projection functor to R.")
      .constructor()
      .field("embedding_dimension", &GPEdmProjectionInterface::embedding_dimension)
      .field("phi", &GPEdmProjectionInterface::phi)
      .field("sigma2", &GPEdmProjectionInterface::sigma2)
      .field("ve", &GPEdmProjectionInterface::ve)
      .method("get_id", &GPEdmProjectionInterface::get_id)
      .method("fit", &GPEdmProjectionInterface::fit)
      .method("predict", &GPEdmProjectionInterface::predict);
}
