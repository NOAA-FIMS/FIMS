/*! \file test_dlnorm_distribution.cpp 
 * links TMB to test_dlnorm_distribution.hpp
 */

#include "test_dlnorm_distribution.hpp"

template<class Type>
Type objective_function<Type>::operator()(){
  /*
   * create pointer, inst, that points to singleton class of Model in test_dlnorm_distribution.hpp
   * getinstance is defined in test_dlnorm_distribution.hpp
   */ 
  fims_model::Model<Type>* inst = fims_model::Model<Type>::getInstance();

  DATA_VECTOR(logy);
  PARAMETER_VECTOR(p);
  Type meanlog = p[0];
  Type sdlog = exp(p[1]);
  /*
   * access and assign members of Model class using inst pointer
   */
  inst -> logy = logy;
  inst -> meanlog = meanlog;
  inst -> sdlog = sdlog;
  /*
   *   create Type nll and assign value to the return of the 
   *   evaluate() function defined in test_dlnorm_distribution.hpp
   */ 
  Type nll = inst -> evaluate();
  return nll;
}
