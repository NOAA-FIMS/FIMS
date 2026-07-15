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
      .field("estimation_type", &Variable::estimation_type_m);
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
      .method("set_estimation_types", &VariableVector::set_estimation_types)
      .method("fill", &VariableVector::fill)
      .method("deep_copy", &VariableVector::deep_copy_rcpp)
      .method("get_id", &VariableVector::get_id);

  Rcpp::class_<RealVector>(
      "RealVector",
      "See https://noaa-fims.github.io/FIMS/doxygen/classRealVector.html.")
      .constructor()
      .constructor<size_t>()
      .constructor<Rcpp::NumericVector, size_t>()
      .method("get", &RealVector::get)
      .method("set", &RealVector::set)
      .method("set_values", &RealVector::set_values)
      .method("get_values", &RealVector::get_values)
      .method("show", &RealVector::show)
      .method("at", &RealVector::at)
      .method("size", &RealVector::size)
      .method("resize", &RealVector::resize)
      .method("deep_copy", &RealVector::deep_copy_rcpp)
      .method("get_id", &RealVector::get_id);
}
