#ifndef COMMON_HPP
#define COMMON_HPP

//#define TMB_MODEL

#ifdef TMB_MODEL

#include <TMB.hpp>
//#include <Rcpp.h>
//using namespace Rcpp;

template<typename Type>
struct model_traits{
 typedef typename CppAD::vector<Type> data_vector;
 typedef typename CppAD::vector<Type> parameter_vector;
};

template<typename T>
T exp(const T& x){
  return exp(x);
}

template <class T>
const T log(const T& x){return std::log(x);}

#endif

#ifdef STD_LIB

#include <cmath>
#include <vector>

template<typename Type>
struct model_traits{
 typedef typename std::vector<Type> data_vector;
 typedef typename std::vector<Type> parameter_vector;
};

template<typename T>
T exp(const T& x){
  return std::exp(x);
}

template <class T>
const T log(const T& x){return std::log(x);}

#endif

#endif
