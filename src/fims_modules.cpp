/**
 * \file fims_modules.cpp
 * \brief Implementation of module registration functions for the FIMS framework.
 */

#include "../inst/include/interface/rcpp/fims_modules.hpp"
#include <Rcpp.h>

/**
 * \brief Register the population module.
 * \param m The Rcpp module to register functions in.
 */
void register_population(Rcpp::Module& m);
/**
 * \brief Register the selectivity module.
 * \param m The Rcpp module to register functions in.
 */
void register_selectivity(Rcpp::Module& m);
/**
 * \brief Register the growth module.
 * \param m The Rcpp module to register functions in.
 */
void register_growth(Rcpp::Module& m);
/**
 * \brief Register the distributions module.
 * \param m The Rcpp module to register functions in.
 */
void register_distributions(Rcpp::Module& m);
/**
 * \brief Register the data module.
 * \param m The Rcpp module to register functions in.
 */
void register_data(Rcpp::Module& m);
/**
 * \brief Register the recruitment module.
 * \param m The Rcpp module to register functions in.
 */
void register_recruitment(Rcpp::Module& m);
/**
 * \brief Register the fleet module.
 * \param m The Rcpp module to register functions in.
 */
void register_fleet(Rcpp::Module& m);
/**
 * \brief Register the parameter module.
 * \param m The Rcpp module to register functions in.
 */
void register_parameter(Rcpp::Module& m);
/**
 * \brief Register the vectors module.
 * \param m The Rcpp module to register functions in.
 */
void register_vectors(Rcpp::Module& m);
/**
 * \brief Register the shared module.
 * \param m The Rcpp module to register functions in.
 */
void register_shared(Rcpp::Module& m);
/**
 * \brief Register the functions module.
 * \param m The Rcpp module to register functions in.
 */
void register_functions(Rcpp::Module& m);
/**
 * \brief Register the fishery models module.
 * \param m The Rcpp module to register functions in.
 */
void register_fishery_models(Rcpp::Module& m);
/**
 * \brief Register the maturity module.
 * \param m The Rcpp module to register functions in.
 */
void register_maturity(Rcpp::Module& m);

/**
 * Rcpp module definition for the FIMS framework. This module registers all the classes and functions that are exposed to R through Rcpp.
 */
RCPP_MODULE(fims) {
  Rcpp::Module m("fims");
  register_functions(m);
  register_parameter(m);
  register_vectors(m);
  register_shared(m);
  register_recruitment(m);
  register_maturity(m);
  register_fleet(m);
  register_population(m);
  register_selectivity(m);
  register_growth(m);
  register_distributions(m);
  register_data(m);
  register_fishery_models(m);
}