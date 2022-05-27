/*
 * File:   interface.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 */

#ifndef FIMS_INTERFACE_HPP
#define FIMS_INTERFACE_HPP
/*
 * Interface file. Uses pre-processing macros
 * to interface with multiple modeling platforms.
 */

// traits for interfacing with TMB
#ifdef TMB_MODEL
// use isnan macro in math.h instead of TMB's isnan for fixing the r-cmd-check
// issue
#include <math.h>
//#define TMB_LIB_INIT R_init_FIMS
#include <TMB.hpp>

template <typename Type>
struct ModelTraits {
  typedef typename CppAD::vector<Type> DataVector;
  typedef typename CppAD::vector<Type> ParameterVector;
};

#endif /* TMB_MODEL */

#define RCPP_NO_SUGAR
#include <Rcpp.h>

using namespace Rcpp;

void hello_fims() { std::cout << "hello fims"; }

RCPP_EXPOSED_CLASS(GrowthBase)
RCPP_EXPOSED_CLASS(EWAAgrowth)
RCPP_EXPOSED_CLASS(FIMSObject)

RCPP_MODULE(fims) {
  // place holder for module elements
  Rcpp::function("hello_fims", hello_fims);
};

  // RCPP_MODULE(LogisticSelectivity) {
  //   class_<LogisticSelectivity>("LogisticSelectivity")
  //   .constructor<LogisticSelectivity>()
  //   .field("median", &LogisticSelectivity::median)
  //   .field("slope", &LogisticSelectivity::slope)
  //   .method("evaluate", &LogisticSelectivity::evaluate)
  //   ;
  // }
  RCPP_MODULE(EWAAgrowth) {
    class_<FIMSObject>("FIMSObject")
    .field("id", &FIMSObject::id)
    .field("parameters", &FIMSObject::parameters)
    .field("random_effects_parameters", &FIMSObject::random_effects_parameters)
    .field("fixed_effects_parameters", &FIMSObject::fixed_effects_parameters)
    .method("GetID", &FIMSObject::GetId);

    class_<GrowthBase>("GrowthBase")
    .derives<FIMSObject>("FIMSObject")
    .constructor()
    .method("evaluate", &GrowthBase::evaluate)
    ;

    Rcpp::class_<EWAAgrowth>("EWAAgrowth")
    .derives<GrowthBase>("GrowthBase")
    .constructor()
    .field("ewaa", &EWAAgrowth::ewaa)
    .method("evaluate", &EWAAgrowth::evaluate)
    ;
};

#endif /* FIMS_INTERFACE_HPP */
