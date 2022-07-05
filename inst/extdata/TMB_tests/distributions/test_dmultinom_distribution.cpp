/*! \file test_dmultinom_likelihood.cpp 
 * links TMB to test_dmultinom_likelihood.hpp
 */

#include "test_dmultinom_likelihood.hpp"

template<class Type>
Type objective_function<Type>::operator()(){
  /*
   * create pointer, inst, that points to singleton class of Model in test_dmultinom_likelihood.hpp
   * getinstance is defined in test_dmultinom_likelihood.hpp
   */ 
  fims::Model<Type>* inst = fims::Model<Type>::getInstance();

  DATA_VECTOR(x);
  PARAMETER_VECTOR(p);
  /*
   * access and assign members of Model class using inst pointer
   */
  inst -> x = x;
  inst -> p = p;
  
  /*
   *   create Type nll and assign value to the return of the 
   *   evaluate() function defined in test_dmultinom_likelihood.hpp
   */ 
  Type nll = inst -> evaluate();
  return nll;
}
