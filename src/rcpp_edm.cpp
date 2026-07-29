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
      .property("embedding_dimension",
                &DelayEmbeddingInterface::get_embedding_dimension,
                &DelayEmbeddingInterface::set_embedding_dimension)
      .property("time_lag",
                &DelayEmbeddingInterface::get_time_lag,
                &DelayEmbeddingInterface::set_time_lag)
      .property("n_rows",
                &DelayEmbeddingInterface::get_n_rows,
                &DelayEmbeddingInterface::set_n_rows)
      .property("n_cols",
                &DelayEmbeddingInterface::get_n_cols,
                &DelayEmbeddingInterface::set_n_cols)
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
      .property("embedding_dimension",
                &SimplexProjectionInterface::get_embedding_dimension,
                &SimplexProjectionInterface::set_embedding_dimension)
      .property("n_neighbors",
                &SimplexProjectionInterface::get_n_neighbors,
                &SimplexProjectionInterface::set_n_neighbors)
      .property("lib_de_id",
                &SimplexProjectionInterface::get_lib_de_id,
                &SimplexProjectionInterface::set_lib_de_id)
      .property("test_de_id",
                &SimplexProjectionInterface::get_test_de_id,
                &SimplexProjectionInterface::set_test_de_id)
      .property("forecast_horizon",
                &SimplexProjectionInterface::get_forecast_horizon,
                &SimplexProjectionInterface::set_forecast_horizon)
      .field("predictions", &SimplexProjectionInterface::predictions)
      .method("get_id", &SimplexProjectionInterface::get_id)
      .method("predict", &SimplexProjectionInterface::predict)
      .method("finalize", &SimplexProjectionInterface::finalize);

  Rcpp::class_<SMapProjectionInterface>(
      "SMapProjection",
      "Exposes S-Map projection functor to R.")
      .constructor()
      .property("embedding_dimension",
                &SMapProjectionInterface::get_embedding_dimension,
                &SMapProjectionInterface::set_embedding_dimension)
      .property("theta",
                &SMapProjectionInterface::get_theta,
                &SMapProjectionInterface::set_theta)
      .property("kernel",
                &SMapProjectionInterface::get_kernel,
                &SMapProjectionInterface::set_kernel)
      .property("lib_de_id",
                &SMapProjectionInterface::get_lib_de_id,
                &SMapProjectionInterface::set_lib_de_id)
      .property("test_de_id",
                &SMapProjectionInterface::get_test_de_id,
                &SMapProjectionInterface::set_test_de_id)
      .property("forecast_horizon",
                &SMapProjectionInterface::get_forecast_horizon,
                &SMapProjectionInterface::set_forecast_horizon)
      .field("predictions", &SMapProjectionInterface::predictions)
      .method("get_id", &SMapProjectionInterface::get_id)
      .method("predict", &SMapProjectionInterface::predict)
      .method("finalize", &SMapProjectionInterface::finalize);

  Rcpp::class_<GPEdmProjectionInterface>(
      "GPEdmProjection",
      "Exposes GP-EDM projection functor to R.")
      .constructor()
      .property("embedding_dimension",
                &GPEdmProjectionInterface::get_embedding_dimension,
                &GPEdmProjectionInterface::set_embedding_dimension)
      .field("phi", &GPEdmProjectionInterface::phi)
      .property("sigma2",
                &GPEdmProjectionInterface::get_sigma2,
                &GPEdmProjectionInterface::set_sigma2)
      .property("ve",
                &GPEdmProjectionInterface::get_ve,
                &GPEdmProjectionInterface::set_ve)
      .property("lib_de_id",
                &GPEdmProjectionInterface::get_lib_de_id,
                &GPEdmProjectionInterface::set_lib_de_id)
      .property("test_de_id",
                &GPEdmProjectionInterface::get_test_de_id,
                &GPEdmProjectionInterface::set_test_de_id)
      .property("forecast_horizon",
                &GPEdmProjectionInterface::get_forecast_horizon,
                &GPEdmProjectionInterface::set_forecast_horizon)
      .field("predictions", &GPEdmProjectionInterface::predictions)
      .method("get_id", &GPEdmProjectionInterface::get_id)
      .method("fit", &GPEdmProjectionInterface::fit)
      .method("predict", &GPEdmProjectionInterface::predict)
      .method("finalize", &GPEdmProjectionInterface::finalize);

  Rcpp::class_<SharedDouble>("SharedDouble")
      .constructor()
      .constructor<double>()
      .method("get", &SharedDouble::get)
      .method("set", &SharedDouble::set);
}

