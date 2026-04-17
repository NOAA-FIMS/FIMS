/**
 * \file fims_modules.cpp
 * \brief Implementation of module registration functions for the FIMS framework.
 */

#include "../inst/include/interface/rcpp/fims_modules.hpp"
#include <Rcpp.h>

void register_population(Rcpp::Module& m);
void register_selectivity(Rcpp::Module& m);
void register_growth(Rcpp::Module& m);
void register_distributions(Rcpp::Module& m);
void register_data(Rcpp::Module& m);
void register_recruitment(Rcpp::Module& m);
void register_fleet(Rcpp::Module& m);
void register_parameter(Rcpp::Module& m);
void register_vectors(Rcpp::Module& m);
void register_shared(Rcpp::Module& m);
void register_functions(Rcpp::Module& m);
void register_fishery_models(Rcpp::Module& m);
void register_maturity(Rcpp::Module& m);
void register_functions(Rcpp::Module& m);

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