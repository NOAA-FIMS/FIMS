/*! \file test_fleet_nll.cpp
 * links TMB to test_fleet_nll.hpp
 */

#include "test_fleet_index_nll.hpp"

template<class Type>
Type objective_function<Type>::operator()(){
  /*
   * create pointer, inst, that points to singleton class of Model in test_dlnorm_distribution.hpp
   * getinstance is defined in test_dlnorm_distribution.hpp
   */
  fims::Model<Type>* inst = fims::Model<Type>::getInstance();
 
  DATA_VECTOR(y);
  PARAMETER_VECTOR(p);
  PARAMETER(log_sd)
  
  /*
   * access and assign members of Model class using inst pointer
   */
  inst->mean = p;
  inst -> y = y;
  inst -> logsd = log_sd;
  inst -> of = this; 
  /*
   *   create Type nll and assign value to the return of the
   *   evaluate() function defined in test_dlnorm_distribution.hpp
   */
  Type nll = inst -> evaluate();
  return nll;
}
