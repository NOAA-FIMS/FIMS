/**
 * \file rcpp_variable.cpp
 * \brief Implementation of Rcpp variable interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_interface_base.hpp"
#include <RcppCommon.h>
#include <Rcpp.h>

/**
 * Function to register variable classes with the Rcpp module system.
 *
 */
void register_variable(Rcpp::Module& m) {
  Rcpp::class_<Variable>(
      "Variable",
      "See https://noaa-fims.github.io/FIMS/doxygen/classVariable.html.")
      .constructor()
      .constructor<double>()
      .constructor<Variable>()
      .field("value", &Variable::initial_value_m)
      .field("estimated_value", &Variable::final_value_m)
      .field("id", &Variable::id_m)
      .method("get_estimation_status", &Variable::get_estimation_status)
      .method("set_estimation_status", &Variable::set_estimation_status);
}

/**
 *
 * Function to register vector classes with the Rcpp module system.
 *
 */
void register_vectors(Rcpp::Module& m) {
  Rcpp::class_<VariableVector>(
      "VariableVector",
      "See https://noaa-fims.github.io/FIMS/doxygen/classVariableVector.html.")
      .constructor()
      .constructor<size_t>()
      .constructor<Rcpp::NumericVector, size_t>()
      .method("get", &VariableVector::get)
      .method("set", &VariableVector::set)
      .method("show", &VariableVector::show)
      .method("at", &VariableVector::at)
      .method("size", &VariableVector::size)
      .method("resize", &VariableVector::resize)
      .method("set_values", &VariableVector::set_values)
      .method("set_estimation_status", &VariableVector::set_estimation_status)
      .method("fill", &VariableVector::fill)
      .method("deep_copy", &VariableVector::deep_copy)
      .method("get_id", &VariableVector::get_id);
}
