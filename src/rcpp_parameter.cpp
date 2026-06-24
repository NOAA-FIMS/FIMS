/**
 * \file rcpp_parameter.cpp
 * \brief Implementation of Rcpp parameter interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_interface_base.hpp"
#include <RcppCommon.h>
#include <Rcpp.h>

/**
 * Function to register parameter classes with the Rcpp module system.
 *
 */
void register_parameter(Rcpp::Module& m) {
  Rcpp::class_<Parameter>(
      "Parameter",
      "See https://noaa-fims.github.io/FIMS/doxygen/classParameter.html.")
      .constructor()
      .constructor<double>()
      .constructor<Parameter>()
      .field("value", &Parameter::initial_value_m)
      .field("estimated_value", &Parameter::final_value_m)
      .field("id", &Parameter::id_m)
      .field("estimation_type", &Parameter::estimation_type_m);
}

/**
 *
 * Function to register vector classes with the Rcpp module system.
 *
 */
void register_vectors(Rcpp::Module& m) {
  Rcpp::class_<ParameterVector>(
      "ParameterVector",
      "See https://noaa-fims.github.io/FIMS/doxygen/classParameterVector.html.")
      .constructor()
      .constructor<size_t>()
      .constructor<Rcpp::NumericVector, size_t>()
      .method("get", &ParameterVector::get)
      .method("set", &ParameterVector::set)
      .method("show", &ParameterVector::show)
      .method("at", &ParameterVector::at)
      .method("size", &ParameterVector::size)
      .method("resize", &ParameterVector::resize)
      .method("set_values", &ParameterVector::set_values)
      .method("set_estimation_types", &ParameterVector::set_estimation_types)
      .method("set_all_estimable", &ParameterVector::set_all_estimable)
      .method("fill", &ParameterVector::fill)
      .method("get_id", &ParameterVector::get_id);

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
      .method("get_id", &RealVector::get_id);
}
