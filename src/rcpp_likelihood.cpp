/**
 * \file rcpp_likelihood.cpp
 * \brief Implementation of Rcpp likelihood interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_likelihood.hpp"

uint32_t LikelihoodInterfaceBase::id_g = 1;
std::vector<std::shared_ptr<LikelihoodInterfaceBase>>
    LikelihoodInterfaceBase::live_objects;

std::vector<std::shared_ptr<NormalLikelihoodInterface>>
    NormalLikelihoodInterface::live_objects;

std::vector<std::shared_ptr<LognormalLikelihoodInterface>>
    LognormalLikelihoodInterface::live_objects;

std::vector<std::shared_ptr<GammaLikelihoodInterface>>
    GammaLikelihoodInterface::live_objects;

std::vector<std::shared_ptr<InvGammaLikelihoodInterface>>
    InvGammaLikelihoodInterface::live_objects;

std::vector<std::shared_ptr<MultinomialLikelihoodInterface>>
    MultinomialLikelihoodInterface::live_objects;

#include <Rcpp.h>

/**
 * Function to register likelihood classes with the Rcpp module system.
 */
void register_likelihood(Rcpp::Module& m) {
  Rcpp::class_<LikelihoodInterfaceBase>(
      "LikelihoodBase",
      "Base class for likelihood component interfaces.")
      .method("set_role", &LikelihoodInterfaceBase::set_role)
      .method("set_real_input", &LikelihoodInterfaceBase::set_real_input)
      .method("set_parameter_input",
              &LikelihoodInterfaceBase::set_parameter_input)
      .field("observed_values", &LikelihoodInterfaceBase::observed_values)
      .field("expected_values", &LikelihoodInterfaceBase::expected_values)
      .field("real_input", &LikelihoodInterfaceBase::real_input)
      .field("nll_components", &LikelihoodInterfaceBase::nll_components);

  Rcpp::class_<NormalLikelihoodInterface>(
      "NormalLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classNormalLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &NormalLikelihoodInterface::get_id)
      .method("evaluate", &NormalLikelihoodInterface::evaluate)
      .field("log_sd", &NormalLikelihoodInterface::log_sd);

  Rcpp::class_<LognormalLikelihoodInterface>(
      "LognormalLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLognormalLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &LognormalLikelihoodInterface::get_id)
      .method("evaluate", &LognormalLikelihoodInterface::evaluate)
      .field("log_sd", &LognormalLikelihoodInterface::log_sd);

  Rcpp::class_<GammaLikelihoodInterface>(
      "GammaLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classGammaLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &GammaLikelihoodInterface::get_id)
      .method("evaluate", &GammaLikelihoodInterface::evaluate)
      .field("log_sd", &GammaLikelihoodInterface::log_sd);

  Rcpp::class_<InvGammaLikelihoodInterface>(
      "InvGammaLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classInvGammaLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &InvGammaLikelihoodInterface::get_id)
      .method("evaluate", &InvGammaLikelihoodInterface::evaluate)
      .field("log_shape", &InvGammaLikelihoodInterface::log_shape)
      .field("log_scale", &InvGammaLikelihoodInterface::log_scale);

  Rcpp::class_<MultinomialLikelihoodInterface>(
      "MultinomialLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classMultinomialLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &MultinomialLikelihoodInterface::get_id)
      .method("evaluate", &MultinomialLikelihoodInterface::evaluate)
      .field("dims", &MultinomialLikelihoodInterface::dims);
}
