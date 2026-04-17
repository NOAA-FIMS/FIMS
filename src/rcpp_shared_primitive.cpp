 /**
 * \file rcpp_shared_primitive.cpp  
 * \brief Implementation of Rcpp shared primitive interfaces for the FIMS framework.
 */
 #include "../inst/include/interface/rcpp/rcpp_objects/rcpp_shared_primitive.hpp"
 #include "../inst/include/interface/rcpp/fims_modules.hpp"




 #include <Rcpp.h>

/**
 * Function to register shared primitive classes with the Rcpp module system.
 * @param m The Rcpp module to register the classes with.
 */
void register_shared(Rcpp::Module& m) {

  Rcpp::class_<SharedInt>(
      "SharedInt",
      "See https://noaa-fims.github.io/FIMS/doxygen/classSharedInt.html.")
      .constructor()
      .constructor<int>()
      .method("get", &SharedInt::get)
      .method("set", &SharedInt::set);

  Rcpp::class_<SharedString>(
      "SharedString",
      "See https://noaa-fims.github.io/FIMS/doxygen/classSharedString.html.")
      .constructor()
      .constructor<std::string>()
      .method("get", &SharedString::get)
      .method("set", &SharedString::set);

  Rcpp::class_<SharedBoolean>(
      "SharedBoolean",
      "See https://noaa-fims.github.io/FIMS/doxygen/classSharedBoolean.html.")
      .constructor()
      .constructor<bool>()
      .method("get", &SharedBoolean::get)
      .method("set", &SharedBoolean::set);

  Rcpp::class_<SharedReal>(
      "SharedReal",
      "See https://noaa-fims.github.io/FIMS/doxygen/classSharedReal.html.")
      .constructor()
      .constructor<double>()
      .method("get", &SharedReal::get)
      .method("set", &SharedReal::set);
}