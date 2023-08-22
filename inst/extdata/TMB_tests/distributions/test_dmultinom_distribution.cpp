/*! \file test_dmultinom_distribution.cpp 
 * links TMB to test_dmultinom_distribution.hpp
 */

#include "test_dmultinom_distribution.hpp"

template<class Type>
Type objective_function<Type>::operator()(){
  /*
   * create pointer, inst, that points to singleton class of Model in test_dmultinom_distribution.hpp
   * getinstance is defined in test_dmultinom_distribution.hpp
   */ 
  fims_model::Model<Type>* inst = fims_model::Model<Type>::getInstance();

  DATA_VECTOR(x);
  PARAMETER_VECTOR(p);
  /*
   * access and assign members of Model class using inst pointer
   */
  inst -> x = x;
  inst -> p = p;
  
  /*
   *   create Type nll and assign value to the return of the 
   *   evaluate() function defined in test_dmultinom_distribution.hpp
   */ 
  Type nll = inst -> evaluate();
  return nll;
}
