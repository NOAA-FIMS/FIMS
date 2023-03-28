/*! \file test_fleet_acomp_nll.cpp 
 * links TMB to test_fleet_acomp_nll.hpp
 */

#include "test_fleet_acomp_nll.hpp"
  
template<class Type>
Type objective_function<Type>::operator()(){
  /*
   * create pointer, inst, that points to singleton class of Model in test_fleet_acomp_nll.hpp
   * getinstance is defined in test_fleet_acomp_nll.hpp
   */ 
  fims::Model<Type>* inst = fims::Model<Type>::getInstance();
 
  DATA_VECTOR(x);
  PARAMETER_VECTOR(p);
  /*   
   * access and assign members of Model class using inst pointer
   */
  inst -> of = this; 
  inst -> x = x;
  inst -> p = p;
  /*
   *   create Type nll and assign value to the return of the 
   *   evaluate() function defined in test_fleet_acomp_nll.hpp
   */ 
  Type nll = inst -> evaluate(); 

  return nll;
}
