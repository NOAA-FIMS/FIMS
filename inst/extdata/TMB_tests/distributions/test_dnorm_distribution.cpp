/*! \file test_dnorm_distribution.cpp 
 * links TMB to test_dnorm_distribution.hpp
 */

#include "test_dnorm_distribution.hpp"

template<class Type>
Type objective_function<Type>::operator()(){
  /*
   * create pointer, inst, that points to singleton class of Model in test_dnorm_distribution.hpp
   * getinstance is defined in test_dnorm_distribution.hpp
   */ 
  fims::Model<Type>* inst = fims::Model<Type>::getInstance();

  DATA_VECTOR(y);
  PARAMETER_VECTOR(p);
  Type mean = p[0];
  Type sd = exp(p[1]);

  /*
   * access and assign members of Model class using inst pointer
   */
  inst -> y = y;
  inst -> mean = mean;
  inst -> sd = sd;

  /*
   *   create Type nll and assign value to the return of the 
   *   evaluate() function defined in test_dnorm_distribution.hpp
   */  
  Type nll = inst -> evaluate();
  return nll;
}
